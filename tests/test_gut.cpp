/**
 * @file test_gut.cpp
 * @brief Unit tests for Gut library
 */

#define GUT_IMPLEMENTATION
#include "gut_single.h"
#include <iostream>
#include <cmath>
#include <cstring>

static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Testing " #name "... "; \
    try { test_##name(); std::cout << "PASS\n"; testsPassed++; } \
    catch (const std::exception& e) { std::cout << "FAIL: " << e.what() << "\n"; testsFailed++; } \
} while(0)

#define ASSERT(cond) if (!(cond)) throw std::runtime_error("Assertion failed: " #cond)
#define ASSERT_EQ(a, b) if ((a) != (b)) throw std::runtime_error("Expected equality")
#define ASSERT_NEAR(a, b, eps) if (std::abs((a) - (b)) > (eps)) throw std::runtime_error("Expected near")

// =============================================================================
// Core Tests
// =============================================================================

TEST(initialize_shutdown) {
    ASSERT(gut::initialize());
    gut::shutdown();
}

TEST(version) {
    const char* v = gut::versionString();
    ASSERT(v != nullptr);
    ASSERT(std::strlen(v) > 0);
    ASSERT(std::strchr(v, '.') != nullptr);
}

// =============================================================================
// Reference Counting Tests
// =============================================================================

TEST(ref_basic) {
    auto btn = gut::make<gut::Button>();
    ASSERT(btn != nullptr);
    ASSERT_EQ(btn->refCount(), 1);
}

TEST(ref_copy) {
    auto btn1 = gut::make<gut::Button>();
    auto btn2 = btn1;
    ASSERT_EQ(btn1->refCount(), 2);
    ASSERT_EQ(btn2->refCount(), 2);
}

TEST(ref_scope) {
    gut::Ref<gut::Button> outer;
    {
        auto inner = gut::make<gut::Button>();
        outer = inner;
        ASSERT_EQ(inner->refCount(), 2);
    }
    ASSERT_EQ(outer->refCount(), 1);
}

// =============================================================================
// Property Tests
// =============================================================================

TEST(property_get_set) {
    gut::Property<int> prop(42);
    ASSERT_EQ(prop.get(), 42);
    prop.set(100);
    ASSERT_EQ(prop.get(), 100);
}

TEST(property_change_signal) {
    gut::Property<int> prop(0);
    int oldVal = -1, newVal = -1;
    // Signal emits (newValue, oldValue)
    prop.changed().connect([&](const int& n, const int& o) {
        newVal = n;
        oldVal = o;
    });
    prop.set(42);
    ASSERT_EQ(oldVal, 0);
    ASSERT_EQ(newVal, 42);
}

TEST(property_no_change_no_signal) {
    gut::Property<int> prop(42);
    bool called = false;
    prop.changed().connect([&](const int&, const int&) { called = true; });
    prop.set(42);  // Same value
    ASSERT(!called);
}

// =============================================================================
// Signal Tests
// =============================================================================

TEST(signal_connect_emit) {
    gut::Signal<int, int> sig;
    int sum = 0;
    sig.connect([&](int a, int b) { sum = a + b; });
    sig.emit(3, 4);
    ASSERT_EQ(sum, 7);
}

TEST(signal_multiple_slots) {
    gut::Signal<> sig;
    int count = 0;
    sig.connect([&]() { count++; });
    sig.connect([&]() { count++; });
    sig.connect([&]() { count++; });
    sig.emit();
    ASSERT_EQ(count, 3);
}

// =============================================================================
// Element Tests
// =============================================================================

TEST(element_create) {
    auto elem = gut::make<gut::Element>();
    ASSERT(elem != nullptr);
}

TEST(element_opacity) {
    auto elem = gut::make<gut::Element>();
    ASSERT_NEAR(elem->opacity(), 1.0f, 0.001f);
    elem->setopacity(0.5f);
    ASSERT_NEAR(elem->opacity(), 0.5f, 0.001f);
}

TEST(element_visibility) {
    auto elem = gut::make<gut::Element>();
    ASSERT(elem->visibility() == gut::Visibility::Visible);
    elem->setvisibility(gut::Visibility::Hidden);
    ASSERT(elem->visibility() == gut::Visibility::Hidden);
}

