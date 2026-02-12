/**
 * @file layout_demo.cpp
 * @brief Example demonstrating different layout panels.
 * 
 * This example shows:
 * - StackPanel for horizontal/vertical stacking
 * - Grid layout with rows and columns
 * - Canvas for absolute positioning
 * - Nested layouts
 */

#define GUT_IMPLEMENTATION
#include "gut_single.h"
#include <iostream>

void printElementBounds(const char* name, gut::Element* element) {
    auto b = element->bounds();
    std::cout << name << ": x=" << b.x << ", y=" << b.y 
              << ", w=" << b.width << ", h=" << b.height << "\n";
}

int main() {
    std::cout << "Gut Layout Demo\n\n";
    
    gut::initialize();
    
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    
    // ---- StackPanel Demo ----
    std::cout << "--- StackPanel Demo ---\n";
    {
        auto vstack = gut::makeRef<gut::StackPanel>();
        vstack->setorientation(gut::Orientation::Vertical);
        vstack->setwidth(200);
        
        for (int i = 0; i < 3; ++i) {
            auto item = gut::makeRef<gut::Button>();
            item->setlabel("Item " + std::to_string(i + 1));
            item->setheight(40);
            item->setmargin({5, 5, 5, 5});
            vstack->addChild(item);
        }
        
        ctx.setRoot(vstack);
        ctx.update(0);
        ctx.render(400, 300);
        
        std::cout << "Vertical StackPanel with 3 buttons:\n";
        printElementBounds("StackPanel", vstack.get());
        for (gut::usize i = 0; i < vstack->childCount(); ++i) {
            auto name = "  Button " + std::to_string(i + 1);
            printElementBounds(name.c_str(), vstack->childAt(i));
        }
    }
    
    // ---- Grid Demo ----
    std::cout << "\n--- Grid Demo ---\n";
    {
        auto grid = gut::makeRef<gut::Grid>();
        grid->setwidth(400);
        grid->setheight(300);
        
        // Define 2 columns using ColumnDefinition
        grid->setColumnDefinitions({
            gut::ColumnDefinition{gut::GridLength::Star(1)},
            gut::ColumnDefinition{gut::GridLength::Star(2)}
        });
        
        // Define 3 rows using RowDefinition
        grid->setRowDefinitions({
            gut::RowDefinition{gut::GridLength::Auto()},
            gut::RowDefinition{gut::GridLength::Star(1)},
            gut::RowDefinition{gut::GridLength::Pixel(50)}
        });
        
        // Add elements to grid cells
        auto header = gut::makeRef<gut::Text>();
        header->settext("Header");
        header->setfontSize(20.0f);
        gut::Grid::setColumn(*header, 0);
        gut::Grid::setRow(*header, 0);
        gut::Grid::setColumnSpan(*header, 2);  // Span both columns
        grid->addChild(header);
        
        auto sidebar = gut::makeRef<gut::Button>();
        sidebar->setlabel("Sidebar");
        gut::Grid::setColumn(*sidebar, 0);
        gut::Grid::setRow(*sidebar, 1);
        grid->addChild(sidebar);
        
        auto content = gut::makeRef<gut::Text>();
        content->settext("Main Content Area");
        gut::Grid::setColumn(*content, 1);
        gut::Grid::setRow(*content, 1);
        grid->addChild(content);
        
        auto footer = gut::makeRef<gut::Text>();
        footer->settext("Footer");
        gut::Grid::setColumn(*footer, 0);
        gut::Grid::setRow(*footer, 2);
        gut::Grid::setColumnSpan(*footer, 2);
        grid->addChild(footer);
        
        ctx.setRoot(grid);
        ctx.update(0);
        ctx.render(400, 300);
        
        std::cout << "Grid with 2 columns (1:2 ratio) and 3 rows:\n";
        printElementBounds("Grid", grid.get());
        printElementBounds("  Header (0,0 colspan=2)", header.get());
        printElementBounds("  Sidebar (0,1)", sidebar.get());
        printElementBounds("  Content (1,1)", content.get());
        printElementBounds("  Footer (0,2 colspan=2)", footer.get());
    }
    
    // ---- Canvas Demo ----
    std::cout << "\n--- Canvas Demo ---\n";
    {
        auto canvas = gut::makeRef<gut::Canvas>();
        canvas->setwidth(400);
        canvas->setheight(300);
        
        // Position elements absolutely
        auto box1 = gut::makeRef<gut::Button>();
        box1->setlabel("Box 1");
        box1->setwidth(80);
        box1->setheight(60);
        gut::Canvas::setLeft(*box1, 10);
        gut::Canvas::setTop(*box1, 10);
        canvas->addChild(box1);
        
        auto box2 = gut::makeRef<gut::Button>();
        box2->setlabel("Box 2");
        box2->setwidth(100);
        box2->setheight(50);
        gut::Canvas::setLeft(*box2, 150);
        gut::Canvas::setTop(*box2, 100);
        canvas->addChild(box2);
        
        // Use right/bottom positioning
        auto box3 = gut::makeRef<gut::Button>();
        box3->setlabel("Box 3");
        box3->setwidth(80);
        box3->setheight(40);
        gut::Canvas::setRight(*box3, 10);
        gut::Canvas::setBottom(*box3, 10);
        canvas->addChild(box3);
        
        ctx.setRoot(canvas);
        ctx.update(0);
        ctx.render(400, 300);
        
        std::cout << "Canvas with absolute positioning:\n";
        printElementBounds("Canvas", canvas.get());
        printElementBounds("  Box 1 (left=10, top=10)", box1.get());
        printElementBounds("  Box 2 (left=150, top=100)", box2.get());
        printElementBounds("  Box 3 (right=10, bottom=10)", box3.get());
    }
    
    // ---- Nested Layout Demo ----
    std::cout << "\n--- Nested Layout Demo ---\n";
    {
        auto root = gut::makeRef<gut::StackPanel>();
        root->setorientation(gut::Orientation::Vertical);
        root->setwidth(300);
        
        // Title
        auto title = gut::makeRef<gut::Text>();
        title->settext("Nested Layout Example");
        title->setfontSize(18.0f);
        title->setmargin({0, 0, 0, 10});
        root->addChild(title);
        
        // Horizontal row with buttons
        auto buttonRow = gut::makeRef<gut::StackPanel>();
        buttonRow->setorientation(gut::Orientation::Horizontal);
        
        for (int i = 0; i < 4; ++i) {
            auto btn = gut::makeRef<gut::Button>();
            btn->setlabel("B" + std::to_string(i + 1));
            btn->setwidth(60);
            btn->setheight(30);
            btn->setmargin({2, 0, 2, 0});
            buttonRow->addChild(btn);
        }
        root->addChild(buttonRow);
        
        ctx.setRoot(root);
        ctx.update(0);
        ctx.render(300, 200);
        
        std::cout << "Nested StackPanels:\n";
        printElementBounds("Root VStack", root.get());
        printElementBounds("  Title", title.get());
        printElementBounds("  Button Row (HStack)", buttonRow.get());
        for (gut::usize i = 0; i < buttonRow->childCount(); ++i) {
            auto name = "    Button " + std::to_string(i + 1);
            printElementBounds(name.c_str(), buttonRow->childAt(i));
        }
    }
    
    std::cout << "\nLayout demo completed!\n";
    
    gut::shutdown();
    return 0;
}
