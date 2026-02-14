/**
 * @file d3d12_backend.cpp
 * @brief D3D12 render backend implementation for Gut
 */

#include "d3d12_backend.h"
#include <cstring>
#include <algorithm>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace gut;

// ============================================================================
// HLSL Shaders (same as D3D11 but compiled for SM 5.1 root signature style)
// ============================================================================

static const char HLSL_VS[] = R"(
cbuffer cbFrame : register(b0) {
    float4x4 projection;
    float4   clipRect;
    float    cornerRadius;
    float    clipEnabled;
    float2   _pad;
};

struct VSInput {
    float2 pos   : POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

struct PSInput {
    float4 pos   : SV_Position;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

PSInput main(VSInput input) {
    PSInput output;
    output.pos   = mul(projection, float4(input.pos, 0.0, 1.0));
    output.uv    = input.uv;
    output.color = input.color;
    return output;
}
)";

static const char HLSL_PS_SOLID[] = R"(
cbuffer cbFrame : register(b0) {
    float4x4 projection;
    float4   clipRect;
    float    cornerRadius;
    float    clipEnabled;
    float2   _pad;
};

struct PSInput {
    float4 pos   : SV_Position;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

float roundedRectSDF(float2 p, float2 rectMin, float2 rectSize, float radius) {
    float2 center = rectMin + rectSize * 0.5;
    float2 halfSize = rectSize * 0.5 - radius;
    float2 d = abs(p - center) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

float4 main(PSInput input) : SV_Target {
    float4 color = input.color;
    if (clipEnabled > 0.5) {
        float dist = roundedRectSDF(input.pos.xy, clipRect.xy, clipRect.zw, cornerRadius);
        color.a *= 1.0 - smoothstep(-0.75, 0.75, dist);
    }
    return color;
}
)";

static const char HLSL_PS_TEXTURED[] = R"(
cbuffer cbFrame : register(b0) {
    float4x4 projection;
    float4   clipRect;
    float    cornerRadius;
    float    clipEnabled;
    float2   _pad;
};

Texture2D    colorTexture : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput {
    float4 pos   : SV_Position;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

float roundedRectSDF(float2 p, float2 rectMin, float2 rectSize, float radius) {
    float2 center = rectMin + rectSize * 0.5;
    float2 halfSize = rectSize * 0.5 - radius;
    float2 d = abs(p - center) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

float4 main(PSInput input) : SV_Target {
    float4 texColor = colorTexture.Sample(linearSampler, input.uv);
    float4 color = input.color * texColor;
    if (clipEnabled > 0.5) {
        float dist = roundedRectSDF(input.pos.xy, clipRect.xy, clipRect.zw, cornerRadius);
        color.a *= 1.0 - smoothstep(-0.75, 0.75, dist);
    }
    return color;
}
)";

// ============================================================================
// Helper: compile HLSL
// ============================================================================

static ComPtr<ID3DBlob> CompileShader(const char* source, const char* entry,
                                       const char* target) {
    ComPtr<ID3DBlob> blob, errorBlob;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr,
                            entry, target, flags, 0, &blob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        return nullptr;
    }
    return blob;
}

// ============================================================================
// Construction / destruction
// ============================================================================

D3D12RenderBackend::D3D12RenderBackend(
    ID3D12Device* device,
    ID3D12CommandQueue* commandQueue,
    IDXGISwapChain3* swapChain)
    : m_device(device)
    , m_commandQueue(commandQueue)
    , m_swapChain(swapChain)
{
    if (!initialize()) {
        OutputDebugStringA("D3D12RenderBackend: initialization failed!\n");
    }
}

D3D12RenderBackend::~D3D12RenderBackend() {
    shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

bool D3D12RenderBackend::initialize() {
    if (!m_device || !m_commandQueue || !m_swapChain) return false;

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create fence
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    m_fenceValues[m_frameIndex] = 1;
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent) return false;

    // Create command allocators
    for (u32 i = 0; i < FRAME_COUNT; i++) {
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                         IID_PPV_ARGS(&m_commandAllocators[i]));
    }

