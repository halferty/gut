/**
 * @file animation_demo.cpp
 * @brief Example demonstrating the animation system.
 * 
 * This example shows:
 * - Creating property animations
 * - Using easing functions
 * - Animation timeline management
 * - Storyboards for grouped animations
 */

#define GUT_IMPLEMENTATION
#include "gut_single.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "Gut Animation Demo\n\n";
    
    gut::initialize();
    
    // Create a simple property to animate
    gut::Property<gut::f32> position(0.0f);
    gut::Property<gut::f32> opacity(0.0f);
    gut::Property<gut::Color> color({255, 0, 0, 255});
    
    // Monitor property changes
    position.changed().connect([](const gut::f32& val, const gut::f32&) {
        std::cout << "Position: " << std::fixed << std::setprecision(2) << val << "\n";
    });
    
    // Create a float animation
    std::cout << "--- Basic Float Animation ---\n";
    auto posAnim = gut::makeRef<gut::FloatAnimation>();
    posAnim->setTargetProperty(&position);
    posAnim->setFrom(0.0f);
    posAnim->setTo(100.0f);
    posAnim->setduration(500.0f);  // 500ms
    
    posAnim->completed.connect([]() {
        std::cout << "Position animation completed!\n";
    });
    
    // Start the animation
    posAnim->begin();
    
    // Simulate timeline updates
    auto* timeline = gut::Timeline::global();
    for (int i = 0; i < 10; ++i) {
        timeline->update(60.0f);  // 60ms per step
    }
    
    // Demo easing functions
    std::cout << "\n--- Easing Functions Demo ---\n";
    std::cout << "Testing various easing functions at t=0.5:\n";
    
    std::cout << "  linear:       " << gut::easing::linear(0.5f) << "\n";
    std::cout << "  easeInQuad:   " << gut::easing::easeInQuad(0.5f) << "\n";
    std::cout << "  easeOutQuad:  " << gut::easing::easeOutQuad(0.5f) << "\n";
    std::cout << "  easeInOutQuad:" << gut::easing::easeInOutQuad(0.5f) << "\n";
    std::cout << "  easeInBack:   " << gut::easing::easeInBack(0.5f) << "\n";
    std::cout << "  easeOutBack:  " << gut::easing::easeOutBack(0.5f) << "\n";
    std::cout << "  easeOutBounce:" << gut::easing::easeOutBounce(0.5f) << "\n";
    
    // Cubic bezier easing
    std::cout << "\n--- Cubic Bezier Easing ---\n";
    auto ease = gut::CubicBezierEasing::ease();
    auto easeIn = gut::CubicBezierEasing::easeIn();
    auto easeOut = gut::CubicBezierEasing::easeOut();
    
    std::cout << "CSS 'ease' at t=0.5:     " << ease(0.5f) << "\n";
    std::cout << "CSS 'ease-in' at t=0.5:  " << easeIn(0.5f) << "\n";
    std::cout << "CSS 'ease-out' at t=0.5: " << easeOut(0.5f) << "\n";
    
    // Spring animation
    std::cout << "\n--- Spring Easing ---\n";
    gut::SpringEasing spring(1.0f, 100.0f, 10.0f);
    std::cout << "Spring easing curve:\n";
    for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
        std::cout << "  t=" << std::fixed << std::setprecision(1) << t 
                  << ": " << std::setprecision(3) << spring(t) << "\n";
    }
    
    // Storyboard demo
    std::cout << "\n--- Storyboard Demo ---\n";
    
    position.set(0.0f);
    opacity.set(0.0f);
    
    auto storyboard = gut::makeRef<gut::Storyboard>();
    
    // Position animation
    auto posAnim2 = gut::makeRef<gut::FloatAnimation>();
    posAnim2->setTargetProperty(&position);
    posAnim2->setFrom(0.0f);
    posAnim2->setTo(200.0f);
    posAnim2->setduration(1000.0f);
    
    // Opacity animation with delay
    auto opacityAnim = gut::makeRef<gut::FloatAnimation>();
    opacityAnim->setTargetProperty(&opacity);
    opacityAnim->setFrom(0.0f);
    opacityAnim->setTo(1.0f);
    opacityAnim->setbeginTime(250.0f);  // Start after 250ms
    opacityAnim->setduration(500.0f);
    
    storyboard->addAnimation(posAnim2);
    storyboard->addAnimation(opacityAnim);
    
    storyboard->completed.connect([]() {
        std::cout << "Storyboard completed!\n";
    });
    
    std::cout << "Starting storyboard...\n";
    storyboard->begin();
    
    // Run the timeline
    for (int i = 0; i < 25; ++i) {
        timeline->update(50.0f);
        std::cout << "Frame " << i << ": pos=" << std::fixed << std::setprecision(1) 
                  << position.get() << ", opacity=" << std::setprecision(2) 
                  << opacity.get() << "\n";
    }
    
    std::cout << "\nAnimation demo completed!\n";
    
    gut::shutdown();
    return 0;
}
