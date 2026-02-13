/**
 * @file metal_backend.h
 * @brief Metal render backend for Gut
 */

#pragma once

#define GUT_IMPLEMENTATION
#include "../gut_single.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <simd/simd.h>

namespace gut {

// Concrete Metal texture wrapping id<MTLTexture>
class MetalTexture : public Texture {
    GUT_OBJECT(MetalTexture, Texture)
public:
    MetalTexture(id<MTLTexture> tex, u32 w, u32 h) : m_tex(tex), m_w(w), m_h(h) {}
    u32 width() const override { return m_w; }
    u32 height() const override { return m_h; }
    void* nativeHandle() const override { return (__bridge void*)m_tex; }
    id<MTLTexture> metalTexture() const { return m_tex; }
private:
    id<MTLTexture> m_tex;
    u32 m_w, m_h;
};

class MetalRenderBackend : public RenderBackend {
public:
    MetalRenderBackend(CAMetalLayer* layer);
    ~MetalRenderBackend() override;
    
    bool initialize() override;
    void shutdown() override;
    
    void beginFrame(u32 width, u32 height, f32 devicePixelRatio) override;
    void endFrame() override;
    
    void render(
        std::span<const Vertex> vertices,
        std::span<const u32> indices,
        std::span<const DrawCommand> commands
    ) override;
    
    Ref<Texture> createTexture(u32 width, u32 height, const u8* pixels) override;
    Ref<Texture> createTextureFromFile(StringView path) override;
    void updateTexture(Texture& texture, u32 x, u32 y, u32 width, u32 height, const u8* pixels) override;
    void destroyTexture(Texture& texture) override;
    
    Size2f measureText(const TextLayout& layout) override;
    void renderText(const TextLayout& layout, Point2f position, Color color,
                   std::vector<Vertex>& vertices, std::vector<u32>& indices) override;

    // 3D background scene
    void enableScene3D(bool enabled);

private:
    void createBuffers();
    void createPipelineState();
    void createBlurResources();
    void ensureBlurTextures(u32 width, u32 height);
    
    // Perform multi-pass Kawase blur and draw result
    void executeBackdropBlur(const DrawCommand& cmd,
                             const simd::float4x4& projection);
    
    // Resume the main render pass after blur
    void resumeMainRenderPass();

    // 3D spinning cube
    void create3DResources();
    void render3DCube(id<MTLRenderCommandEncoder> encoder);
    void ensureDepthTexture(u32 width, u32 height);
    
    CAMetalLayer* m_layer;
    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_commandQueue;
    id<MTLRenderPipelineState> m_pipelineState;
    id<MTLRenderPipelineState> m_texturePipelineState;
    
    // Blur resources
    id<MTLRenderPipelineState> m_blurPipelineState;
    id<MTLRenderPipelineState> m_blurCompositePipelineState;
    id<MTLTexture> m_blurTexA;    // ping-pong texture A
    id<MTLTexture> m_blurTexB;    // ping-pong texture B
    u32 m_blurTexWidth{0};
    u32 m_blurTexHeight{0};

    // 3D scene resources
    id<MTLRenderPipelineState> m_cubePipelineState;
    id<MTLDepthStencilState> m_cubeDepthState;
    id<MTLBuffer> m_cubeVertexBuffer;
    id<MTLBuffer> m_cubeIndexBuffer;
    id<MTLTexture> m_cubeDepthTexture;
    u32 m_depthTexWidth{0};
    u32 m_depthTexHeight{0};
    bool m_scene3DEnabled{false};

    id<MTLBuffer> m_vertexBuffer;
    id<MTLBuffer> m_indexBuffer;
    
    // Accumulated offsets within the vertex/index buffers for current frame.
    // Each render() call appends data at these offsets instead of overwriting from 0.
    size_t m_vertexBufferOffset{0};
    size_t m_indexBufferOffset{0};
    
    id<MTLCommandBuffer> m_commandBuffer;
    id<MTLRenderCommandEncoder> m_renderEncoder;
    id<CAMetalDrawable> m_drawable;
    
    // Clip state that needs to persist across render pass breaks
    struct ClipUniforms {
        simd::float4 clipRect;
        float cornerRadius;
        float enabled;
        simd::float2 _pad;
    };
    ClipUniforms m_clipUniforms;
    MTLScissorRect m_currentScissor;
    bool m_hasCustomScissor{false};
    
    u32 m_frameWidth{0};
    u32 m_frameHeight{0};
    f32 m_devicePixelRatio{1.0f};
    
    static constexpr size_t MAX_VERTICES = 65536;
    static constexpr size_t MAX_INDICES = 196608;
};

} // namespace gut