    // Create command list
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                m_commandAllocators[m_frameIndex].Get(),
                                nullptr, IID_PPV_ARGS(&m_commandList));
    m_commandList->Close();

    createSrvHeap();
    createRenderTargets();
    createRootSignatureAndPSO();
    createBuffers();

    return m_psoSolid != nullptr;
}

void D3D12RenderBackend::shutdown() {
    waitForGpu();

    if (m_fenceEvent) {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    for (u32 i = 0; i < FRAME_COUNT; i++) {
        if (m_vertexUploadPtr[i]) {
            m_vertexUploadBuffer[i]->Unmap(0, nullptr);
            m_vertexUploadPtr[i] = nullptr;
        }
        if (m_indexUploadPtr[i]) {
            m_indexUploadBuffer[i]->Unmap(0, nullptr);
            m_indexUploadPtr[i] = nullptr;
        }
        if (m_constantBufferPtr[i]) {
            m_constantBuffer[i]->Unmap(0, nullptr);
            m_constantBufferPtr[i] = nullptr;
        }
    }
}

// ============================================================================
// Synchronization
// ============================================================================

void D3D12RenderBackend::waitForGpu() {
    if (!m_fence || !m_commandQueue) return;

    UINT64 fenceValue = m_fenceValues[m_frameIndex];
    m_commandQueue->Signal(m_fence.Get(), fenceValue);

    m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    m_fenceValues[m_frameIndex]++;
}

void D3D12RenderBackend::moveToNextFrame() {
    UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex]) {
        m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

// ============================================================================
// Render targets
// ============================================================================

void D3D12RenderBackend::createRenderTargets() {
    // RTV heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (u32 i = 0; i < FRAME_COUNT; i++) {
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
}

// ============================================================================
// SRV heap
// ============================================================================

void D3D12RenderBackend::createSrvHeap() {
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.NumDescriptors = MAX_SRV_DESCRIPTORS;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));
    m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_nextSrvSlot = 0;
}

u32 D3D12RenderBackend::allocateSrvSlot() {
    u32 slot = m_nextSrvSlot++;
    return slot;
}

// ============================================================================
// Root signature & PSO
// ============================================================================

void D3D12RenderBackend::createRootSignatureAndPSO() {
    // Root signature:
    //   [0] CBV (b0) — frame constants (projection + clip)
    //   [1] Descriptor table with 1 SRV (t0) — texture
    //   Static sampler at s0

    D3D12_ROOT_PARAMETER rootParams[2]{};

    // Root parameter 0: inline CBV
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Root parameter 1: descriptor table (1 SRV at t0)
    D3D12_DESCRIPTOR_RANGE srvRange{};
    srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors = 1;
    srvRange.BaseShaderRegister = 0;
    srvRange.RegisterSpace = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &srvRange;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // Static sampler
    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;

    D3D12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.NumParameters = 2;
    rsDesc.pParameters = rootParams;
    rsDesc.NumStaticSamplers = 1;
    rsDesc.pStaticSamplers = &sampler;
    rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serialized, error;
    D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                &serialized, &error);
    if (error) {
        OutputDebugStringA((char*)error->GetBufferPointer());
        return;
    }

    m_device->CreateRootSignature(0, serialized->GetBufferPointer(),
                                  serialized->GetBufferSize(),
                                  IID_PPV_ARGS(&m_rootSignature));

    // Compile shaders
    auto vsBlob = CompileShader(HLSL_VS, "main", "vs_5_1");
    auto psSolidBlob = CompileShader(HLSL_PS_SOLID, "main", "ps_5_1");
    auto psTexBlob = CompileShader(HLSL_PS_TEXTURED, "main", "ps_5_1");
    if (!vsBlob || !psSolidBlob || !psTexBlob) return;

    // Input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof(Vertex, x),     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof(Vertex, u),     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // PSO for solid drawing
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psSolidBlob->GetBufferPointer(), psSolidBlob->GetBufferSize() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // Alpha blending
    psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoSolid));

    // PSO for textured drawing
    psoDesc.PS = { psTexBlob->GetBufferPointer(), psTexBlob->GetBufferSize() };
    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoTextured));
}

