/**
 * @file d3d11_backend.cpp
 * @brief D3D11 render backend implementation for Gut
 */

#include "d3d11_backend.h"
#include <cstring>
#include <algorithm>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

using namespace gut;

// ============================================================================
// HLSL Shaders (embedded)
// ============================================================================

static const char HLSL_VS[] = R"(
cbuffer cbProjection : register(b0) {
    float4x4 projection;
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
cbuffer cbClip : register(b0) {
    float4 clipRect;       // x, y, width, height in physical pixels
    float  cornerRadius;
    float  clipEnabled;
    float2 _pad;
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
cbuffer cbClip : register(b0) {
    float4 clipRect;
    float  cornerRadius;
    float  clipEnabled;
    float2 _pad;
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

// Kawase blur shaders
static const char HLSL_BLUR_VS[] = R"(
struct PSInput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

// Full-screen triangle from vertex ID
PSInput main(uint vid : SV_VertexID) {
    PSInput output;
    output.uv = float2((vid << 1) & 2, vid & 2);
    output.pos = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}
)";

static const char HLSL_BLUR_PS[] = R"(
cbuffer cbBlur : register(b0) {
    float2 texelSize;
    float  offset;
    float  _pad;
};

Texture2D    srcTexture : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target {
    float2 uv = input.uv;
    float2 off = texelSize * (offset + 0.5);
    
    float4 sum  = srcTexture.Sample(linearSampler, uv + float2(-off.x, -off.y));
    sum        += srcTexture.Sample(linearSampler, uv + float2( off.x, -off.y));
    sum        += srcTexture.Sample(linearSampler, uv + float2(-off.x,  off.y));
    sum        += srcTexture.Sample(linearSampler, uv + float2( off.x,  off.y));
    
    return sum * 0.25;
}
)";

static const char HLSL_COMPOSITE_VS[] = R"(
cbuffer cbComposite : register(b0) {
    float4x4 projection;
    float4   blurRect;
    float    compositeCornerRadius;
    float3   _pad;
};

struct VSInput {
    float2 pos   : POSITION;
    float2 uv    : TEXCOORD0;
    float4 color : COLOR0;
};

struct PSInput {
    float4 pos       : SV_Position;
    float2 uv        : TEXCOORD0;
    float4 tintColor : COLOR0;
};

PSInput main(VSInput input) {
    PSInput output;
    output.pos = mul(projection, float4(input.pos, 0.0, 1.0));
    output.uv  = input.uv;
    output.tintColor = input.color;
    return output;
}
)";

static const char HLSL_COMPOSITE_PS[] = R"(
cbuffer cbComposite : register(b0) {
    float4x4 projection;
    float4   blurRect;
    float    compositeCornerRadius;
    float3   _pad;
};

Texture2D    blurredTexture : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput {
    float4 pos       : SV_Position;
    float2 uv        : TEXCOORD0;
    float4 tintColor : COLOR0;
};

