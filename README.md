# Gut - Game UI Toolkit

A lightweight, single-header C++20 GUI library for games and real-time applications.

## Features

- **Single-header** — just `#include "gut_single.h"`
- **Modern C++20** — concepts, ranges, spans
- **Retained-mode UI** — declarative element tree with automatic layout
- **Font rendering** — built-in TrueType font support via stb_truetype
- **Flexible styling** — CSS-inspired style system with selectors
- **Pluggable rendering** — bring your own backend (OpenGL, Vulkan, Metal, D3D)
- **Animation** — timeline-based animations with 30+ easing functions
- **~9k lines** — small enough to understand, complete enough to use

## Quick Start

```cpp
// In ONE .cpp file:
#define GUT_IMPLEMENTATION
#include "gut_single.h"

// In other files, just:
#include "gut_single.h"
```

### Basic Example

```cpp
#define GUT_IMPLEMENTATION
#include "gut_single.h"

int main() {
    // Create context with your render backend
    auto backend = std::make_unique<MyOpenGLBackend>();
    gut::Context ctx(std::move(backend));
    
    // Build UI
    auto root = gut::make<gut::StackPanel>();
    
    auto label = gut::make<gut::Text>();
    label->setText("Hello, Gut!");
    root->addChild(label);
    
    auto button = gut::make<gut::Button>();
    button->setText("Click Me");
    root->addChild(button);
    
    ctx.setRoot(root);
    
    // Game loop
    while (running) {
        ctx.processMouseMove(mouseX, mouseY);
        ctx.processMouseButton(button, pressed);
        ctx.update(deltaTime);
        ctx.render(width, height);
    }
}
```

## Font Rendering

Gut includes built-in TrueType font rendering powered by stb_truetype. 
You provide the font file data; Gut handles parsing, glyph rasterization, and atlas generation.

```cpp
// Load font data (you handle file I/O)
std::vector<uint8_t> fontData = loadFile("fonts/Roboto-Regular.ttf");

// Create font from memory
auto font = ctx.loadFont(fontData.data(), fontData.size());
if (!font) {
    // Handle error
}

// Get a font face at a specific size
auto face = font->getFace(24.0f);

// Measure text
float width = face->measureWidth("Hello, World!");
gut::Size2f bounds = face->measureText("Hello\nMultiline", maxWidth);

// Draw text directly with RenderContext
ctx.renderBackend().beginFrame(800, 600, 1.0f);
renderContext.drawText(face.get(), "Hello!", {100, 100}, gut::Color::white());
// ...
```

### Font Atlas

Gut automatically generates a texture atlas for each font size. The atlas bitmap
is available for your backend to upload to the GPU:

```cpp
// In your RenderBackend implementation:
Ref<Texture> createAlphaTexture(u32 width, u32 height, const u8* pixels) override {
    // pixels is single-channel (alpha only)
    // Upload to GPU as R8 or convert to RGBA
}
```

The default implementation converts alpha to RGBA, but you can override for efficiency.

## Elements

| Element | Description |
|---------|-------------|
| `Element` | Base class for all UI elements |
| `Panel` | Container base class |
| `StackPanel` | Horizontal/vertical stacking layout |
| `Grid` | Row/column grid layout |
| `Canvas` | Absolute positioning |
| `Button` | Clickable button |
| `Text` | Text display |
| `Image` | Image display |
| `ScrollViewer` | Scrollable container |

## Render Backend

Implement `RenderBackend` to integrate with your graphics API:

```cpp
class MyBackend : public gut::RenderBackend {
public:
    bool initialize() override;
    void shutdown() override;
    void beginFrame(u32 width, u32 height, f32 dpi) override;
    void endFrame() override;
    void render(std::span<const Vertex> verts,
                std::span<const u32> indices,
                std::span<const DrawCommand> cmds) override;
    
    // Textures
    Ref<Texture> createTexture(u32 w, u32 h, const u8* rgba) override;
    Ref<Texture> createAlphaTexture(u32 w, u32 h, const u8* alpha) override;
    // ...
};
```

## Building Examples

```bash
mkdir build && cd build
cmake ..
cmake --build .
./examples/hello_gut
```

## Dependencies

- **C++20 compiler** (GCC 10+, Clang 10+, MSVC 2019+)
- **stb_truetype.h** — included in repo for font rendering

## Design

- **WPF-inspired** — two-pass layout (measure/arrange), property system, signals
- **Game-friendly** — predictable frame times, no hidden allocations in hot paths
- **Embeddable** — integrates into your game loop, not the other way around

## License

MIT License
