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

struct ClipUniforms {
    float4 clipRect;       // x, y, width, height in physical pixels
    float cornerRadius;    // in physical pixels
    float enabled;         // 1.0 = rounded clip active, 0.0 = no rounded clip
    float2 _pad;
};

// Signed distance from point to rounded rect boundary.
// Returns negative inside, positive outside.
float roundedRectSDF(float2 p, float2 rectMin, float2 rectSize, float radius) {
    float2 center = rectMin + rectSize * 0.5;
    float2 halfSize = rectSize * 0.5 - radius;
    float2 d = abs(p - center) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

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

fragment float4 fragment_main(
    VertexOut in [[stage_in]],
    constant ClipUniforms& clip [[buffer(0)]]
) {
    float4 color = in.color;
    if (clip.enabled > 0.5) {
        float dist = roundedRectSDF(in.position.xy, clip.clipRect.xy, clip.clipRect.zw, clip.cornerRadius);
        color.a *= 1.0 - smoothstep(-0.75, 0.75, dist);
    }
    return color;
}

fragment float4 fragment_textured(
    VertexOut in [[stage_in]],
    texture2d<float> colorTexture [[texture(0)]],
    constant ClipUniforms& clip [[buffer(0)]]
) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    float4 color = in.color * texColor;
    if (clip.enabled > 0.5) {
        float dist = roundedRectSDF(in.position.xy, clip.clipRect.xy, clip.clipRect.zw, clip.cornerRadius);
        color.a *= 1.0 - smoothstep(-0.75, 0.75, dist);
    }
    return color;
}
)";

// Kawase blur + composite shaders
static const char* BLUR_SHADERS = R"(
#include <metal_stdlib>
using namespace metal;

struct BlurVertexOut {
    float4 position [[position]];
    float2 texCoord;
};

struct BlurUniforms {
    float2 texelSize;    // 1.0 / textureSize
    float offset;        // Kawase kernel offset (increases each pass)
    float _pad;
};

// Full-screen triangle trick: 3 vertices cover the screen
vertex BlurVertexOut blur_vertex(uint vid [[vertex_id]]) {
    BlurVertexOut out;
    // Generate a full-screen triangle from vertex ID
    out.texCoord = float2((vid << 1) & 2, vid & 2);
    out.position = float4(out.texCoord * float2(2, -2) + float2(-1, 1), 0, 1);
    return out;
}

// Kawase blur kernel: samples 4 diagonal neighbors at increasing offsets
fragment float4 blur_fragment(
    BlurVertexOut in [[stage_in]],
    texture2d<float> srcTexture [[texture(0)]],
    constant BlurUniforms& uniforms [[buffer(0)]]
) {
    constexpr sampler s(mag_filter::linear, min_filter::linear, address::clamp_to_edge);
    float2 uv = in.texCoord;
    float2 off = uniforms.texelSize * (uniforms.offset + 0.5);
    
    float4 sum = srcTexture.sample(s, uv + float2(-off.x, -off.y));
    sum += srcTexture.sample(s, uv + float2( off.x, -off.y));
    sum += srcTexture.sample(s, uv + float2(-off.x,  off.y));
    sum += srcTexture.sample(s, uv + float2( off.x,  off.y));
    
    return sum * 0.25;
}

// Composite: draw blurred region with tint and rounded-rect mask
struct CompositeVertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    float4 color [[attribute(2)]];
};

struct CompositeVertexOut {
    float4 position [[position]];
    float2 texCoord;
    float4 tintColor;
};

struct CompositeUniforms {
    float4x4 projection;
    float4 blurRect;       // screen-space rect (x, y, w, h) in physical px
    float cornerRadius;
    float _pad[3];
};