float roundedRectSDF(float2 p, float2 rectMin, float2 rectSize, float radius) {
    float2 center = rectMin + rectSize * 0.5;
    float2 halfSize = rectSize * 0.5 - radius;
    float2 d = abs(p - center) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

float4 main(PSInput input) : SV_Target {
    float4 blurred = blurredTexture.Sample(linearSampler, input.uv);
    float4 tint = input.tintColor;
    float4 result = blurred * (1.0 - tint.a) + tint * tint.a;
    result.a = 1.0;

    if (compositeCornerRadius > 0) {
        float dist = roundedRectSDF(input.pos.xy, blurRect.xy, blurRect.zw, compositeCornerRadius);
        result.a *= 1.0 - smoothstep(-0.75, 0.75, dist);
    }
    return result;
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

D3D11RenderBackend::D3D11RenderBackend(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    IDXGISwapChain* swapChain)
    : m_device(device)
    , m_context(context)
    , m_swapChain(swapChain)
{
    m_clipConstants = {};
    if (!initialize()) {
        OutputDebugStringA("D3D11RenderBackend: initialization failed!\n");
    }
}

D3D11RenderBackend::~D3D11RenderBackend() {
    shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

bool D3D11RenderBackend::initialize() {
    if (!m_device || !m_context || !m_swapChain) return false;

    createRenderTarget();
    createShaders();
    createBuffers();
    createRasterizerStates();
    createBlendState();
    createSamplerState();
    createDepthStencilState();
    createBlurResources();

    return m_vertexShader != nullptr;
}

void D3D11RenderBackend::shutdown() {
    releaseRenderTarget();
    m_vertexShader.Reset();
    m_pixelShaderSolid.Reset();
    m_pixelShaderTextured.Reset();
    m_inputLayout.Reset();
    m_blurVertexShader.Reset();
    m_blurPixelShader.Reset();
    m_compositePixelShader.Reset();
    m_blurInputLayout.Reset();
    m_cbProjection.Reset();
    m_cbClip.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_rasterizerState.Reset();
    m_rasterizerScissorState.Reset();
    m_blendState.Reset();
    m_samplerState.Reset();
    m_depthStencilState.Reset();
    m_blurTexA.Reset(); m_blurSrvA.Reset(); m_blurRtvA.Reset();
    m_blurTexB.Reset(); m_blurSrvB.Reset(); m_blurRtvB.Reset();
}

// ============================================================================
// Render target
// ============================================================================

void D3D11RenderBackend::createRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (backBuffer) {
        m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv);
        m_backBuffer = backBuffer;
    }
}

void D3D11RenderBackend::releaseRenderTarget() {
    m_rtv.Reset();
    m_backBuffer.Reset();
}

// ============================================================================
// Shaders
// ============================================================================

void D3D11RenderBackend::createShaders() {
    // --- Main vertex shader ---
    auto vsBlob = CompileShader(HLSL_VS, "main", "vs_5_0");
    if (!vsBlob) return;
    m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                 nullptr, &m_vertexShader);

    // Create input layout matching gut::Vertex { float x,y; float u,v; u32 color; }
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof(Vertex, x),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof(Vertex, u),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    m_device->CreateInputLayout(layoutDesc, 3,
                                vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                &m_inputLayout);

    // --- Pixel shaders ---
    auto psSolid = CompileShader(HLSL_PS_SOLID, "main", "ps_5_0");
    if (psSolid)
        m_device->CreatePixelShader(psSolid->GetBufferPointer(), psSolid->GetBufferSize(),
                                    nullptr, &m_pixelShaderSolid);

    auto psTex = CompileShader(HLSL_PS_TEXTURED, "main", "ps_5_0");
    if (psTex)
        m_device->CreatePixelShader(psTex->GetBufferPointer(), psTex->GetBufferSize(),
                                    nullptr, &m_pixelShaderTextured);

    // --- Blur vertex shader (no input layout, uses SV_VertexID) ---
    auto blurVsBlob = CompileShader(HLSL_BLUR_VS, "main", "vs_5_0");
    if (blurVsBlob)
        m_device->CreateVertexShader(blurVsBlob->GetBufferPointer(), blurVsBlob->GetBufferSize(),
                                     nullptr, &m_blurVertexShader);

    auto blurPs = CompileShader(HLSL_BLUR_PS, "main", "ps_5_0");
    if (blurPs)
        m_device->CreatePixelShader(blurPs->GetBufferPointer(), blurPs->GetBufferSize(),
                                    nullptr, &m_blurPixelShader);

    // --- Composite shaders (use same input layout as main) ---
    auto compVsBlob = CompileShader(HLSL_COMPOSITE_VS, "main", "vs_5_0");
    // Composite VS uses the same vertex layout as main
    // We already have m_inputLayout for that

    auto compPs = CompileShader(HLSL_COMPOSITE_PS, "main", "ps_5_0");
    if (compPs)
        m_device->CreatePixelShader(compPs->GetBufferPointer(), compPs->GetBufferSize(),
                                    nullptr, &m_compositePixelShader);
}

// ============================================================================
// Buffers
// ============================================================================

