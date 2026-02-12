/**
 * @file metal_backend.mm
 * @brief Metal render backend implementation
 */

#include "metal_backend.h"
#include <simd/simd.h>

using namespace gut;

// Vertex shader input structure matching Gut's Vertex
struct ShaderVertex {
    simd::float2 position;
    simd::float2 texCoord;
    packed_float4 color;
};

// Metal shader source code (embedded)
static const char* METAL_SHADERS = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float4 color [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

struct Uniforms {
    float4x4 projection;
};

vertex VertexOut vertex_main(
    VertexIn in [[stage_in]],
    constant Uniforms& uniforms [[buffer(1)]]
) {
    VertexOut out;
    out.position = uniforms.projection * float4(in.position, 0.0, 1.0);
    out.texCoord = in.texCoord;
    out.color = in.color;
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
    return in.color;
}

fragment float4 fragment_textured(
    VertexOut in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]]
) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    return in.color * texColor;
}
)";

MetalRenderBackend::MetalRenderBackend(CAMetalLayer* layer)
    : m_layer(layer)
{
    if (!initialize()) {
        NSLog(@"Failed to initialize Metal backend!");
    }
}

MetalRenderBackend::~MetalRenderBackend() {
    shutdown();
}

bool MetalRenderBackend::initialize() {
    NSLog(@"MetalRenderBackend::initialize() starting");
    m_device = m_layer.device;
    if (!m_device) {
        m_device = MTLCreateSystemDefaultDevice();
        if (!m_device) {
            NSLog(@"Failed to create Metal device");
            return false;
        }
        m_layer.device = m_device;
    }
    
    m_commandQueue = [m_device newCommandQueue];
    if (!m_commandQueue) {
        NSLog(@"Failed to create command queue");
        return false;
    }
    
    createBuffers();
    createPipelineState();
    
    NSLog(@"MetalRenderBackend::initialize() complete");
    return true;
}

void MetalRenderBackend::shutdown() {
    m_vertexBuffer = nil;
    m_indexBuffer = nil;
    m_pipelineState = nil;
    m_texturePipelineState = nil;
    m_commandQueue = nil;
    m_device = nil;
}

void MetalRenderBackend::createBuffers() {
    m_vertexBuffer = [m_device newBufferWithLength:MAX_VERTICES * sizeof(Vertex)
                                          options:MTLResourceStorageModeShared];
    m_indexBuffer = [m_device newBufferWithLength:MAX_INDICES * sizeof(u32)
                                         options:MTLResourceStorageModeShared];
}

void MetalRenderBackend::createPipelineState() {
    NSError* error = nil;
    
    // Compile shaders
    NSString* shaderSource = [NSString stringWithUTF8String:METAL_SHADERS];
    
    id<MTLLibrary> library = [m_device newLibraryWithSource:shaderSource
                                                   options:nil
                                                     error:&error];
    if (!library) {
        NSLog(@"Failed to compile shaders: %@", error);
        return;
    }
    
    id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];
    id<MTLFunction> fragmentTexturedFunc = [library newFunctionWithName:@"fragment_textured"];
    
    // Vertex descriptor
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    
    // Position (float2)
    vertexDesc.attributes[0].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[0].offset = offsetof(Vertex, x);
    vertexDesc.attributes[0].bufferIndex = 0;
    
    // TexCoord (float2)
    vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[1].offset = offsetof(Vertex, u);
    vertexDesc.attributes[1].bufferIndex = 0;
    
    // Color (uchar4 normalized)
    vertexDesc.attributes[2].format = MTLVertexFormatUChar4Normalized;
    vertexDesc.attributes[2].offset = offsetof(Vertex, color);
    vertexDesc.attributes[2].bufferIndex = 0;
    
    vertexDesc.layouts[0].stride = sizeof(Vertex);
    vertexDesc.layouts[0].stepRate = 1;
    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // Pipeline state (solid color)
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunc;
    pipelineDesc.fragmentFunction = fragmentFunc;
    pipelineDesc.vertexDescriptor = vertexDesc;
    pipelineDesc.colorAttachments[0].pixelFormat = m_layer.pixelFormat;
    pipelineDesc.colorAttachments[0].blendingEnabled = YES;
    pipelineDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pipelineDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    pipelineDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    
    m_pipelineState = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!m_pipelineState) {
        NSLog(@"Failed to create pipeline state: %@", error);
    }
    
    // Pipeline state (textured)
    pipelineDesc.fragmentFunction = fragmentTexturedFunc;
    m_texturePipelineState = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!m_texturePipelineState) {
        NSLog(@"Failed to create textured pipeline state: %@", error);
    }
}

void MetalRenderBackend::beginFrame(u32 width, u32 height, f32 devicePixelRatio) {
    m_frameWidth = width;
    m_frameHeight = height;
    m_devicePixelRatio = devicePixelRatio;
    
    m_drawable = [m_layer nextDrawable];
    if (!m_drawable) return;
    
    MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    passDesc.colorAttachments[0].texture = m_drawable.texture;
    passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.2, 0.2, 0.2, 1.0);
    
    m_commandBuffer = [m_commandQueue commandBuffer];
    m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    
    // Set viewport - m_frameWidth/Height are already physical pixels
    MTLViewport viewport = {0, 0, (double)m_frameWidth, (double)m_frameHeight, 0, 1};
    [m_renderEncoder setViewport:viewport];
}