// ============================================================================
// Buffers
// ============================================================================

void D3D12RenderBackend::createBuffers() {
    D3D12_HEAP_PROPERTIES uploadHeap{};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    for (u32 i = 0; i < FRAME_COUNT; i++) {
        // Vertex upload buffer
        {
            D3D12_RESOURCE_DESC desc{};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Width = MAX_VERTICES * sizeof(Vertex);
            desc.Height = 1;
            desc.DepthOrArraySize = 1;
            desc.MipLevels = 1;
            desc.SampleDesc.Count = 1;
            desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                                              &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                              nullptr, IID_PPV_ARGS(&m_vertexUploadBuffer[i]));
            m_vertexUploadBuffer[i]->Map(0, nullptr, &m_vertexUploadPtr[i]);
        }
        // Index upload buffer
        {
            D3D12_RESOURCE_DESC desc{};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Width = MAX_INDICES * sizeof(u32);
            desc.Height = 1;
            desc.DepthOrArraySize = 1;
            desc.MipLevels = 1;
            desc.SampleDesc.Count = 1;
            desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                                              &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                              nullptr, IID_PPV_ARGS(&m_indexUploadBuffer[i]));
            m_indexUploadBuffer[i]->Map(0, nullptr, &m_indexUploadPtr[i]);
        }
        // Constant buffer (256-byte aligned)
        {
            D3D12_RESOURCE_DESC desc{};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            desc.Width = CONSTANT_BUFFER_SIZE;
            desc.Height = 1;
            desc.DepthOrArraySize = 1;
            desc.MipLevels = 1;
            desc.SampleDesc.Count = 1;
            desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                                              &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                              nullptr, IID_PPV_ARGS(&m_constantBuffer[i]));
            m_constantBuffer[i]->Map(0, nullptr, &m_constantBufferPtr[i]);
        }
    }
}

// ============================================================================
// Frame control
// ============================================================================

void D3D12RenderBackend::beginFrame(u32 width, u32 height, f32 devicePixelRatio) {
    m_frameWidth = width;
    m_frameHeight = height;
    m_devicePixelRatio = devicePixelRatio;
    m_vertexBufferOffset = 0;
    m_indexBufferOffset = 0;

    auto* allocator = m_commandAllocators[m_frameIndex].Get();
    allocator->Reset();
    m_commandList->Reset(allocator, nullptr);

    // Transition render target to RENDER_TARGET state
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    // Set render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear
    float clearColor[4] = { 0.1f, 0.1f, 0.12f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Set viewport and scissor
    D3D12_VIEWPORT viewport{};
    viewport.Width = (FLOAT)width;
    viewport.Height = (FLOAT)height;
    viewport.MaxDepth = 1.0f;
    m_commandList->RSSetViewports(1, &viewport);

    D3D12_RECT scissor = { 0, 0, (LONG)width, (LONG)height };
    m_commandList->RSSetScissorRects(1, &scissor);

    // Set descriptor heaps
    ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(1, heaps);
}

void D3D12RenderBackend::endFrame() {
    // Transition render target to PRESENT state
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    m_commandList->Close();

    ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, cmdLists);

    moveToNextFrame();
}

// ============================================================================
// Rendering
// ============================================================================