void D3D11RenderBackend::createBuffers() {
    // Dynamic vertex buffer
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = (UINT)(MAX_VERTICES * sizeof(Vertex));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_device->CreateBuffer(&desc, nullptr, &m_vertexBuffer);
    }
    // Dynamic index buffer
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = (UINT)(MAX_INDICES * sizeof(u32));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_device->CreateBuffer(&desc, nullptr, &m_indexBuffer);
    }
    // Projection constant buffer (VS b0)
    {
        D3D11_BUFFER_DESC desc{};
        // 96 bytes: large enough for CompositeCB used during backdrop blur
        // (projection 64 + blurRect 16 + cornerRadius 4 + pad 12)
        desc.ByteWidth = 96;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_device->CreateBuffer(&desc, nullptr, &m_cbProjection);
    }
    // Clip constant buffer (PS b0)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = sizeof(ClipConstants);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_device->CreateBuffer(&desc, nullptr, &m_cbClip);
    }
}

// ============================================================================
// State objects
// ============================================================================

void D3D11RenderBackend::createRasterizerStates() {
    // No-scissor state
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = FALSE;
        desc.DepthClipEnable = TRUE;
        m_device->CreateRasterizerState(&desc, &m_rasterizerState);
    }
    // Scissor-enabled state
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = TRUE;
        desc.DepthClipEnable = TRUE;
        m_device->CreateRasterizerState(&desc, &m_rasterizerScissorState);
    }
}

void D3D11RenderBackend::createBlendState() {
    D3D11_BLEND_DESC desc{};
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    m_device->CreateBlendState(&desc, &m_blendState);
}

void D3D11RenderBackend::createSamplerState() {
    D3D11_SAMPLER_DESC desc{};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    m_device->CreateSamplerState(&desc, &m_samplerState);
}

void D3D11RenderBackend::createDepthStencilState() {
    D3D11_DEPTH_STENCIL_DESC desc{};
    desc.DepthEnable = FALSE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    desc.StencilEnable = FALSE;
    m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
}

// ============================================================================
// Blur resources
// ============================================================================

void D3D11RenderBackend::createBlurResources() {
    // Created on demand via ensureBlurTextures
}

void D3D11RenderBackend::ensureBlurTextures(u32 width, u32 height) {
    if (width == m_blurTexWidth && height == m_blurTexHeight && m_blurTexA) return;

    m_blurTexWidth = width;
    m_blurTexHeight = height;

    // Release old
    m_blurTexA.Reset(); m_blurSrvA.Reset(); m_blurRtvA.Reset();
    m_blurTexB.Reset(); m_blurSrvB.Reset(); m_blurRtvB.Reset();

    D3D11_TEXTURE2D_DESC td{};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    m_device->CreateTexture2D(&td, nullptr, &m_blurTexA);
    m_device->CreateShaderResourceView(m_blurTexA.Get(), nullptr, &m_blurSrvA);
    m_device->CreateRenderTargetView(m_blurTexA.Get(), nullptr, &m_blurRtvA);

    m_device->CreateTexture2D(&td, nullptr, &m_blurTexB);
    m_device->CreateShaderResourceView(m_blurTexB.Get(), nullptr, &m_blurSrvB);
    m_device->CreateRenderTargetView(m_blurTexB.Get(), nullptr, &m_blurRtvB);
}

// ============================================================================
// Frame control
// ============================================================================

void D3D11RenderBackend::beginFrame(u32 width, u32 height, f32 devicePixelRatio) {
    m_frameWidth = width;
    m_frameHeight = height;
    m_devicePixelRatio = devicePixelRatio;

    // Resize render target if needed
    D3D11_TEXTURE2D_DESC bbDesc{};
    if (m_backBuffer) m_backBuffer->GetDesc(&bbDesc);
    if (!m_backBuffer || bbDesc.Width != width || bbDesc.Height != height) {
        releaseRenderTarget();
        m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        createRenderTarget();
    }

    ensureBlurTextures(width, height);

    // Reset buffer offsets
    m_vertexBufferOffset = 0;
    m_indexBufferOffset = 0;

    // Reset clip state
    m_clipConstants = {};
    m_hasCustomScissor = false;

    // Clear render target
    float clearColor[4] = { 0.1f, 0.1f, 0.12f, 1.0f };
    m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);

    // Set render target
    ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
    m_context->OMSetRenderTargets(1, rtvs, nullptr);

    // Set viewport
    D3D11_VIEWPORT vp{};
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);
}

void D3D11RenderBackend::endFrame() {
    // Present is done by the application (swap chain present)
}