void MetalRenderBackend::endFrame() {
    if (m_renderEncoder) {
        [m_renderEncoder endEncoding];
        m_renderEncoder = nil;
    }
    
    if (m_drawable && m_commandBuffer) {
        [m_commandBuffer presentDrawable:m_drawable];
        [m_commandBuffer commit];
    }
    
    m_commandBuffer = nil;
    m_drawable = nil;
}

void MetalRenderBackend::render(
    std::span<const Vertex> vertices,
    std::span<const u32> indices,
    std::span<const DrawCommand> commands)
{
    if (!m_renderEncoder || vertices.empty() || indices.empty()) return;
    
    // Upload vertex data - scale from logical to physical pixels
    std::vector<Vertex> scaledVertices(vertices.begin(), vertices.end());
    for (auto& v : scaledVertices) {
        v.x *= m_devicePixelRatio;
        v.y *= m_devicePixelRatio;
    }
    memcpy([m_vertexBuffer contents], scaledVertices.data(), scaledVertices.size() * sizeof(Vertex));
    memcpy([m_indexBuffer contents], indices.data(), indices.size() * sizeof(u32));
    
    // Create orthographic projection matrix
    // Vertices from RenderContext are already in physical pixels (0-frameWidth, 0-frameHeight)
    // Convert to NDC: (0,0) -> (-1,1), (frameWidth,frameHeight) -> (1,-1)
    simd::float4x4 projection = {
        simd::make_float4(2.0f / m_frameWidth, 0, 0, 0),
        simd::make_float4(0, -2.0f / m_frameHeight, 0, 0),
        simd::make_float4(0, 0, 1, 0),
        simd::make_float4(-1, 1, 0, 1)
    };
    
    [m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
    [m_renderEncoder setVertexBytes:&projection length:sizeof(projection) atIndex:1];
    
    // Execute draw commands
    for (const auto& cmd : commands) {
        if (cmd.type == DrawCommandType::SetClip) {
            // Scale scissor rect from logical to physical pixels
            MTLScissorRect scissor = {
                (NSUInteger)std::max(0.0f, cmd.clipRect.x * m_devicePixelRatio),
                (NSUInteger)std::max(0.0f, cmd.clipRect.y * m_devicePixelRatio),
                (NSUInteger)std::max(1.0f, cmd.clipRect.width * m_devicePixelRatio),
                (NSUInteger)std::max(1.0f, cmd.clipRect.height * m_devicePixelRatio)
            };
            [m_renderEncoder setScissorRect:scissor];
        }
        else if (cmd.type == DrawCommandType::ClearClip) {
            // Reset scissor to full viewport - m_frameWidth/Height are physical pixels
            MTLScissorRect scissor = {0, 0, m_frameWidth, m_frameHeight};
            [m_renderEncoder setScissorRect:scissor];
        }
        else if (cmd.type == DrawCommandType::DrawTriangles) {
            [m_renderEncoder setRenderPipelineState:m_pipelineState];
            [m_renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                        indexCount:cmd.indexCount
                                         indexType:MTLIndexTypeUInt32
                                       indexBuffer:m_indexBuffer
                                 indexBufferOffset:cmd.indexOffset * sizeof(u32)];
        }
        else if (cmd.type == DrawCommandType::DrawTexturedTriangles) {
            [m_renderEncoder setRenderPipelineState:m_texturePipelineState];
            if (cmd.texture) {
                auto* mt = static_cast<MetalTexture*>(cmd.texture);
                [m_renderEncoder setFragmentTexture:mt->metalTexture() atIndex:0];
            }
            [m_renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                        indexCount:cmd.indexCount
                                         indexType:MTLIndexTypeUInt32
                                       indexBuffer:m_indexBuffer
                                 indexBufferOffset:cmd.indexOffset * sizeof(u32)];
        }
    }
}

Ref<Texture> MetalRenderBackend::createTexture(u32 width, u32 height, const u8* pixels) {
    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                    width:width
                                                                                   height:height
                                                                                mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> tex = [m_device newTextureWithDescriptor:desc];
    if (!tex) return nullptr;
    
    if (pixels) {
        [tex replaceRegion:MTLRegionMake2D(0, 0, width, height)
               mipmapLevel:0
                 withBytes:pixels
               bytesPerRow:width * 4];
    }
    return makeRef<MetalTexture>(tex, width, height);
}

Ref<Texture> MetalRenderBackend::createTextureFromFile(StringView path) {
    // Not implemented — use createTexture with pixel data
    return nullptr;
}

void MetalRenderBackend::updateTexture(Texture& texture, u32 x, u32 y, u32 width, u32 height, const u8* pixels) {
    auto* mt = static_cast<MetalTexture*>(&texture);
    [mt->metalTexture() replaceRegion:MTLRegionMake2D(x, y, width, height)
                          mipmapLevel:0
                            withBytes:pixels
                          bytesPerRow:width * 4];
}

void MetalRenderBackend::destroyTexture(Texture& texture) {
    // ARC handles Metal texture deallocation
}

Size2f MetalRenderBackend::measureText(const TextLayout& layout) {
    // Fallback to simple measurement
    return {static_cast<f32>(layout.text.length()) * 8.0f, 16.0f};
}

void MetalRenderBackend::renderText(
    const TextLayout& layout,
    Point2f position,
    Color color,
    std::vector<Vertex>& vertices,
    std::vector<u32>& indices)
{
    // TODO: Implement text rendering
}