void D3D12RenderBackend::render(
    std::span<const Vertex> vertices,
    std::span<const u32> indices,
    std::span<const DrawCommand> commands)
{
    if (commands.empty()) return;

    u32 fi = m_frameIndex;

    // Upload vertices (scaled to physical pixels)
    if (!vertices.empty()) {
        std::vector<Vertex> scaled(vertices.begin(), vertices.end());
        for (auto& v : scaled) {
            v.x *= m_devicePixelRatio;
            v.y *= m_devicePixelRatio;
        }
        size_t byteOff = m_vertexBufferOffset * sizeof(Vertex);
        memcpy((u8*)m_vertexUploadPtr[fi] + byteOff,
               scaled.data(), scaled.size() * sizeof(Vertex));
    }

    // Upload indices (remapped for vertex offset)
    if (!indices.empty()) {
        std::vector<u32> remapped(indices.begin(), indices.end());
        for (auto& idx : remapped)
            idx += (u32)m_vertexBufferOffset;
        size_t byteOff = m_indexBufferOffset * sizeof(u32);
        memcpy((u8*)m_indexUploadPtr[fi] + byteOff,
               remapped.data(), remapped.size() * sizeof(u32));
    }

    // Build frame constants: projection + initial clip state
    // DirectXMath is row-major; HLSL float4x4 is column-major by default.
    // Uploading row-major data into column-major storage implicitly transposes,
    // which is exactly what mul(projection, v) needs. Do NOT add XMMatrixTranspose.
    FrameConstants fc{};
    DirectX::XMStoreFloat4x4(&fc.projection,
        DirectX::XMMatrixOrthographicOffCenterLH(0, (float)m_frameWidth, (float)m_frameHeight, 0, 0, 1)
    );
    fc.clipEnabled = 0.0f;
    memcpy(m_constantBufferPtr[fi], &fc, sizeof(fc));

    // Bind pipeline state
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetGraphicsRootConstantBufferView(0,
        m_constantBuffer[fi]->GetGPUVirtualAddress());

    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = m_vertexUploadBuffer[fi]->GetGPUVirtualAddress();
    vbv.SizeInBytes = (UINT)(MAX_VERTICES * sizeof(Vertex));
    vbv.StrideInBytes = sizeof(Vertex);
    m_commandList->IASetVertexBuffers(0, 1, &vbv);

    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = m_indexUploadBuffer[fi]->GetGPUVirtualAddress();
    ibv.SizeInBytes = (UINT)(MAX_INDICES * sizeof(u32));
    ibv.Format = DXGI_FORMAT_R32_UINT;
    m_commandList->IASetIndexBuffer(&ibv);

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Process draw commands
    for (const auto& cmd : commands) {
        if (cmd.type == DrawCommandType::SetClip) {
            // Update clip in constant buffer
            fc.clipRect[0] = cmd.clipRect.x * m_devicePixelRatio;
            fc.clipRect[1] = cmd.clipRect.y * m_devicePixelRatio;
            fc.clipRect[2] = cmd.clipRect.width * m_devicePixelRatio;
            fc.clipRect[3] = cmd.clipRect.height * m_devicePixelRatio;

            if (cmd.clipCornerRadius > 0) {
                fc.cornerRadius = cmd.clipCornerRadius * m_devicePixelRatio;
                fc.clipEnabled = 1.0f;
            } else {
                fc.clipEnabled = 0.0f;
            }
            memcpy(m_constantBufferPtr[fi], &fc, sizeof(fc));

            // Also set scissor rect
            D3D12_RECT scissor;
            scissor.left   = (LONG)std::max(0.0f, cmd.clipRect.x * m_devicePixelRatio);
            scissor.top    = (LONG)std::max(0.0f, cmd.clipRect.y * m_devicePixelRatio);
            scissor.right  = (LONG)std::max(0.0f, (cmd.clipRect.x + cmd.clipRect.width) * m_devicePixelRatio);
            scissor.bottom = (LONG)std::max(0.0f, (cmd.clipRect.y + cmd.clipRect.height) * m_devicePixelRatio);
            m_commandList->RSSetScissorRects(1, &scissor);
        }
        else if (cmd.type == DrawCommandType::ClearClip) {
            fc.clipEnabled = 0.0f;
            memcpy(m_constantBufferPtr[fi], &fc, sizeof(fc));

            D3D12_RECT scissor = { 0, 0, (LONG)m_frameWidth, (LONG)m_frameHeight };
            m_commandList->RSSetScissorRects(1, &scissor);
        }
        else if (cmd.type == DrawCommandType::DrawTriangles) {
            m_commandList->SetPipelineState(m_psoSolid.Get());
            m_commandList->DrawIndexedInstanced(
                cmd.indexCount, 1,
                (UINT)(m_indexBufferOffset + cmd.indexOffset), 0, 0);
        }
        else if (cmd.type == DrawCommandType::DrawTexturedTriangles) {
            m_commandList->SetPipelineState(m_psoTextured.Get());
            if (cmd.texture) {
                auto* dt = static_cast<D3D12Texture*>(cmd.texture);
                m_commandList->SetGraphicsRootDescriptorTable(1, dt->gpuHandle());
            }
            m_commandList->DrawIndexedInstanced(
                cmd.indexCount, 1,
                (UINT)(m_indexBufferOffset + cmd.indexOffset), 0, 0);
        }
        else if (cmd.type == DrawCommandType::DrawBackdropBlur) {
            // TODO: Implement D3D12 backdrop blur (multi-pass Kawase)
            // For now, skip — the UI will render without the blur effect
        }
    }

    m_vertexBufferOffset += vertices.size();
    m_indexBufferOffset += indices.size();
}