// ============================================================================
// Backdrop blur
// ============================================================================

void D3D11RenderBackend::executeBackdropBlur(
    const DrawCommand& cmd,
    const DirectX::XMFLOAT4X4& projection)
{
    if (!m_blurVertexShader || !m_blurPixelShader || !m_compositePixelShader) return;
    if (!m_blurTexA || !m_blurTexB) return;

    // 1. Copy current render target to blurTexA
    m_context->CopyResource(m_blurTexA.Get(), m_backBuffer.Get());

    // 2. Multi-pass Kawase blur
    f32 radius = cmd.blurRadius * m_devicePixelRatio;
    int numPasses = std::max(2, std::min(8, (int)(radius / 4.0f)));

    struct alignas(16) BlurConstants {
        float texelSize[2];
        float offset;
        float _pad;
    };

    ID3D11ShaderResourceView* srcSrv = m_blurSrvA.Get();
    ID3D11RenderTargetView*   dstRtv = m_blurRtvB.Get();
    ID3D11ShaderResourceView* dstSrv = m_blurSrvB.Get();
    ID3D11RenderTargetView*   srcRtv = m_blurRtvA.Get();

    // Set viewport for blur passes
    D3D11_VIEWPORT blurVp{};
    blurVp.Width = (FLOAT)m_blurTexWidth;
    blurVp.Height = (FLOAT)m_blurTexHeight;
    blurVp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &blurVp);
    m_context->RSSetState(m_rasterizerState.Get());

    for (int pass = 0; pass < numPasses; pass++) {
        // Unbind SRV from output before using as input
        ID3D11ShaderResourceView* nullSrvs[] = { nullptr };
        m_context->PSSetShaderResources(0, 1, nullSrvs);

        m_context->OMSetRenderTargets(1, &dstRtv, nullptr);
        m_context->IASetInputLayout(nullptr);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->VSSetShader(m_blurVertexShader.Get(), nullptr, 0);
        m_context->PSSetShader(m_blurPixelShader.Get(), nullptr, 0);
        m_context->PSSetShaderResources(0, 1, &srcSrv);
        m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

        BlurConstants bc{};
        bc.texelSize[0] = 1.0f / m_blurTexWidth;
        bc.texelSize[1] = 1.0f / m_blurTexHeight;
        bc.offset = (float)(pass + 1);

        D3D11_MAPPED_SUBRESOURCE mapped;
        // Reuse cbClip for blur constants (same size works, or create a dedicated one)
        m_context->Map(m_cbClip.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &bc, sizeof(bc));
        m_context->Unmap(m_cbClip.Get(), 0);
        m_context->PSSetConstantBuffers(0, 1, m_cbClip.GetAddressOf());

        m_context->Draw(3, 0);  // Fullscreen triangle

        // Swap ping-pong
        std::swap(srcSrv, dstSrv);
        std::swap(srcRtv, dstRtv);
    }

    // srcSrv now holds the final blurred result

    // 3. Restore main render target
    ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
    m_context->OMSetRenderTargets(1, rtvs, nullptr);

    D3D11_VIEWPORT mainVp{};
    mainVp.Width = (FLOAT)m_frameWidth;
    mainVp.Height = (FLOAT)m_frameHeight;
    mainVp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &mainVp);

    // 4. Draw composite quad
    f32 rx = cmd.blurRect.x * m_devicePixelRatio;
    f32 ry = cmd.blurRect.y * m_devicePixelRatio;
    f32 rw = cmd.blurRect.width * m_devicePixelRatio;
    f32 rh = cmd.blurRect.height * m_devicePixelRatio;

    f32 u0 = rx / m_blurTexWidth;
    f32 v0 = ry / m_blurTexHeight;
    f32 u1 = (rx + rw) / m_blurTexWidth;
    f32 v1 = (ry + rh) / m_blurTexHeight;

    Color tint = cmd.blurTint;
    u32 packedTint = ((u32)(tint.a * 255) << 24) |
                     ((u32)(tint.b * 255) << 16) |
                     ((u32)(tint.g * 255) << 8)  |
                     ((u32)(tint.r * 255));

    Vertex quadVerts[4] = {
        {rx,      ry,      u0, v0, packedTint},
        {rx + rw, ry,      u1, v0, packedTint},
        {rx + rw, ry + rh, u1, v1, packedTint},
        {rx,      ry + rh, u0, v1, packedTint},
    };
    u32 quadIdx[6] = {0, 1, 2, 0, 2, 3};

    // Upload quad
    D3D11_MAPPED_SUBRESOURCE mapped;
    m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, quadVerts, sizeof(quadVerts));
    m_context->Unmap(m_vertexBuffer.Get(), 0);

    m_context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, quadIdx, sizeof(quadIdx));
    m_context->Unmap(m_indexBuffer.Get(), 0);

    // Set composite shaders + state
    UINT stride = sizeof(Vertex), offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_compositePixelShader.Get(), nullptr, 0);

    // Composite constant buffer
    struct alignas(16) CompositeCB {
        DirectX::XMFLOAT4X4 proj;
        float blurRect[4];
        float cornerRadius;
        float _pad[3];
    };
    CompositeCB ccb;
    ccb.proj = projection;
    ccb.blurRect[0] = rx; ccb.blurRect[1] = ry;
    ccb.blurRect[2] = rw; ccb.blurRect[3] = rh;
    ccb.cornerRadius = cmd.blurCornerRadius * m_devicePixelRatio;
    ccb._pad[0] = ccb._pad[1] = ccb._pad[2] = 0;

    m_context->Map(m_cbProjection.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &ccb, sizeof(ccb));
    m_context->Unmap(m_cbProjection.Get(), 0);
    m_context->VSSetConstantBuffers(0, 1, m_cbProjection.GetAddressOf());
    m_context->PSSetConstantBuffers(0, 1, m_cbProjection.GetAddressOf());

    ID3D11ShaderResourceView* nullSrvs[] = { nullptr };
    m_context->PSSetShaderResources(0, 1, nullSrvs);
    m_context->PSSetShaderResources(0, 1, &srcSrv);
    m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    float blendFactor[4] = { 0, 0, 0, 0 };
    m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    m_context->DrawIndexed(6, 0, 0);

    // NOTE: Do NOT reset m_vertexBufferOffset / m_indexBufferOffset here.
    // The caller re-uploads the original vertex/index data at the current
    // offset after this function returns. Resetting would cause a mismatch
    // between index values (remapped with the original offset) and vertex
    // positions (re-uploaded at offset 0).
}

