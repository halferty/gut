/**
 * @file metal_backend.h
 * @brief Metal render backend for Gut
 */

#pragma once

#define GUT_IMPLEMENTATION
#include "../gut_single.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

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

private:
    void createBuffers();
    void createPipelineState();
    
    CAMetalLayer* m_layer;
    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_commandQueue;
    id<MTLRenderPipelineState> m_pipelineState;
    id<MTLRenderPipelineState> m_texturePipelineState;
    
    id<MTLBuffer> m_vertexBuffer;
    id<MTLBuffer> m_indexBuffer;
    
    id<MTLCommandBuffer> m_commandBuffer;
    id<MTLRenderCommandEncoder> m_renderEncoder;
    id<CAMetalDrawable> m_drawable;
    
    u32 m_frameWidth{0};
    u32 m_frameHeight{0};
    f32 m_devicePixelRatio{1.0f};
    
    static constexpr size_t MAX_VERTICES = 65536;
    static constexpr size_t MAX_INDICES = 196608;
};

} // namespace gut