// =============================================================================
// Panel Tests
// =============================================================================

TEST(panel_add_child) {
    auto panel = gut::make<gut::StackPanel>();
    auto child = gut::make<gut::Element>();
    panel->addChild(child);
    ASSERT_EQ(panel->childCount(), 1);
}

TEST(panel_remove_child) {
    auto panel = gut::make<gut::StackPanel>();
    auto child = gut::make<gut::Element>();
    panel->addChild(child);
    panel->removeChild(child.get());
    ASSERT_EQ(panel->childCount(), 0);
}

TEST(panel_parent_reference) {
    auto panel = gut::make<gut::StackPanel>();
    auto child = gut::make<gut::Element>();
    panel->addChild(child);
    ASSERT(child->parent() == panel.get());
}

// =============================================================================
// Text Tests
// =============================================================================

TEST(text_content) {
    auto text = gut::make<gut::Text>();
    text->settext("Hello");
    ASSERT_EQ(text->text(), "Hello");
}

TEST(text_shadow_properties) {
    auto t = gut::make<gut::Text>("Shadow");
    // Defaults
    ASSERT(t->textShadowColor().a == 0.0f);
    ASSERT(t->textShadowOffsetX() == 2.0f);
    ASSERT(t->textShadowOffsetY() == 2.0f);
    ASSERT(t->textShadowBlurRadius() == 0.0f);
    // Set
    t->settextShadowColor(gut::Color::fromRgba8(0, 0, 0, 128));
    t->settextShadowOffsetX(3.0f);
    t->settextShadowOffsetY(4.0f);
    t->settextShadowBlurRadius(5.0f);
    ASSERT(t->textShadowColor().a > 0.4f);
    ASSERT(t->textShadowOffsetX() == 3.0f);
    ASSERT(t->textShadowOffsetY() == 4.0f);
    ASSERT(t->textShadowBlurRadius() == 5.0f);
}

TEST(text_stroke_properties) {
    auto t = gut::make<gut::Text>("Stroke");
    ASSERT(t->textStrokeColor().a == 0.0f);
    ASSERT(t->textStrokeWidth() == 1.0f);
    t->settextStrokeColor(gut::Color::fromRgba8(255, 0, 0, 255));
    t->settextStrokeWidth(2.5f);
    ASSERT(t->textStrokeColor().r == 1.0f);
    ASSERT(t->textStrokeWidth() == 2.5f);
}

TEST(text_glow_properties) {
    auto t = gut::make<gut::Text>("Glow");
    ASSERT(t->textGlowColor().a == 0.0f);
    ASSERT(t->textGlowRadius() == 4.0f);
    t->settextGlowColor(gut::Color::fromRgba8(0, 200, 255, 100));
    t->settextGlowRadius(8.0f);
    ASSERT(t->textGlowColor().g > 0.7f);
    ASSERT(t->textGlowRadius() == 8.0f);
}

TEST(text_effects_combined) {
    auto t = gut::make<gut::Text>("All");
    t->settextShadowColor(gut::Color::fromRgba8(0, 0, 0, 180));
    t->settextStrokeColor(gut::Color::fromRgba8(40, 40, 60, 255));
    t->settextStrokeWidth(1.5f);
    t->settextGlowColor(gut::Color::fromRgba8(100, 180, 255, 80));
    t->settextGlowRadius(4.0f);
    // All three effects set simultaneously
    ASSERT(t->textShadowColor().a > 0);
    ASSERT(t->textStrokeColor().a > 0);
    ASSERT(t->textGlowColor().a > 0);
}

// =============================================================================
// Button Tests
// =============================================================================

TEST(button_create) {
    auto btn = gut::make<gut::Button>();
    ASSERT(btn != nullptr);
}

// =============================================================================
// Animation Tests
// =============================================================================