// ============================================================================
// Texture management
// ============================================================================

Ref<Texture> D3D12RenderBackend::createTexture(u32 width, u32 height, const u8* pixels) {
    // Create default heap texture
    D3D12_HEAP_PROPERTIES defaultHeap{};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC texDesc{};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ComPtr<ID3D12Resource> texResource;
    HRESULT hr = m_device->CreateCommittedResource(
        &defaultHeap, D3D12_HEAP_FLAG_NONE,
        &texDesc, D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, IID_PPV_ARGS(&texResource));
    if (FAILED(hr)) return nullptr;

    // Upload via staging buffer
    if (pixels) {
        UINT64 uploadSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        UINT numRows = 0;
        UINT64 rowSizeInBytes = 0;
        m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &uploadSize);

        D3D12_HEAP_PROPERTIES uploadHeap{};
        uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC uploadDesc{};
        uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.Width = uploadSize;
        uploadDesc.Height = 1;
        uploadDesc.DepthOrArraySize = 1;
        uploadDesc.MipLevels = 1;
        uploadDesc.SampleDesc.Count = 1;
        uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ComPtr<ID3D12Resource> uploadBuffer;
        m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                                          &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                          nullptr, IID_PPV_ARGS(&uploadBuffer));

        // Map and copy row by row (handle pitch alignment)
        void* mapped = nullptr;
        uploadBuffer->Map(0, nullptr, &mapped);
        for (UINT row = 0; row < numRows; row++) {
            memcpy((u8*)mapped + footprint.Offset + row * footprint.Footprint.RowPitch,
                   pixels + row * width * 4,
                   width * 4);
        }
        uploadBuffer->Unmap(0, nullptr);

        // Record copy command
        // We need a temporary command list for texture upload
        ComPtr<ID3D12CommandAllocator> uploadAllocator;
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                         IID_PPV_ARGS(&uploadAllocator));
        ComPtr<ID3D12GraphicsCommandList> uploadList;
        m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                    uploadAllocator.Get(), nullptr,
                                    IID_PPV_ARGS(&uploadList));

        D3D12_TEXTURE_COPY_LOCATION dst{};
        dst.pResource = texResource.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src{};
        src.pResource = uploadBuffer.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint = footprint;

        uploadList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

        // Transition to shader resource
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = texResource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        uploadList->ResourceBarrier(1, &barrier);

        uploadList->Close();
        ID3D12CommandList* lists[] = { uploadList.Get() };
        m_commandQueue->ExecuteCommandLists(1, lists);

        // Wait for upload to complete
        waitForGpu();
    }

    // Create SRV
    u32 srvSlot = allocateSrvSlot();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += srvSlot * m_srvDescriptorSize;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    m_device->CreateShaderResourceView(texResource.Get(), &srvDesc, cpuHandle);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    gpuHandle.ptr += srvSlot * m_srvDescriptorSize;

    return makeRef<D3D12Texture>(std::move(texResource), gpuHandle, srvSlot, width, height);
}

