/**
 * @file d3d12_backend.h
 * @brief D3D12 render backend for Gut
 *
 * Usage:
 *   In exactly one .cpp file:
 *     #define GUT_IMPLEMENTATION
 *     #include "d3d12_backend.h"
 *     #include "d3d12_backend.cpp"
 *
 *   Link against: d3d12.lib dxgi.lib d3dcompiler.lib
 */

#pragma once

#define GUT_IMPLEMENTATION
#include "../gut_single.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

namespace gut {

// ============================================================================
// D3D12 Texture
// ============================================================================

class D3D12Texture : public Texture {
    GUT_OBJECT(D3D12Texture, Texture)
public:
    D3D12Texture(ComPtr<ID3D12Resource> resource,
                 D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
                 u32 srvIndex,
                 u32 w, u32 h)
        : m_resource(std::move(resource))
        , m_gpuHandle(gpuHandle)
        , m_srvIndex(srvIndex)
        , m_w(w), m_h(h) {}

    u32 width() const override { return m_w; }
    u32 height() const override { return m_h; }
    void* nativeHandle() const override { return m_resource.Get(); }

    ID3D12Resource* resource() const { return m_resource.Get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle() const { return m_gpuHandle; }
    u32 srvIndex() const { return m_srvIndex; }

private:
    ComPtr<ID3D12Resource> m_resource;
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
    u32 m_srvIndex;
    u32 m_w, m_h;
};

// ============================================================================
// D3D12 Render Backend
// ============================================================================

class D3D12RenderBackend : public RenderBackend {
public:
    static constexpr u32 FRAME_COUNT = 2;

    /**
     * @brief Construct a D3D12 backend.
     * @param device      An existing D3D12 device.
     * @param commandQueue The direct command queue.
     * @param swapChain   The swap chain (DXGI 1.4).
     */
    D3D12RenderBackend(ID3D12Device* device,
                       ID3D12CommandQueue* commandQueue,
                       IDXGISwapChain3* swapChain);
    ~D3D12RenderBackend() override;

    // Lifecycle
    bool initialize() override;
    void shutdown() override;

    // Frame
    void beginFrame(u32 width, u32 height, f32 devicePixelRatio) override;
    void endFrame() override;

    // Rendering
    void render(
        std::span<const Vertex> vertices,
        std::span<const u32> indices,
        std::span<const DrawCommand> commands
    ) override;

    // Textures
    Ref<Texture> createTexture(u32 width, u32 height, const u8* pixels) override;
    Ref<Texture> createTextureFromFile(StringView path) override;
    void updateTexture(Texture& texture, u32 x, u32 y, u32 width, u32 height, const u8* pixels) override;
    void destroyTexture(Texture& texture) override;

    // Text (stub)
    Size2f measureText(const TextLayout& layout) override;
    void renderText(const TextLayout& layout, Point2f position, Color color,
                    std::vector<Vertex>& vertices, std::vector<u32>& indices) override;

private:
    void waitForGpu();
    void moveToNextFrame();
    void createRenderTargets();
    void createRootSignatureAndPSO();
    void createBuffers();
    void createSrvHeap();

    // Allocate an SRV slot and return the index
    u32 allocateSrvSlot();

    // External D3D12 objects (not owned)
    ID3D12Device*        m_device{nullptr};
    ID3D12CommandQueue*  m_commandQueue{nullptr};
    IDXGISwapChain3*     m_swapChain{nullptr};

    // Command allocators and list (one per frame)
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_COUNT];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // Render targets
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
    u32 m_rtvDescriptorSize{0};

    // SRV/CBV/UAV descriptor heap for textures
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    u32 m_srvDescriptorSize{0};
    u32 m_nextSrvSlot{0};
    static constexpr u32 MAX_SRV_DESCRIPTORS = 256;

    // Root signature & PSO
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_psoSolid;
    ComPtr<ID3D12PipelineState> m_psoTextured;

    // Upload buffers (ring buffer per frame)
    ComPtr<ID3D12Resource> m_vertexUploadBuffer[FRAME_COUNT];
    ComPtr<ID3D12Resource> m_indexUploadBuffer[FRAME_COUNT];
    void* m_vertexUploadPtr[FRAME_COUNT]{};
    void* m_indexUploadPtr[FRAME_COUNT]{};

    // Constant buffer (projection + clip in one upload region)
    ComPtr<ID3D12Resource> m_constantBuffer[FRAME_COUNT];
    void* m_constantBufferPtr[FRAME_COUNT]{};

    // Synchronization
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FRAME_COUNT]{};
    HANDLE m_fenceEvent{nullptr};
    u32 m_frameIndex{0};

    // Frame state
    u32 m_frameWidth{0};
    u32 m_frameHeight{0};
    f32 m_devicePixelRatio{1.0f};

    size_t m_vertexBufferOffset{0};
    size_t m_indexBufferOffset{0};

    // Clip state
    struct alignas(256) FrameConstants {
        // First 64 bytes: projection
        DirectX::XMFLOAT4X4 projection;
        // Next 32 bytes: clip
        float clipRect[4];
        float cornerRadius;
        float clipEnabled;
        float _pad[2];
    };

    static constexpr size_t MAX_VERTICES = 65536;
    static constexpr size_t MAX_INDICES  = 196608;
    static constexpr size_t CONSTANT_BUFFER_SIZE = 256; // Aligned to 256
};

} // namespace gut
