/**
 * @file d3d11_backend.h
 * @brief D3D11 render backend for Gut
 *
 * Usage:
 *   In exactly one .cpp file:
 *     #define GUT_IMPLEMENTATION
 *     #include "d3d11_backend.h"
 *     #include "d3d11_backend.cpp"
 *
 *   Link against: d3d11.lib d3dcompiler.lib dxgi.lib
 */

#pragma once

#define GUT_IMPLEMENTATION
#include "../gut_single.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

namespace gut {

// ============================================================================
// D3D11 Texture
// ============================================================================

class D3D11Texture : public Texture {
    GUT_OBJECT(D3D11Texture, Texture)
public:
    D3D11Texture(ComPtr<ID3D11Texture2D> tex,
                 ComPtr<ID3D11ShaderResourceView> srv,
                 u32 w, u32 h)
        : m_texture(std::move(tex))
        , m_srv(std::move(srv))
        , m_w(w), m_h(h) {}

    u32 width() const override { return m_w; }
    u32 height() const override { return m_h; }
    void* nativeHandle() const override { return m_srv.Get(); }

    ID3D11ShaderResourceView* srv() const { return m_srv.Get(); }
    ID3D11Texture2D* texture2D() const { return m_texture.Get(); }

private:
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_srv;
    u32 m_w, m_h;
};

// ============================================================================
// D3D11 Render Backend
// ============================================================================

class D3D11RenderBackend : public RenderBackend {
public:
    /**
     * @brief Construct a D3D11 backend.
     * @param device   An existing D3D11 device.
     * @param context  The immediate device context.
     * @param swapChain The swap chain to present to.
     */
    D3D11RenderBackend(ID3D11Device* device,
                       ID3D11DeviceContext* context,
                       IDXGISwapChain* swapChain);
    ~D3D11RenderBackend() override;

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

    // Text (stub — uses font system in gut)
    Size2f measureText(const TextLayout& layout) override;
    void renderText(const TextLayout& layout, Point2f position, Color color,
                    std::vector<Vertex>& vertices, std::vector<u32>& indices) override;

private:
    void createRenderTarget();
    void releaseRenderTarget();
    void createShaders();
    void createBuffers();
    void createRasterizerStates();
    void createBlendState();
    void createSamplerState();
    void createDepthStencilState();

    // Backdrop blur helpers
    void createBlurResources();
    void ensureBlurTextures(u32 width, u32 height);
    void executeBackdropBlur(const DrawCommand& cmd,
                             const DirectX::XMFLOAT4X4& projection);

    // External D3D11 objects (not owned)
    ID3D11Device*           m_device{nullptr};
    ID3D11DeviceContext*    m_context{nullptr};
    IDXGISwapChain*         m_swapChain{nullptr};

    // Render target
    ComPtr<ID3D11RenderTargetView> m_rtv;
    ComPtr<ID3D11Texture2D>        m_backBuffer;

    // Shaders
    ComPtr<ID3D11VertexShader>   m_vertexShader;
    ComPtr<ID3D11PixelShader>    m_pixelShaderSolid;
    ComPtr<ID3D11PixelShader>    m_pixelShaderTextured;
    ComPtr<ID3D11InputLayout>    m_inputLayout;

    // Blur shaders
    ComPtr<ID3D11VertexShader>   m_blurVertexShader;
    ComPtr<ID3D11PixelShader>    m_blurPixelShader;
    ComPtr<ID3D11PixelShader>    m_compositePixelShader;
    ComPtr<ID3D11InputLayout>    m_blurInputLayout;

    // Constant buffers
    ComPtr<ID3D11Buffer> m_cbProjection;     // VS constant buffer (projection matrix)
    ComPtr<ID3D11Buffer> m_cbClip;           // PS constant buffer (clip rect + SDF)

    // Dynamic vertex/index buffers
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;

    // State objects
    ComPtr<ID3D11RasterizerState>   m_rasterizerState;
    ComPtr<ID3D11RasterizerState>   m_rasterizerScissorState;
    ComPtr<ID3D11BlendState>        m_blendState;
    ComPtr<ID3D11SamplerState>      m_samplerState;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    // Blur resources
    ComPtr<ID3D11Texture2D>          m_blurTexA;
    ComPtr<ID3D11ShaderResourceView> m_blurSrvA;
    ComPtr<ID3D11RenderTargetView>   m_blurRtvA;
    ComPtr<ID3D11Texture2D>          m_blurTexB;
    ComPtr<ID3D11ShaderResourceView> m_blurSrvB;
    ComPtr<ID3D11RenderTargetView>   m_blurRtvB;
    u32 m_blurTexWidth{0};
    u32 m_blurTexHeight{0};

    // Accumulated buffer offsets per frame
    size_t m_vertexBufferOffset{0};
    size_t m_indexBufferOffset{0};

    // Frame state
    u32 m_frameWidth{0};
    u32 m_frameHeight{0};
    f32 m_devicePixelRatio{1.0f};

    // Clip state
    struct alignas(16) ClipConstants {
        float clipRect[4];   // x, y, width, height in physical pixels
        float cornerRadius;
        float enabled;
        float _pad[2];
    };
    ClipConstants m_clipConstants{};
    bool m_hasCustomScissor{false};

    static constexpr size_t MAX_VERTICES = 65536;
    static constexpr size_t MAX_INDICES  = 196608;
};

} // namespace gut