TEST(animation_property_changed_signal) {
    // Test that PropertyAnimation fires the property's changed signal
    auto elem = gut::make<gut::Element>();
    
    int changeCount = 0;
    gut::f32 lastValue = 0.0f;
    
    // Connect to the opacity property's changed signal
    elem->opacityProperty().changed().connect([&](const gut::f32& newVal, const gut::f32&) {
        changeCount++;
        lastValue = newVal;
    });
    
    // Create animation from 1.0 to 0.0
    auto anim = gut::makeRef<gut::FloatAnimation>();
    anim->setTargetProperty(&elem->opacityProperty());
    anim->setFrom(1.0f);
    anim->setTo(0.0f);
    anim->setduration(100.0f);  // 100ms
    
    // Manually update the animation (simulate time passing)
    anim->begin();
    anim->update(50.0f);  // 50% through
    
    // Changed signal should have fired at least once
    ASSERT(changeCount > 0);
    ASSERT_NEAR(lastValue, 0.5f, 0.01f);  // Should be ~0.5 at 50% progress
    
    // Update to completion
    anim->update(50.0f);  // Complete
    ASSERT_NEAR(lastValue, 0.0f, 0.01f);
}

// =============================================================================
// Easing Tests
// =============================================================================

TEST(easing_linear) {
    ASSERT_NEAR(gut::easing::linear(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(gut::easing::linear(0.5f), 0.5f, 0.001f);
    ASSERT_NEAR(gut::easing::linear(1.0f), 1.0f, 0.001f);
}

TEST(easing_quad) {
    ASSERT_NEAR(gut::easing::easeInQuad(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(gut::easing::easeInQuad(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(gut::easing::easeOutQuad(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(gut::easing::easeOutQuad(1.0f), 1.0f, 0.001f);
}

TEST(easing_bounds) {
    ASSERT_NEAR(gut::easing::easeInCubic(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(gut::easing::easeInCubic(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(gut::easing::easeOutExpo(0.0f), 0.0f, 0.01f);
    ASSERT_NEAR(gut::easing::easeOutExpo(1.0f), 1.0f, 0.001f);
}

// =============================================================================
// Font Tests
// =============================================================================

TEST(font_create) {
    auto font = gut::makeRef<gut::Font>();
    ASSERT(font != nullptr);
    ASSERT(!font->isLoaded()); // No data loaded yet
}

TEST(font_face_placeholder) {
    // Create a font without loading data - should give placeholder glyphs
    auto font = gut::makeRef<gut::Font>();
    auto face = font->getFace(16.0f);
    ASSERT(face != nullptr);
    ASSERT_NEAR(face->size(), 16.0f, 0.001f);
    ASSERT(face->lineHeight() > 0);
    ASSERT(face->ascender() > 0);
    
    // Should have placeholder glyph for ASCII
    const gut::Glyph* g = face->glyph('A');
    ASSERT(g != nullptr);
    ASSERT(g->advance > 0);
}

TEST(font_face_measure) {
    auto font = gut::makeRef<gut::Font>();
    auto face = font->getFace(16.0f);
    
    // Measure text
    gut::f32 width = face->measureWidth("Hello");
    ASSERT(width > 0);
    
    // Longer text should be wider
    gut::f32 longerWidth = face->measureWidth("Hello, World!");
    ASSERT(longerWidth > width);
    
    // Empty text should be zero width
    ASSERT_NEAR(face->measureWidth(""), 0.0f, 0.001f);
}

TEST(font_face_cache) {
    auto font = gut::makeRef<gut::Font>();
    
    // Get same size twice - should return same cached face
    auto face1 = font->getFace(16.0f);
    auto face2 = font->getFace(16.0f);
    ASSERT(face1.get() == face2.get());
    
    // Different size should return different face
    auto face3 = font->getFace(24.0f);
    ASSERT(face1.get() != face3.get());
}

// =============================================================================
// Geometry Tests
// =============================================================================

TEST(rect_contains) {
    gut::Rectf rect{10, 10, 100, 50};
    ASSERT(rect.contains({50, 30}));
    ASSERT(!rect.contains({5, 30}));
    ASSERT(!rect.contains({200, 30}));
}

TEST(rect_intersection) {
    gut::Rectf a{0, 0, 100, 100};
    gut::Rectf b{50, 50, 100, 100};
    auto c = a.intersection(b);
    ASSERT_NEAR(c.x, 50, 0.001f);
    ASSERT_NEAR(c.y, 50, 0.001f);
    ASSERT_NEAR(c.width, 50, 0.001f);
    ASSERT_NEAR(c.height, 50, 0.001f);
}

// =============================================================================
// Context Tests
// =============================================================================

TEST(context_create) {
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
}

TEST(context_set_root) {
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    auto root = gut::make<gut::StackPanel>();
    ctx.setRoot(root);
    ASSERT(ctx.root() == root.get());
}

TEST(context_update_render) {
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    auto root = gut::make<gut::StackPanel>();
    ctx.setRoot(root);
    ctx.update(0.016f);
    ctx.render(800, 600);
}

// =============================================================================
// Event Bubbling Tests
// =============================================================================

/// Helper element that records mouse events it receives.
/// Extends Panel so it can have children (needed for tree building).
/// Acts like a simple Canvas — all children are given the full available size.
class EventSpy : public gut::Panel {
public:
    int mouseDownCount{0};
    int mouseUpCount{0};
    int mouseMoveCount{0};
    int mouseWheelCount{0};
    int keyDownCount{0};
    bool shouldHandle{false};   // When true, returns true to stop bubbling
    gut::Element* lastSource{nullptr};

protected:
    gut::Size2f measureOverride(gut::Size2f availableSize) override {
        // Measure all children with full available space
        for (gut::usize i = 0; i < childCount(); ++i) {
            childAt(i)->measure(availableSize);
        }
        return availableSize;
    }

    gut::Size2f arrangeOverride(gut::Size2f finalSize) override {
        // Arrange all children at (0,0) with the full size
        for (gut::usize i = 0; i < childCount(); ++i) {
            childAt(i)->arrange({0, 0, finalSize.width, finalSize.height});
        }
        return finalSize;
    }

    bool onMouseEvent(const gut::MouseEvent& event) override {
        lastSource = event.source;
        switch (event.type) {
            case gut::MouseEventType::ButtonDown: mouseDownCount++; break;
            case gut::MouseEventType::ButtonUp:   mouseUpCount++;   break;
            case gut::MouseEventType::Move:       mouseMoveCount++; break;
            case gut::MouseEventType::Wheel:      mouseWheelCount++; break;
            default: break;
        }
        return shouldHandle;
    }

    bool onKeyEvent(const gut::KeyEvent& event) override {
        if (event.type == gut::KeyEventType::KeyDown) keyDownCount++;
        return shouldHandle;
    }
};

/// Helper: set up a Context with a laid-out tree and return the InputManager.
/// Tree: root (400x400) → child (400x400) → leaf (400x400)
/// All positioned at (0,0), stacked on top of each other.
static void setupBubbleTree(
    gut::Context& ctx,
    gut::Ref<EventSpy>& root,
    gut::Ref<EventSpy>& child,
    gut::Ref<EventSpy>& leaf)
{
    // Build tree: root panel has child panel, which has leaf element
    auto rootPanel = gut::make<gut::StackPanel>();
    rootPanel->addChild(root);
    root->addChild(child);
    child->addChild(leaf);

    // Give all elements explicit sizes so hit testing works
    root->setwidth(400);  root->setheight(400);
    child->setwidth(400); child->setheight(400);
    leaf->setwidth(400);  leaf->setheight(400);

    ctx.setRoot(rootPanel);
    rootPanel->setwidth(400);
    rootPanel->setheight(400);

    // Force layout so bounds are valid
    ctx.render(400, 400);
}

TEST(bubble_mouse_to_parent) {
    // When the leaf doesn't handle the event, it should bubble up to parent and grandparent
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));

    auto root  = gut::make<EventSpy>();
    auto child = gut::make<EventSpy>();
    auto leaf  = gut::make<EventSpy>();

    setupBubbleTree(ctx, root, child, leaf);

    // Click inside the tree — hit testing should find the leaf
    ctx.inputManager().processMouseMove(10, 10);
    ctx.inputManager().processMouseButton(gut::MouseButton::Left, true);

    // All three should see the ButtonDown event (nobody handled it)
    ASSERT(leaf->mouseDownCount  == 1);
    ASSERT(child->mouseDownCount == 1);
    ASSERT(root->mouseDownCount  == 1);
}

TEST(bubble_stops_when_handled) {
    // When the child handles the event, the root should NOT see it
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));

    auto root  = gut::make<EventSpy>();
    auto child = gut::make<EventSpy>();
    auto leaf  = gut::make<EventSpy>();

    child->shouldHandle = true;  // child swallows the event

    setupBubbleTree(ctx, root, child, leaf);

    ctx.inputManager().processMouseMove(10, 10);
    ctx.inputManager().processMouseButton(gut::MouseButton::Left, true);

    // Leaf sees it (first), child sees and handles it, root does NOT
    ASSERT(leaf->mouseDownCount  == 1);
    ASSERT(child->mouseDownCount == 1);
    ASSERT(root->mouseDownCount  == 0);
}

TEST(bubble_source_is_original_target) {
    // event.source should always point to the original hit-test target (leaf)
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));

    auto root  = gut::make<EventSpy>();
    auto child = gut::make<EventSpy>();
    auto leaf  = gut::make<EventSpy>();

    setupBubbleTree(ctx, root, child, leaf);

    ctx.inputManager().processMouseMove(10, 10);
    ctx.inputManager().processMouseButton(gut::MouseButton::Left, true);

    // All handlers should see the same source: the leaf
    ASSERT(leaf->lastSource  == leaf.get());
    ASSERT(child->lastSource == leaf.get());
    ASSERT(root->lastSource  == leaf.get());
}

TEST(bubble_wheel_to_parent) {
    // Wheel events should also bubble
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));

    auto root  = gut::make<EventSpy>();
    auto child = gut::make<EventSpy>();
    auto leaf  = gut::make<EventSpy>();

    setupBubbleTree(ctx, root, child, leaf);

    ctx.inputManager().processMouseMove(10, 10);
    ctx.inputManager().processMouseWheel(0, 1.0f);

    ASSERT(leaf->mouseWheelCount  == 1);
    ASSERT(child->mouseWheelCount == 1);
    ASSERT(root->mouseWheelCount  == 1);
}

TEST(bubble_key_to_parent) {
    // Key events bubble from the focused element up through parents
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));

    auto root  = gut::make<EventSpy>();
    auto child = gut::make<EventSpy>();
    auto leaf  = gut::make<EventSpy>();

    leaf->setfocusable(true);

    setupBubbleTree(ctx, root, child, leaf);

    // Focus the leaf
    ctx.focusManager().setFocus(leaf.get());

    ctx.inputManager().processKey(gut::Key::A, true);

    ASSERT(leaf->keyDownCount  == 1);
    ASSERT(child->keyDownCount == 1);
    ASSERT(root->keyDownCount  == 1);
}

TEST(button_click_stops_bubbling) {
    // A Button should handle ButtonDown/Up, preventing bubbling past it
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));

    auto rootSpy = gut::make<EventSpy>();
    auto button  = gut::make<gut::Button>("Click Me");
    rootSpy->addChild(button);
    rootSpy->setwidth(400);
    rootSpy->setheight(400);
    button->setwidth(200);
    button->setheight(40);

    auto rootPanel = gut::make<gut::StackPanel>();
    rootPanel->addChild(rootSpy);
    rootPanel->setwidth(400);
    rootPanel->setheight(400);
    ctx.setRoot(rootPanel);
    ctx.render(400, 400);

    bool clicked = false;
    button->setOnClick([&]{ clicked = true; });

    ctx.inputManager().processMouseMove(10, 10);
    ctx.inputManager().processMouseButton(gut::MouseButton::Left, true);
    ctx.inputManager().processMouseButton(gut::MouseButton::Left, false);

    // Button should have handled the click — parent spy should NOT see it
    ASSERT(clicked);
    ASSERT(rootSpy->mouseDownCount == 0);
    ASSERT(rootSpy->mouseUpCount   == 0);
}