// ============================================================================
// Rendering
// ============================================================================

void D3D11RenderBackend::render(
    std::span<const Vertex> vertices,
    std::span<const u32> indices,
    std::span<const DrawCommand> commands)
{
    if (commands.empty()) return;

    // Upload vertex data (scale from logical to physical pixels)
    // Keep scaled/remapped alive — backdrop blur may need to re-upload them
    std::vector<Vertex> scaled;
    std::vector<u32> remapped;

    if (!vertices.empty()) {
        scaled.assign(vertices.begin(), vertices.end());
        for (auto& v : scaled) {
            v.x *= m_devicePixelRatio;
            v.y *= m_devicePixelRatio;
        }

        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        size_t byteOff = m_vertexBufferOffset * sizeof(Vertex);
        memcpy((u8*)mapped.pData + byteOff, scaled.data(), scaled.size() * sizeof(Vertex));
        m_context->Unmap(m_vertexBuffer.Get(), 0);
    }

    if (!indices.empty()) {
        remapped.assign(indices.begin(), indices.end());
        for (auto& idx : remapped)
            idx += (u32)m_vertexBufferOffset;

        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        size_t byteOff = m_indexBufferOffset * sizeof(u32);
        memcpy((u8*)mapped.pData + byteOff, remapped.data(), remapped.size() * sizeof(u32));
        m_context->Unmap(m_indexBuffer.Get(), 0);
    }

    // Orthographic projection:  (0,0) top-left → (W,H) bottom-right
    // DirectXMath is row-major; HLSL float4x4 is column-major by default.
    // Uploading row-major data into column-major storage implicitly transposes,
    // which is exactly what mul(projection, v) needs. Do NOT add XMMatrixTranspose.
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMStoreFloat4x4(&projection,
        DirectX::XMMatrixOrthographicOffCenterLH(0, (float)m_frameWidth, (float)m_frameHeight, 0, 0, 1)
    );

    // Upload projection
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_cbProjection.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &projection, sizeof(projection));
        m_context->Unmap(m_cbProjection.Get(), 0);
    }

    // Upload initial clip constants
    m_clipConstants = {};
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_cbClip.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &m_clipConstants, sizeof(m_clipConstants));
        m_context->Unmap(m_cbClip.Get(), 0);
    }

    // Bind pipeline state
    UINT stride = sizeof(Vertex), vbOffset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &vbOffset);
    m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, m_cbProjection.GetAddressOf());
    m_context->PSSetConstantBuffers(0, 1, m_cbClip.GetAddressOf());
    m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    float blendFactor[4] = { 0, 0, 0, 0 };
    m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    m_context->RSSetState(m_rasterizerState.Get());

    // Execute draw commands
    for (const auto& cmd : commands) {
        if (cmd.type == DrawCommandType::SetClip) {
            // Scissor rect
            D3D11_RECT scissor;
            scissor.left   = (LONG)std::max(0.0f, cmd.clipRect.x * m_devicePixelRatio);
            scissor.top    = (LONG)std::max(0.0f, cmd.clipRect.y * m_devicePixelRatio);
            scissor.right  = (LONG)std::max(0.0f, (cmd.clipRect.x + cmd.clipRect.width) * m_devicePixelRatio);
            scissor.bottom = (LONG)std::max(0.0f, (cmd.clipRect.y + cmd.clipRect.height) * m_devicePixelRatio);
            m_context->RSSetScissorRects(1, &scissor);
            m_context->RSSetState(m_rasterizerScissorState.Get());
            m_hasCustomScissor = true;

            // SDF rounded-rect clip
            if (cmd.clipCornerRadius > 0) {
                m_clipConstants.clipRect[0] = cmd.clipRect.x * m_devicePixelRatio;
                m_clipConstants.clipRect[1] = cmd.clipRect.y * m_devicePixelRatio;
                m_clipConstants.clipRect[2] = cmd.clipRect.width * m_devicePixelRatio;
                m_clipConstants.clipRect[3] = cmd.clipRect.height * m_devicePixelRatio;
                m_clipConstants.cornerRadius = cmd.clipCornerRadius * m_devicePixelRatio;
                m_clipConstants.enabled = 1.0f;
            } else {
                m_clipConstants.enabled = 0.0f;
            }

            D3D11_MAPPED_SUBRESOURCE mapped;
            m_context->Map(m_cbClip.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, &m_clipConstants, sizeof(m_clipConstants));
            m_context->Unmap(m_cbClip.Get(), 0);
        }
        else if (cmd.type == DrawCommandType::ClearClip) {
            m_context->RSSetState(m_rasterizerState.Get());
            m_hasCustomScissor = false;
            m_clipConstants.enabled = 0.0f;

            D3D11_MAPPED_SUBRESOURCE mapped;
            m_context->Map(m_cbClip.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, &m_clipConstants, sizeof(m_clipConstants));
            m_context->Unmap(m_cbClip.Get(), 0);
        }
        else if (cmd.type == DrawCommandType::DrawTriangles) {
            m_context->PSSetShader(m_pixelShaderSolid.Get(), nullptr, 0);
            m_context->DrawIndexed(cmd.indexCount,
                                   (UINT)(m_indexBufferOffset + cmd.indexOffset), 0);
        }
        else if (cmd.type == DrawCommandType::DrawTexturedTriangles) {
            m_context->PSSetShader(m_pixelShaderTextured.Get(), nullptr, 0);
            if (cmd.texture) {
                auto* dt = static_cast<D3D11Texture*>(cmd.texture);
                ID3D11ShaderResourceView* srvs[] = { dt->srv() };
                m_context->PSSetShaderResources(0, 1, srvs);
            }
            m_context->DrawIndexed(cmd.indexCount,
                                   (UINT)(m_indexBufferOffset + cmd.indexOffset), 0);
        }
        else if (cmd.type == DrawCommandType::DrawBackdropBlur) {
            executeBackdropBlur(cmd, projection);

            // Backdrop blur used MAP_WRITE_DISCARD on vertex/index buffers,
            // destroying all previously uploaded data. Re-upload everything.
            if (!scaled.empty()) {
                D3D11_MAPPED_SUBRESOURCE mapped;
                m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                size_t byteOff = m_vertexBufferOffset * sizeof(Vertex);
                memcpy((u8*)mapped.pData + byteOff, scaled.data(), scaled.size() * sizeof(Vertex));
                m_context->Unmap(m_vertexBuffer.Get(), 0);
            }
            if (!remapped.empty()) {
                D3D11_MAPPED_SUBRESOURCE mapped;
                m_context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                size_t byteOff = m_indexBufferOffset * sizeof(u32);
                memcpy((u8*)mapped.pData + byteOff, remapped.data(), remapped.size() * sizeof(u32));
                m_context->Unmap(m_indexBuffer.Get(), 0);
            }

            // Re-upload projection (blur overwrote cbProjection with CompositeCB)
            {
                D3D11_MAPPED_SUBRESOURCE mapped;
                m_context->Map(m_cbProjection.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                memcpy(mapped.pData, &projection, sizeof(projection));
                m_context->Unmap(m_cbProjection.Get(), 0);
            }

            // Re-upload clip constants (blur overwrote cbClip with BlurConstants)
            {
                D3D11_MAPPED_SUBRESOURCE mapped;
                m_context->Map(m_cbClip.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                memcpy(mapped.pData, &m_clipConstants, sizeof(m_clipConstants));
                m_context->Unmap(m_cbClip.Get(), 0);
            }

            // Re-bind pipeline state
            m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &vbOffset);
            m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            m_context->IASetInputLayout(m_inputLayout.Get());
            m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
            m_context->VSSetConstantBuffers(0, 1, m_cbProjection.GetAddressOf());
            m_context->PSSetConstantBuffers(0, 1, m_cbClip.GetAddressOf());
            m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
            m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
            m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
            if (m_hasCustomScissor)
                m_context->RSSetState(m_rasterizerScissorState.Get());
            else
                m_context->RSSetState(m_rasterizerState.Get());
        }
    }

    // Advance offsets
    m_vertexBufferOffset += vertices.size();
    m_indexBufferOffset += indices.size();
}

