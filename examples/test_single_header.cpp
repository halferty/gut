// Test single-header build
#define GUT_IMPLEMENTATION
#include "gut_single.h"

#include <iostream>

int main() {
    std::cout << "Gut Single Header v" << gut::versionString() << std::endl;
    
    // Create a simple UI with a null backend
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    
    auto root = gut::make<gut::StackPanel>();
    
    auto btn = gut::make<gut::Button>();
    root->addChild(btn);
    
    ctx.setRoot(root);
    ctx.update(0.016f);
    ctx.render(800, 600);
    
    std::cout << "Single-header test passed!" << std::endl;
    return 0;
}