// =============================================================================
// ListView
// =============================================================================

TEST(listview_create) {
    auto lv = gut::make<gut::ListView>();
    ASSERT(lv != nullptr);
    ASSERT(lv->itemCount() == 0);
    ASSERT(lv->selectionMode() == gut::SelectionMode::Single);
    ASSERT(lv->selectedIndex() == -1);
    ASSERT(lv->selectedIndices().empty());
}

TEST(listview_item_count) {
    auto lv = gut::make<gut::ListView>();
    lv->setItemCount(500);
    ASSERT(lv->itemCount() == 500);
    // Changing item count clears selection
    lv->selectIndex(0);
    lv->setItemCount(100);
    ASSERT(lv->itemCount() == 100);
    ASSERT(lv->selectedIndices().empty());
}

TEST(listview_single_selection) {
    auto lv = gut::make<gut::ListView>();
    lv->setselectionMode(gut::SelectionMode::Single);
    lv->setItemCount(10);

    lv->selectIndex(3);
    ASSERT(lv->selectedIndex() == 3);
    ASSERT(lv->selectedIndices().size() == 1);

    // Selecting another index in Single mode replaces the previous
    lv->selectIndex(7);
    ASSERT(lv->selectedIndex() == 7);
    ASSERT(lv->selectedIndices().size() == 1);

    // Out-of-range is ignored
    lv->selectIndex(999);
    ASSERT(lv->selectedIndex() == 7);
}