float roundedRectSDF_c(float2 p, float2 rectMin, float2 rectSize, float radius) {
    float2 center = rectMin + rectSize * 0.5;
    float2 halfSize = rectSize * 0.5 - radius;
    float2 d = abs(p - center) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

vertex CompositeVertexOut composite_vertex(
    CompositeVertexIn in [[stage_in]],
    constant CompositeUniforms& uniforms [[buffer(1)]]
) {
    CompositeVertexOut out;
    out.position = uniforms.projection * float4(in.position, 0.0, 1.0);
    out.texCoord = in.texCoord;
    out.tintColor = in.color;
    return out;
}

fragment float4 composite_fragment(
    CompositeVertexOut in [[stage_in]],
    texture2d<float> blurredTexture [[texture(0)]],
    constant CompositeUniforms& uniforms [[buffer(1)]]
) {
    constexpr sampler s(mag_filter::linear, min_filter::linear, address::clamp_to_edge);
    float4 blurred = blurredTexture.sample(s, in.texCoord);
    
    // Apply tint: blend tint color over the blurred backdrop
    float4 tint = in.tintColor;
    float4 result = blurred * (1.0 - tint.a) + tint * tint.a;
    result.a = 1.0;
    
    // Apply rounded-rect mask
    if (uniforms.cornerRadius > 0) {
        float dist = roundedRectSDF_c(in.position.xy, uniforms.blurRect.xy, uniforms.blurRect.zw, uniforms.cornerRadius);
        result.a *= 1.0 - smoothstep(-0.75, 0.75, dist);
    }
    
    return result;
}
)";

// 3D spinning cube shaders
static const char* CUBE_3D_SHADERS = R"(
#include <metal_stdlib>
using namespace metal;

struct CubeVertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float4 color    [[attribute(2)]];
};

struct CubeVertexOut {
    float4 position [[position]];
    float4 color;
    float3 worldNormal;
};

struct CubeUniforms {
    float4x4 mvp;
    float4x4 model;
};

vertex CubeVertexOut cube3d_vertex(CubeVertexIn in [[stage_in]],
                                   constant CubeUniforms& u [[buffer(1)]])
{
    CubeVertexOut out;
    out.position = u.mvp * float4(in.position, 1.0);
    out.color = in.color;
    out.worldNormal = (u.model * float4(in.normal, 0.0)).xyz;
    return out;
}

fragment float4 cube3d_fragment(CubeVertexOut in [[stage_in]])
{
    float3 lightDir = normalize(float3(0.5, 1.0, -0.7));
    float3 n = normalize(in.worldNormal);
    float ndotl = max(dot(n, lightDir), 0.0);
    // Ambient + diffuse + subtle rim light
    float rim = pow(1.0 - max(dot(n, float3(0, 0, 1)), 0.0), 3.0) * 0.15;
    float3 color = in.color.rgb * (0.25 + 0.75 * ndotl) + rim;
    return float4(color, 1.0);
}
)";

// ---------- Matrix helpers for 3D rendering ----------
static simd::float4x4 mat4_perspective(float fovYRadians, float aspect, float nearZ, float farZ) {
    float f = 1.0f / tanf(fovYRadians * 0.5f);
    return {
        simd::make_float4(f / aspect, 0, 0, 0),
        simd::make_float4(0, f, 0, 0),
        simd::make_float4(0, 0, farZ / (nearZ - farZ), -1),
        simd::make_float4(0, 0, nearZ * farZ / (nearZ - farZ), 0)
    };
}

static simd::float4x4 mat4_translation(float x, float y, float z) {
    return {
        simd::make_float4(1, 0, 0, 0),
        simd::make_float4(0, 1, 0, 0),
        simd::make_float4(0, 0, 1, 0),
        simd::make_float4(x, y, z, 1)
    };
}

static simd::float4x4 mat4_rotationX(float angle) {
    float c = cosf(angle), s = sinf(angle);
    return {
        simd::make_float4(1, 0, 0, 0),
        simd::make_float4(0, c, s, 0),
        simd::make_float4(0, -s, c, 0),
        simd::make_float4(0, 0, 0, 1)
    };
}

static simd::float4x4 mat4_rotationY(float angle) {
    float c = cosf(angle), s = sinf(angle);
    return {
        simd::make_float4(c, 0, -s, 0),
        simd::make_float4(0, 1, 0, 0),
        simd::make_float4(s, 0, c, 0),
        simd::make_float4(0, 0, 0, 1)
    };
}

static simd::float4x4 mat4_rotationZ(float angle) {
    float c = cosf(angle), s = sinf(angle);
    return {
        simd::make_float4(c, s, 0, 0),
        simd::make_float4(-s, c, 0, 0),
        simd::make_float4(0, 0, 1, 0),
        simd::make_float4(0, 0, 0, 1)
    };
}

MetalRenderBackend::MetalRenderBackend(CAMetalLayer* layer)
    : m_layer(layer)
{
    m_clipUniforms = {{0, 0, 0, 0}, 0, 0, {0, 0}};
    m_currentScissor = {0, 0, 0, 0};
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
    createBlurResources();
    create3DResources();
    
    NSLog(@"MetalRenderBackend::initialize() complete");
    return true;
}

