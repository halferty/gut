/**
 * @file hello_gut.cpp
 * @brief Basic "Hello World" example demonstrating gut initialization and basic UI.
 * 
 * This example shows:
 * - Initializing the gut library
 * - Creating a simple UI tree
 * - Running the main loop (simulated)
 */

#define GUT_IMPLEMENTATION
#include "gut_single.h"
#include <iostream>

// Example null backend for demonstration
class DemoBackend : public gut::NullRenderBackend {
public:
    void beginFrame(gut::u32 width, gut::u32 height, gut::f32 dpr) override {
        std::cout << "Begin frame: " << width << "x" << height << " @" << dpr << "x\n";
    }
    
    void endFrame() override {
        std::cout << "End frame\n";
    }
};

int main() {
    std::cout << "Gut Game UI Toolkit v" << gut::versionString() << "\n\n";
    
    // Initialize gut
    if (!gut::initialize()) {
        std::cerr << "Failed to initialize gut\n";
        return 1;
    }
    
    // Create the render backend
    auto backend = std::make_unique<DemoBackend>();
    
    // Create the context
    gut::Context ctx(std::move(backend));
    
    // Create UI elements
    auto helloText = gut::makeRef<gut::Text>();
    helloText->settext("Hello, Gut!");
    helloText->setfontSize(32.0f);
    helloText->setforeground(gut::Color{255, 255, 255, 255});
    
    auto subtitleText = gut::makeRef<gut::Text>();
    subtitleText->settext("A modern C++ GUI toolkit for games");
    subtitleText->setfontSize(16.0f);
    subtitleText->setforeground(gut::Color{200, 200, 200, 255});
    
    // Create a vertical stack panel
    auto panel = gut::makeRef<gut::StackPanel>();
    panel->setorientation(gut::Orientation::Vertical);
    panel->sethorizontalAlignment(gut::HorizontalAlignment::Center);
    panel->setverticalAlignment(gut::VerticalAlignment::Center);
    panel->addChild(helloText);
    panel->addChild(subtitleText);
    
    // Set as root
    ctx.setRoot(panel);
    
    // Simulate a few frames
    for (int frame = 0; frame < 3; ++frame) {
        std::cout << "\n--- Frame " << frame << " ---\n";
        
        // Update (with 16ms delta for ~60fps)
        ctx.update(16.0f);
        
        // Render
        ctx.render(800, 600);
    }
    
    std::cout << "\nExample completed successfully!\n";
    
    // Shutdown
    gut::shutdown();
    
    return 0;
}