TEST(listview_extended_selection) {
    auto lv = gut::make<gut::ListView>();
    lv->setselectionMode(gut::SelectionMode::Extended);
    lv->setItemCount(20);

    lv->selectIndex(2);
    lv->selectRange(5, 8);
    // Should have indices 5..8 selected (selectRange replaces)
    ASSERT(lv->selectedIndices().count(5) == 1);
    ASSERT(lv->selectedIndices().count(6) == 1);
    ASSERT(lv->selectedIndices().count(7) == 1);
    ASSERT(lv->selectedIndices().count(8) == 1);
}

TEST(listview_deselect_all) {
    auto lv = gut::make<gut::ListView>();
    lv->setItemCount(10);
    lv->selectIndex(3);
    ASSERT(!lv->selectedIndices().empty());
    lv->deselectAll();
    ASSERT(lv->selectedIndices().empty());
    ASSERT(lv->selectedIndex() == -1);
}

TEST(listview_selection_mode_none) {
    auto lv = gut::make<gut::ListView>();
    lv->setselectionMode(gut::SelectionMode::None);
    lv->setItemCount(10);
    lv->selectIndex(3);
    // In None mode, selection should remain empty
    ASSERT(lv->selectedIndices().empty());
    ASSERT(lv->selectedIndex() == -1);
}