void MetalRenderBackend::shutdown() {
    m_vertexBuffer = nil;
    m_indexBuffer = nil;
    m_pipelineState = nil;
    m_texturePipelineState = nil;
    m_blurPipelineState = nil;
    m_blurCompositePipelineState = nil;
    m_blurTexA = nil;
    m_blurTexB = nil;
    m_cubePipelineState = nil;
    m_cubeDepthState = nil;
    m_cubeVertexBuffer = nil;
    m_cubeIndexBuffer = nil;
    m_cubeDepthTexture = nil;
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

void MetalRenderBackend::createBlurResources() {
    NSError* error = nil;
    
    NSString* blurSource = [NSString stringWithUTF8String:BLUR_SHADERS];
    id<MTLLibrary> blurLib = [m_device newLibraryWithSource:blurSource options:nil error:&error];
    if (!blurLib) {
        NSLog(@"Failed to compile blur shaders: %@", error);
        return;
    }
    
    id<MTLFunction> blurVertexFunc = [blurLib newFunctionWithName:@"blur_vertex"];
    id<MTLFunction> blurFragFunc = [blurLib newFunctionWithName:@"blur_fragment"];
    
    // Blur pipeline — no vertex descriptor (fullscreen triangle from vertex_id)
    {
        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexFunction = blurVertexFunc;
        desc.fragmentFunction = blurFragFunc;
        desc.colorAttachments[0].pixelFormat = m_layer.pixelFormat;
        desc.colorAttachments[0].blendingEnabled = NO;
        
        m_blurPipelineState = [m_device newRenderPipelineStateWithDescriptor:desc error:&error];
        if (!m_blurPipelineState) {
            NSLog(@"Failed to create blur pipeline: %@", error);
        }
    }
    
    // Composite pipeline — uses same vertex descriptor as main pipelines
    {
        id<MTLFunction> compVertexFunc = [blurLib newFunctionWithName:@"composite_vertex"];
        id<MTLFunction> compFragFunc = [blurLib newFunctionWithName:@"composite_fragment"];
        
        MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
        vertexDesc.attributes[0].format = MTLVertexFormatFloat2;
        vertexDesc.attributes[0].offset = offsetof(Vertex, x);
        vertexDesc.attributes[0].bufferIndex = 0;
        vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
        vertexDesc.attributes[1].offset = offsetof(Vertex, u);
        vertexDesc.attributes[1].bufferIndex = 0;
        vertexDesc.attributes[2].format = MTLVertexFormatUChar4Normalized;
        vertexDesc.attributes[2].offset = offsetof(Vertex, color);
        vertexDesc.attributes[2].bufferIndex = 0;
        vertexDesc.layouts[0].stride = sizeof(Vertex);
        vertexDesc.layouts[0].stepRate = 1;
        vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        
        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexFunction = compVertexFunc;
        desc.fragmentFunction = compFragFunc;
        desc.vertexDescriptor = vertexDesc;
        desc.colorAttachments[0].pixelFormat = m_layer.pixelFormat;
        desc.colorAttachments[0].blendingEnabled = YES;
        desc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        desc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        m_blurCompositePipelineState = [m_device newRenderPipelineStateWithDescriptor:desc error:&error];
        if (!m_blurCompositePipelineState) {
            NSLog(@"Failed to create blur composite pipeline: %@", error);
        }
    }
    
    NSLog(@"Blur resources created successfully");
}

void MetalRenderBackend::ensureBlurTextures(u32 width, u32 height) {
    if (width == m_blurTexWidth && height == m_blurTexHeight && m_blurTexA && m_blurTexB) return;
    
    m_blurTexWidth = width;
    m_blurTexHeight = height;
    
    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:m_layer.pixelFormat
                                                                                   width:width
                                                                                  height:height
                                                                               mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    desc.storageMode = MTLStorageModePrivate;
    
    m_blurTexA = [m_device newTextureWithDescriptor:desc];
    m_blurTexB = [m_device newTextureWithDescriptor:desc];
    
    m_blurTexA.label = @"BlurTexA";
    m_blurTexB.label = @"BlurTexB";
}

// ---------- 3D Spinning Cube ----------

struct CubeVertex3D {
    simd::float3 position;
    simd::float3 normal;
    simd::float4 color;
};

void MetalRenderBackend::create3DResources() {
    NSError* error = nil;
    
    // Compile 3D shaders
    NSString* cubeSource = [NSString stringWithUTF8String:CUBE_3D_SHADERS];
    id<MTLLibrary> cubeLib = [m_device newLibraryWithSource:cubeSource options:nil error:&error];
    if (!cubeLib) {
        NSLog(@"Failed to compile cube shaders: %@", error);
        return;
    }
    
    id<MTLFunction> cubeVertFunc = [cubeLib newFunctionWithName:@"cube3d_vertex"];
    id<MTLFunction> cubeFragFunc = [cubeLib newFunctionWithName:@"cube3d_fragment"];
    
    // Vertex descriptor for CubeVertex3D
    MTLVertexDescriptor* vd = [[MTLVertexDescriptor alloc] init];
    vd.attributes[0].format = MTLVertexFormatFloat3;   // position
    vd.attributes[0].offset = offsetof(CubeVertex3D, position);
    vd.attributes[0].bufferIndex = 0;
    vd.attributes[1].format = MTLVertexFormatFloat3;   // normal
    vd.attributes[1].offset = offsetof(CubeVertex3D, normal);
    vd.attributes[1].bufferIndex = 0;
    vd.attributes[2].format = MTLVertexFormatFloat4;   // color
    vd.attributes[2].offset = offsetof(CubeVertex3D, color);
    vd.attributes[2].bufferIndex = 0;
    vd.layouts[0].stride = sizeof(CubeVertex3D);
    vd.layouts[0].stepRate = 1;
    vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // Pipeline
    MTLRenderPipelineDescriptor* pd = [[MTLRenderPipelineDescriptor alloc] init];
    pd.vertexFunction = cubeVertFunc;
    pd.fragmentFunction = cubeFragFunc;
    pd.vertexDescriptor = vd;
    pd.colorAttachments[0].pixelFormat = m_layer.pixelFormat;
    pd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    
    m_cubePipelineState = [m_device newRenderPipelineStateWithDescriptor:pd error:&error];
    if (!m_cubePipelineState) {
        NSLog(@"Failed to create cube pipeline: %@", error);
        return;
    }
    
    // Depth stencil state
    MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
    dsd.depthCompareFunction = MTLCompareFunctionLess;
    dsd.depthWriteEnabled = YES;
    m_cubeDepthState = [m_device newDepthStencilStateWithDescriptor:dsd];
    
    // Cube geometry: 24 vertices (4 per face), 36 indices
    CubeVertex3D verts[] = {
        // Front (z = +0.5) — Ember red
        {{-0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {0.90f, 0.25f, 0.15f, 1}},
        {{ 0.5f, -0.5f,  0.5f}, { 0, 0, 1}, {0.90f, 0.25f, 0.15f, 1}},
        {{ 0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {0.90f, 0.25f, 0.15f, 1}},
        {{-0.5f,  0.5f,  0.5f}, { 0, 0, 1}, {0.90f, 0.25f, 0.15f, 1}},
        // Back (z = -0.5) — Emerald green
        {{ 0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {0.15f, 0.70f, 0.35f, 1}},
        {{-0.5f, -0.5f, -0.5f}, { 0, 0,-1}, {0.15f, 0.70f, 0.35f, 1}},
        {{-0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {0.15f, 0.70f, 0.35f, 1}},
        {{ 0.5f,  0.5f, -0.5f}, { 0, 0,-1}, {0.15f, 0.70f, 0.35f, 1}},
        // Top (y = +0.5) — Gold
        {{-0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {0.95f, 0.80f, 0.20f, 1}},
        {{ 0.5f,  0.5f,  0.5f}, { 0, 1, 0}, {0.95f, 0.80f, 0.20f, 1}},
        {{ 0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {0.95f, 0.80f, 0.20f, 1}},
        {{-0.5f,  0.5f, -0.5f}, { 0, 1, 0}, {0.95f, 0.80f, 0.20f, 1}},
        // Bottom (y = -0.5) — Deep purple
        {{-0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {0.50f, 0.20f, 0.80f, 1}},
        {{ 0.5f, -0.5f, -0.5f}, { 0,-1, 0}, {0.50f, 0.20f, 0.80f, 1}},
        {{ 0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {0.50f, 0.20f, 0.80f, 1}},
        {{-0.5f, -0.5f,  0.5f}, { 0,-1, 0}, {0.50f, 0.20f, 0.80f, 1}},
        // Left (x = -0.5) — Ocean blue
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0.20f, 0.50f, 0.90f, 1}},
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {0.20f, 0.50f, 0.90f, 1}},
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {0.20f, 0.50f, 0.90f, 1}},
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0.20f, 0.50f, 0.90f, 1}},
        // Right (x = +0.5) — Hot pink
        {{ 0.5f, -0.5f,  0.5f}, { 1, 0, 0}, {0.90f, 0.30f, 0.65f, 1}},
        {{ 0.5f, -0.5f, -0.5f}, { 1, 0, 0}, {0.90f, 0.30f, 0.65f, 1}},
        {{ 0.5f,  0.5f, -0.5f}, { 1, 0, 0}, {0.90f, 0.30f, 0.65f, 1}},
        {{ 0.5f,  0.5f,  0.5f}, { 1, 0, 0}, {0.90f, 0.30f, 0.65f, 1}},
    };
    
    uint32_t idx[] = {
         0, 1, 2,  0, 2, 3,   // front
         4, 5, 6,  4, 6, 7,   // back
         8, 9,10,  8,10,11,   // top
        12,13,14, 12,14,15,   // bottom
        16,17,18, 16,18,19,   // left
        20,21,22, 20,22,23,   // right
    };
    
    m_cubeVertexBuffer = [m_device newBufferWithBytes:verts
                                              length:sizeof(verts)
                                             options:MTLResourceStorageModeShared];
    m_cubeIndexBuffer = [m_device newBufferWithBytes:idx
                                             length:sizeof(idx)
                                            options:MTLResourceStorageModeShared];
    
    NSLog(@"3D cube resources created successfully");
}

void MetalRenderBackend::ensureDepthTexture(u32 width, u32 height) {
    if (width == m_depthTexWidth && height == m_depthTexHeight && m_cubeDepthTexture) return;
    
    m_depthTexWidth = width;
    m_depthTexHeight = height;
    
    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                   width:width
                                                                                  height:height
                                                                               mipmapped:NO];
    desc.usage = MTLTextureUsageRenderTarget;
    desc.storageMode = MTLStorageModePrivate;
    
    m_cubeDepthTexture = [m_device newTextureWithDescriptor:desc];
    m_cubeDepthTexture.label = @"CubeDepth";
}

void MetalRenderBackend::render3DCube(id<MTLRenderCommandEncoder> encoder) {
    CFTimeInterval time = CACurrentMediaTime();
    
    // Camera & projection
    float aspect = (float)m_frameWidth / (float)m_frameHeight;
    simd::float4x4 proj = mat4_perspective(60.0f * M_PI / 180.0f, aspect, 0.1f, 100.0f);
    simd::float4x4 view = mat4_translation(0, 0, -2.8f);
    
    // Model rotation: gentle tumble on multiple axes
    float t = (float)time;
    simd::float4x4 model = simd_mul(mat4_rotationY(t * 0.7f),
                            simd_mul(mat4_rotationX(t * 0.5f),
                                     mat4_rotationZ(t * 0.3f)));
    
    // Slight vertical bob
    float bob = sinf(t * 1.2f) * 0.08f;
    simd::float4x4 bobM = mat4_translation(0, bob, 0);
    model = simd_mul(bobM, model);
    
    simd::float4x4 mvp = simd_mul(proj, simd_mul(view, model));
    
    struct {
        simd::float4x4 mvp;
        simd::float4x4 model;
    } uniforms;
    uniforms.mvp = mvp;
    uniforms.model = model;
    
    [encoder setRenderPipelineState:m_cubePipelineState];
    [encoder setDepthStencilState:m_cubeDepthState];
    [encoder setCullMode:MTLCullModeBack];
    [encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [encoder setVertexBuffer:m_cubeVertexBuffer offset:0 atIndex:0];
    [encoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:1];
    
    [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:36
                         indexType:MTLIndexTypeUInt32
                       indexBuffer:m_cubeIndexBuffer
                 indexBufferOffset:0];
}

void MetalRenderBackend::enableScene3D(bool enabled) {
    m_scene3DEnabled = enabled;
}

void MetalRenderBackend::resumeMainRenderPass() {
    // Resume main render pass with LoadAction::Load to preserve existing content
    MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    passDesc.colorAttachments[0].texture = m_drawable.texture;
    passDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    
    // Restore viewport
    MTLViewport viewport = {0, 0, (double)m_frameWidth, (double)m_frameHeight, 0, 1};
    [m_renderEncoder setViewport:viewport];
}

void MetalRenderBackend::executeBackdropBlur(
    const DrawCommand& cmd,
    const simd::float4x4& projection)
{
    if (!m_blurPipelineState || !m_blurCompositePipelineState) return;
    if (!m_blurTexA || !m_blurTexB) return;
    
    // 1. End current render encoder so we can use the drawable's contents
    [m_renderEncoder endEncoding];
    m_renderEncoder = nil;
    
    // 2. Blit drawable to blurTexA (our starting point)
    id<MTLBlitCommandEncoder> blit = [m_commandBuffer blitCommandEncoder];
    [blit copyFromTexture:m_drawable.texture
              sourceSlice:0
              sourceLevel:0
             sourceOrigin:MTLOriginMake(0, 0, 0)
               sourceSize:MTLSizeMake(m_blurTexWidth, m_blurTexHeight, 1)
                toTexture:m_blurTexA
         destinationSlice:0
         destinationLevel:0
        destinationOrigin:MTLOriginMake(0, 0, 0)];
    [blit endEncoding];
    
    // 3. Multi-pass Kawase blur: ping-pong between texA and texB
    //    Number of passes scales with blur radius
    f32 radius = cmd.blurRadius * m_devicePixelRatio;
    int numPasses = std::max(2, std::min(8, (int)(radius / 4.0f)));
    
    struct BlurUniforms {
        simd::float2 texelSize;
        float offset;
        float _pad;
    };
    
    id<MTLTexture> srcTex = m_blurTexA;
    id<MTLTexture> dstTex = m_blurTexB;
    
    for (int pass = 0; pass < numPasses; pass++) {
        MTLRenderPassDescriptor* blurPass = [MTLRenderPassDescriptor renderPassDescriptor];
        blurPass.colorAttachments[0].texture = dstTex;
        blurPass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        blurPass.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        id<MTLRenderCommandEncoder> blurEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:blurPass];
        [blurEncoder setRenderPipelineState:m_blurPipelineState];
        [blurEncoder setFragmentTexture:srcTex atIndex:0];
        
        BlurUniforms bu;
        bu.texelSize = {1.0f / m_blurTexWidth, 1.0f / m_blurTexHeight};
        bu.offset = (float)(pass + 1);
        bu._pad = 0;
        [blurEncoder setFragmentBytes:&bu length:sizeof(bu) atIndex:0];
        
        // Draw fullscreen triangle (3 vertices, no vertex buffer)
        [blurEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [blurEncoder endEncoding];
        
        // Swap ping-pong
        std::swap(srcTex, dstTex);
    }
    
    // After the loop, srcTex holds the final blurred result
    
    // 4. Resume the main render pass
    resumeMainRenderPass();
    
    // 5. Restore state: vertex buffer, projection, clip
    [m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
    [m_renderEncoder setVertexBytes:&projection length:sizeof(projection) atIndex:1];
    [m_renderEncoder setFragmentBytes:&m_clipUniforms length:sizeof(m_clipUniforms) atIndex:0];
    if (m_hasCustomScissor) {
        [m_renderEncoder setScissorRect:m_currentScissor];
    }
    
    // 6. Draw a quad with the blurred texture using the composite shader
    //    Build a quad that covers the blur rect area in screen space
    f32 rx = cmd.blurRect.x * m_devicePixelRatio;
    f32 ry = cmd.blurRect.y * m_devicePixelRatio;
    f32 rw = cmd.blurRect.width * m_devicePixelRatio;
    f32 rh = cmd.blurRect.height * m_devicePixelRatio;
    
    NSLog(@"[BLUR DBG] blurRect logical: (%.1f, %.1f, %.1f, %.1f)", cmd.blurRect.x, cmd.blurRect.y, cmd.blurRect.width, cmd.blurRect.height);
    NSLog(@"[BLUR DBG] physical rect: (%.1f, %.1f, %.1f, %.1f) dpr=%.1f", rx, ry, rw, rh, m_devicePixelRatio);
    NSLog(@"[BLUR DBG] blurTexSize: %u x %u  frameSize: %u x %u", m_blurTexWidth, m_blurTexHeight, m_frameWidth, m_frameHeight);
    
    // UV coordinates: map the blur rect in screen space to texture UV
    f32 u0 = rx / m_blurTexWidth;
    f32 v0 = ry / m_blurTexHeight;
    f32 u1 = (rx + rw) / m_blurTexWidth;
    f32 v1 = (ry + rh) / m_blurTexHeight;
    
    // Pack tint color
    Color tint = cmd.blurTint;
    u32 packedTint = ((u32)(tint.a * 255) << 24) |
                     ((u32)(tint.b * 255) << 16) |
                     ((u32)(tint.g * 255) << 8)  |
                     ((u32)(tint.r * 255));
    
    // 4 corner vertices: x, y, u, v, color
    Vertex quadVerts[4] = {
        {rx,      ry,      u0, v0, packedTint},  // top-left
        {rx + rw, ry,      u1, v0, packedTint},  // top-right
        {rx + rw, ry + rh, u1, v1, packedTint},  // bottom-right
        {rx,      ry + rh, u0, v1, packedTint},  // bottom-left
    };
    u32 quadIdx[6] = {0, 1, 2, 0, 2, 3};
    
    NSLog(@"[BLUR DBG] quad v0=(%.1f, %.1f uv=%.3f,%.3f)", quadVerts[0].x, quadVerts[0].y, quadVerts[0].u, quadVerts[0].v);
    NSLog(@"[BLUR DBG] quad v1=(%.1f, %.1f uv=%.3f,%.3f)", quadVerts[1].x, quadVerts[1].y, quadVerts[1].u, quadVerts[1].v);
    NSLog(@"[BLUR DBG] quad v2=(%.1f, %.1f uv=%.3f,%.3f)", quadVerts[2].x, quadVerts[2].y, quadVerts[2].u, quadVerts[2].v);
    NSLog(@"[BLUR DBG] quad v3=(%.1f, %.1f uv=%.3f,%.3f)", quadVerts[3].x, quadVerts[3].y, quadVerts[3].u, quadVerts[3].v);
    NSLog(@"[BLUR DBG] projection: [%.3f, %.3f, %.3f, %.3f]", projection.columns[0][0], projection.columns[1][1], projection.columns[3][0], projection.columns[3][1]);
    
    id<MTLBuffer> quadVB = [m_device newBufferWithBytes:quadVerts
                                                 length:sizeof(quadVerts)
                                                options:MTLResourceStorageModeShared];
    id<MTLBuffer> quadIB = [m_device newBufferWithBytes:quadIdx
                                                 length:sizeof(quadIdx)
                                                options:MTLResourceStorageModeShared];
    
    // Composite uniforms
    struct {
        simd::float4x4 projection;
        simd::float4 blurRect;
        float cornerRadius;
        float _pad[3];
    } compositeUniforms;
    compositeUniforms.projection = projection;
    compositeUniforms.blurRect = {rx, ry, rw, rh};
    compositeUniforms.cornerRadius = cmd.blurCornerRadius * m_devicePixelRatio;
    compositeUniforms._pad[0] = compositeUniforms._pad[1] = compositeUniforms._pad[2] = 0;
    
    [m_renderEncoder setRenderPipelineState:m_blurCompositePipelineState];
    [m_renderEncoder setVertexBuffer:quadVB offset:0 atIndex:0];
    [m_renderEncoder setVertexBytes:&compositeUniforms length:sizeof(compositeUniforms) atIndex:1];
    [m_renderEncoder setFragmentBytes:&compositeUniforms length:sizeof(compositeUniforms) atIndex:1];
    [m_renderEncoder setFragmentTexture:srcTex atIndex:0];
    
    [m_renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexCount:6
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:quadIB
                         indexBufferOffset:0];
    
    // 7. Restore the original vertex buffer for subsequent draws
    [m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
}

void MetalRenderBackend::beginFrame(u32 width, u32 height, f32 devicePixelRatio) {
    m_frameWidth = width;
    m_frameHeight = height;
    m_devicePixelRatio = devicePixelRatio;
    
    m_drawable = [m_layer nextDrawable];
    if (!m_drawable) return;
    
    // Ensure blur textures match frame size
    ensureBlurTextures(m_frameWidth, m_frameHeight);
    
    // Reset clip state
    m_clipUniforms = {{0, 0, 0, 0}, 0, 0, {0, 0}};
    m_currentScissor = {0, 0, m_frameWidth, m_frameHeight};
    m_hasCustomScissor = false;
    
    m_commandBuffer = [m_commandQueue commandBuffer];
    
    if (m_scene3DEnabled && m_cubePipelineState) {
        // --- 3D Scene Pass: render spinning cube with depth ---
        ensureDepthTexture(m_frameWidth, m_frameHeight);
        
        MTLRenderPassDescriptor* cubePass = [MTLRenderPassDescriptor renderPassDescriptor];
        cubePass.colorAttachments[0].texture = m_drawable.texture;
        cubePass.colorAttachments[0].loadAction = MTLLoadActionClear;
        cubePass.colorAttachments[0].storeAction = MTLStoreActionStore;
        cubePass.colorAttachments[0].clearColor = MTLClearColorMake(0.04, 0.05, 0.09, 1.0);
        cubePass.depthAttachment.texture = m_cubeDepthTexture;
        cubePass.depthAttachment.loadAction = MTLLoadActionClear;
        cubePass.depthAttachment.storeAction = MTLStoreActionDontCare;
        cubePass.depthAttachment.clearDepth = 1.0;
        
        id<MTLRenderCommandEncoder> cubeEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:cubePass];
        MTLViewport vp = {0, 0, (double)m_frameWidth, (double)m_frameHeight, 0, 1};
        [cubeEncoder setViewport:vp];
        
        render3DCube(cubeEncoder);
        
        [cubeEncoder endEncoding];
        
        // --- 2D UI Pass: preserves the cube content ---
        MTLRenderPassDescriptor* uiPass = [MTLRenderPassDescriptor renderPassDescriptor];
        uiPass.colorAttachments[0].texture = m_drawable.texture;
        uiPass.colorAttachments[0].loadAction = MTLLoadActionLoad;
        uiPass.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:uiPass];
    } else {
        // --- Normal path: just clear ---
        MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        passDesc.colorAttachments[0].texture = m_drawable.texture;
        passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        passDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.45, 0.45, 0.50, 1.0);
        
        m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    }
    
    // Set viewport
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
    if (!m_renderEncoder || commands.empty()) return;
    
    // Upload vertex data - scale from logical to physical pixels
    if (!vertices.empty()) {
        std::vector<Vertex> scaledVertices(vertices.begin(), vertices.end());
        for (auto& v : scaledVertices) {
            v.x *= m_devicePixelRatio;
            v.y *= m_devicePixelRatio;
        }
        memcpy([m_vertexBuffer contents], scaledVertices.data(), scaledVertices.size() * sizeof(Vertex));
    }
    if (!indices.empty()) {
        memcpy([m_indexBuffer contents], indices.data(), indices.size() * sizeof(u32));
    }
    
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
    
    // Reset clip state for this render batch
    m_clipUniforms = {{0, 0, 0, 0}, 0, 0, {0, 0}};
    [m_renderEncoder setFragmentBytes:&m_clipUniforms length:sizeof(m_clipUniforms) atIndex:0];
    
    // Execute draw commands
    for (const auto& cmd : commands) {
        if (cmd.type == DrawCommandType::SetClip) {
            // Always apply scissor rect (axis-aligned bounding box)
            m_currentScissor = {
                (NSUInteger)std::max(0.0f, cmd.clipRect.x * m_devicePixelRatio),
                (NSUInteger)std::max(0.0f, cmd.clipRect.y * m_devicePixelRatio),
                (NSUInteger)std::max(1.0f, cmd.clipRect.width * m_devicePixelRatio),
                (NSUInteger)std::max(1.0f, cmd.clipRect.height * m_devicePixelRatio)
            };
            m_hasCustomScissor = true;
            [m_renderEncoder setScissorRect:m_currentScissor];
            
            // If there's a corner radius, enable fragment-shader SDF clipping
            if (cmd.clipCornerRadius > 0) {
                m_clipUniforms.clipRect = {
                    cmd.clipRect.x * m_devicePixelRatio,
                    cmd.clipRect.y * m_devicePixelRatio,
                    cmd.clipRect.width * m_devicePixelRatio,
                    cmd.clipRect.height * m_devicePixelRatio
                };
                m_clipUniforms.cornerRadius = cmd.clipCornerRadius * m_devicePixelRatio;
                m_clipUniforms.enabled = 1.0f;
            } else {
                m_clipUniforms.enabled = 0.0f;
            }
            [m_renderEncoder setFragmentBytes:&m_clipUniforms length:sizeof(m_clipUniforms) atIndex:0];
        }
        else if (cmd.type == DrawCommandType::ClearClip) {
            // Reset scissor to full viewport
            m_currentScissor = {0, 0, m_frameWidth, m_frameHeight};
            m_hasCustomScissor = false;
            [m_renderEncoder setScissorRect:m_currentScissor];
            
            // Disable rounded clipping
            m_clipUniforms.enabled = 0.0f;
            [m_renderEncoder setFragmentBytes:&m_clipUniforms length:sizeof(m_clipUniforms) atIndex:0];
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
        else if (cmd.type == DrawCommandType::DrawBackdropBlur) {
            executeBackdropBlur(cmd, projection);
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