// ============================================================================
// Texture management
// ============================================================================

Ref<Texture> D3D11RenderBackend::createTexture(u32 width, u32 height, const u8* pixels) {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = pixels;
    initData.SysMemPitch = width * 4;

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = m_device->CreateTexture2D(&desc, pixels ? &initData : nullptr, &texture);
    if (FAILED(hr)) return nullptr;

    ComPtr<ID3D11ShaderResourceView> srv;
    hr = m_device->CreateShaderResourceView(texture.Get(), nullptr, &srv);
    if (FAILED(hr)) return nullptr;

    return makeRef<D3D11Texture>(std::move(texture), std::move(srv), width, height);
}

Ref<Texture> D3D11RenderBackend::createTextureFromFile(StringView /*path*/) {
    // Not implemented — use createTexture with pixel data
    return nullptr;
}

void D3D11RenderBackend::updateTexture(Texture& texture, u32 x, u32 y,
                                        u32 width, u32 height, const u8* pixels) {
    auto* dt = static_cast<D3D11Texture*>(&texture);
    D3D11_BOX box{};
    box.left = x;
    box.top = y;
    box.right = x + width;
    box.bottom = y + height;
    box.front = 0;
    box.back = 1;
    m_context->UpdateSubresource(dt->texture2D(), 0, &box, pixels, width * 4, 0);
}

void D3D11RenderBackend::destroyTexture(Texture& /*texture*/) {
    // ComPtr handles release
}

// ============================================================================
// Text (stub)
// ============================================================================

Size2f D3D11RenderBackend::measureText(const TextLayout& layout) {
    return { static_cast<f32>(layout.text.length()) * 8.0f, 16.0f };
}

void D3D11RenderBackend::renderText(
    const TextLayout& /*layout*/,
    Point2f /*position*/,
    Color /*color*/,
    std::vector<Vertex>& /*vertices*/,
    std::vector<u32>& /*indices*/)
{
    // Text rendering uses the built-in font atlas system —
    // gut::Context creates textured quads via the atlas.
}