TEST(listview_selection_callback) {
    auto lv = gut::make<gut::ListView>();
    lv->setItemCount(10);

    int callCount = 0;
    gut::isize lastSelectedIdx = -1;
    lv->setOnSelectionChanged([&](const std::set<gut::isize>& sel) {
        callCount++;
        if (!sel.empty()) lastSelectedIdx = *sel.begin();
    });

    lv->selectIndex(4);
    ASSERT(callCount == 1);
    ASSERT(lastSelectedIdx == 4);

    lv->deselectAll();
    ASSERT(callCount == 2);
}

TEST(listview_properties) {
    auto lv = gut::make<gut::ListView>();
    lv->setitemHeight(40.0f);
    ASSERT(lv->itemHeight() == 40.0f);

    lv->setfontSize(16.0f);
    ASSERT(lv->fontSize() == 16.0f);
}

// =============================================================================
// Toast
// =============================================================================

TEST(toast_style_defaults) {
    auto& sty = gut::Toast::style();
    ASSERT(sty.defaultDuration == 3.0f);
    ASSERT(sty.fontSize == 12.0f);
    ASSERT(sty.cornerRadius == 6.0f);
    ASSERT(sty.maxWidth == 320.0f);
}

TEST(toast_show_no_crash) {
    // Showing a toast with a valid context should not crash
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    ctx.update(0.0f); // initialise totalTime

    gut::Toast::show(&ctx, "Hello Toast", gut::ToastPosition::BottomRight);
    // Render a frame so the overlay runs
    ctx.render(800, 600);
    // No crash = pass

    gut::Toast::dismissAll(&ctx);
}

TEST(toast_dismiss_all) {
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    ctx.update(0.0f);

    gut::Toast::show(&ctx, "Toast 1", gut::ToastPosition::TopLeft);
    gut::Toast::show(&ctx, "Toast 2", gut::ToastPosition::TopRight);
    gut::Toast::show(&ctx, "Toast 3", gut::ToastPosition::BottomCenter);

    // Should not crash
    ctx.render(800, 600);
    gut::Toast::dismissAll(&ctx);
    ctx.render(800, 600);
}