Ref<Texture> D3D12RenderBackend::createTextureFromFile(StringView /*path*/) {
    return nullptr;
}

void D3D12RenderBackend::updateTexture(Texture& texture, u32 x, u32 y,
                                        u32 width, u32 height, const u8* pixels) {
    // For simplicity, create a temporary upload buffer and copy
    auto* dt = static_cast<D3D12Texture*>(&texture);

    D3D12_HEAP_PROPERTIES uploadHeap{};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    UINT64 uploadSize = (UINT64)width * height * 4 + 512; // extra for alignment
    D3D12_RESOURCE_DESC uploadDesc{};
    uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Width = uploadSize;
    uploadDesc.Height = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels = 1;
    uploadDesc.SampleDesc.Count = 1;
    uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> uploadBuffer;
    m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                                      &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                      nullptr, IID_PPV_ARGS(&uploadBuffer));

    void* mapped = nullptr;
    uploadBuffer->Map(0, nullptr, &mapped);
    // Simple case: copy rows (assuming tight packing for the update region)
    UINT rowPitch = (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
                    & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
    for (u32 row = 0; row < height; row++) {
        memcpy((u8*)mapped + row * rowPitch,
               pixels + row * width * 4,
               width * 4);
    }
    uploadBuffer->Unmap(0, nullptr);

    // Record copy
    ComPtr<ID3D12CommandAllocator> uploadAllocator;
    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     IID_PPV_ARGS(&uploadAllocator));
    ComPtr<ID3D12GraphicsCommandList> uploadList;
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                uploadAllocator.Get(), nullptr,
                                IID_PPV_ARGS(&uploadList));

    // Transition to copy dest
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = dt->resource();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    uploadList->ResourceBarrier(1, &barrier);

    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = dt->resource();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
    footprint.Offset = 0;
    footprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    footprint.Footprint.Width = width;
    footprint.Footprint.Height = height;
    footprint.Footprint.Depth = 1;
    footprint.Footprint.RowPitch = rowPitch;

    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = uploadBuffer.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint = footprint;

    D3D12_BOX srcBox = { 0, 0, 0, width, height, 1 };
    uploadList->CopyTextureRegion(&dst, x, y, 0, &src, &srcBox);

    // Transition back
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    uploadList->ResourceBarrier(1, &barrier);

    uploadList->Close();
    ID3D12CommandList* lists[] = { uploadList.Get() };
    m_commandQueue->ExecuteCommandLists(1, lists);
    waitForGpu();
}

void D3D12RenderBackend::destroyTexture(Texture& /*texture*/) {
    // ComPtr handles release; descriptor slot is leaked (fine for simple usage)
}

// ============================================================================
// Text (stub)
// ============================================================================

Size2f D3D12RenderBackend::measureText(const TextLayout& layout) {
    return { static_cast<f32>(layout.text.length()) * 8.0f, 16.0f };
}

void D3D12RenderBackend::renderText(
    const TextLayout& /*layout*/,
    Point2f /*position*/,
    Color /*color*/,
    std::vector<Vertex>& /*vertices*/,
    std::vector<u32>& /*indices*/)
{
    // Text rendering uses the built-in font atlas system —
    // gut::Context creates textured quads via the atlas.
}
