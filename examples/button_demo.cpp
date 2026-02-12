/**
 * @file button_demo.cpp
 * @brief Example demonstrating button interaction and signals.
 * 
 * This example shows:
 * - Creating buttons with click handlers
 * - Using signals and slots
 * - Simulating mouse input
 */

#define GUT_IMPLEMENTATION
#include "gut_single.h"
#include <iostream>

int main() {
    std::cout << "Gut Button Demo\n\n";
    
    gut::initialize();
    
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    
    // Track button clicks
    int clickCount = 0;
    
    // Create a button
    auto button = gut::makeRef<gut::Button>();
    button->setlabel("Click Me!");
    button->setwidth(150);
    button->setheight(50);
    button->setmargin({20, 20, 20, 20});
    button->setbackground(gut::Color{100, 150, 200, 255});
    
    // Set click handler
    button->setOnClick([&clickCount]() {
        clickCount++;
        std::cout << "Button clicked! Total clicks: " << clickCount << "\n";
    });
    
    // Create a counter display
    auto counterText = gut::makeRef<gut::Text>();
    counterText->settext("Clicks: 0");
    counterText->setfontSize(24.0f);
    
    // Create a quit button
    bool running = true;
    auto quitButton = gut::makeRef<gut::Button>();
    quitButton->setlabel("Quit");
    quitButton->setwidth(100);
    quitButton->setheight(40);
    quitButton->setbackground(gut::Color{200, 100, 100, 255});
    
    quitButton->setOnClick([&running]() {
        std::cout << "Quit button clicked\n";
        running = false;
    });
    
    // Layout
    auto panel = gut::makeRef<gut::StackPanel>();
    panel->setorientation(gut::Orientation::Vertical);
    panel->sethorizontalAlignment(gut::HorizontalAlignment::Center);
    panel->setverticalAlignment(gut::VerticalAlignment::Center);
    panel->addChild(button);
    panel->addChild(counterText);
    panel->addChild(quitButton);
    
    ctx.setRoot(panel);
    
    // Initial layout
    ctx.update(0);
    ctx.render(800, 600);
    
    // Print element bounds for debugging
    std::cout << "Element layout:\n";
    std::cout << "  Panel: " << panel->bounds().x << "," << panel->bounds().y 
              << " " << panel->bounds().width << "x" << panel->bounds().height << "\n";
    std::cout << "  Button: " << button->bounds().x << "," << button->bounds().y
              << " " << button->bounds().width << "x" << button->bounds().height << "\n";
    auto btnScreen = button->screenBounds();
    std::cout << "  Button screen: " << btnScreen.x << "," << btnScreen.y
              << " " << btnScreen.width << "x" << btnScreen.height << "\n";

    // Simulate some interactions
    std::cout << "\n--- Simulating mouse interactions ---\n";
    
    // Calculate button center in screen coordinates
    float btnCenterX = btnScreen.x + btnScreen.width / 2;
    float btnCenterY = btnScreen.y + btnScreen.height / 2;
    std::cout << "Clicking at button center: " << btnCenterX << ", " << btnCenterY << "\n";
    
    // Move mouse over first button
    ctx.processMouseMove(btnCenterX, btnCenterY);
    ctx.update(16);
    
    // Click the button a few times
    for (int i = 0; i < 3 && running; ++i) {
        std::cout << "\nClick #" << (i + 1) << ":\n";
        ctx.processMouseButton(gut::MouseButton::Left, true);
        ctx.update(16);
        ctx.processMouseButton(gut::MouseButton::Left, false);
        ctx.update(16);
        
        // Update counter text
        counterText->settext("Clicks: " + std::to_string(clickCount));
    }
    
    // Click quit button
    std::cout << "\nClicking quit button:\n";
    auto quitScreen = quitButton->screenBounds();
    float quitCenterX = quitScreen.x + quitScreen.width / 2;
    float quitCenterY = quitScreen.y + quitScreen.height / 2;
    std::cout << "Quit button at: " << quitCenterX << ", " << quitCenterY << "\n";
    
    ctx.processMouseMove(quitCenterX, quitCenterY);
    ctx.update(16);
    ctx.processMouseButton(gut::MouseButton::Left, true);
    ctx.update(16);
    ctx.processMouseButton(gut::MouseButton::Left, false);
    ctx.update(16);
    
    std::cout << "\nDemo completed. Running = " << (running ? "true" : "false") << "\n";
    
    gut::shutdown();
    return 0;
}