TEST(toast_positions) {
    auto backend = std::make_unique<gut::NullRenderBackend>();
    gut::Context ctx(std::move(backend));
    ctx.update(0.0f);

    // Fire one toast at every position — just verify no crash
    gut::Toast::show(&ctx, "TL", gut::ToastPosition::TopLeft);
    gut::Toast::show(&ctx, "TC", gut::ToastPosition::TopCenter);
    gut::Toast::show(&ctx, "TR", gut::ToastPosition::TopRight);
    gut::Toast::show(&ctx, "BL", gut::ToastPosition::BottomLeft);
    gut::Toast::show(&ctx, "BC", gut::ToastPosition::BottomCenter);
    gut::Toast::show(&ctx, "BR", gut::ToastPosition::BottomRight);

    ctx.render(800, 600);
    gut::Toast::dismissAll(&ctx);
}

TEST(toast_null_context_safe) {
    // Passing nullptr should be a no-op, no crash
    gut::Toast::show(nullptr, "nope");
    gut::Toast::dismissAll(nullptr);
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "Gut v" << gut::versionString() << " - Unit Tests\n";
    std::cout << "==========================================\n\n";
    
    // Core
    RUN_TEST(initialize_shutdown);
    RUN_TEST(version);
    
    // Reference Counting
    RUN_TEST(ref_basic);
    RUN_TEST(ref_copy);
    RUN_TEST(ref_scope);
    
    // Properties
    RUN_TEST(property_get_set);
    RUN_TEST(property_change_signal);
    RUN_TEST(property_no_change_no_signal);
    
    // Signals
    RUN_TEST(signal_connect_emit);
    RUN_TEST(signal_multiple_slots);
    
    // Elements
    RUN_TEST(element_create);
    RUN_TEST(element_opacity);
    RUN_TEST(element_visibility);
    
    // Panels
    RUN_TEST(panel_add_child);
    RUN_TEST(panel_remove_child);
    RUN_TEST(panel_parent_reference);
    
    // Text
    RUN_TEST(text_content);
    RUN_TEST(text_shadow_properties);
    RUN_TEST(text_stroke_properties);
    RUN_TEST(text_glow_properties);
    RUN_TEST(text_effects_combined);
    
    // Button
    RUN_TEST(button_create);
    
    // Animation
    RUN_TEST(animation_property_changed_signal);
    
    // Easing
    RUN_TEST(easing_linear);
    RUN_TEST(easing_quad);
    RUN_TEST(easing_bounds);
    
    // Font
    RUN_TEST(font_create);
    RUN_TEST(font_face_placeholder);
    RUN_TEST(font_face_measure);
    RUN_TEST(font_face_cache);
    
    // Geometry
    RUN_TEST(rect_contains);
    RUN_TEST(rect_intersection);
    
    // Context
    RUN_TEST(context_create);
    RUN_TEST(context_set_root);
    RUN_TEST(context_update_render);
    
    // Event Bubbling
    RUN_TEST(bubble_mouse_to_parent);
    RUN_TEST(bubble_stops_when_handled);
    RUN_TEST(bubble_source_is_original_target);
    RUN_TEST(bubble_wheel_to_parent);
    RUN_TEST(bubble_key_to_parent);
    RUN_TEST(button_click_stops_bubbling);
    
    // ListView
    RUN_TEST(listview_create);
    RUN_TEST(listview_item_count);
    RUN_TEST(listview_single_selection);
    RUN_TEST(listview_extended_selection);
    RUN_TEST(listview_deselect_all);
    RUN_TEST(listview_selection_mode_none);
    RUN_TEST(listview_selection_callback);
    RUN_TEST(listview_properties);
    
    // Toast
    RUN_TEST(toast_style_defaults);
    RUN_TEST(toast_show_no_crash);
    RUN_TEST(toast_dismiss_all);
    RUN_TEST(toast_positions);
    RUN_TEST(toast_null_context_safe);
    
    std::cout << "\n==========================================\n";
    std::cout << "Results: " << testsPassed << " passed, " << testsFailed << " failed\n";
    
    return testsFailed > 0 ? 1 : 0;
}
