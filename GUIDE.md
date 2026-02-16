# GUT (Game UI Toolkit) - Agent Guide

A single-header C++20 retained-mode UI library for games. This guide is for AI coding agents building game UIs with GUT.

## Setup

```cpp
// In ONE .cpp file:
#define GUT_IMPLEMENTATION
#include "gut_single.h"

// Everywhere else:
#include "gut_single.h"
```

All types live in `namespace gut`. This guide assumes `using namespace gut;`.

## Core Loop

```cpp
// Create context with your render backend
auto backend = std::make_unique<MyD3D11Backend>();
Context ctx(std::move(backend));

// Load a font
auto fontData = loadFile("Roboto-Regular.ttf");
auto font = ctx.loadFont(fontData.data(), fontData.size());
ctx.setDefaultFont(font);

// Build UI tree
auto root = makeRef<StackPanel>(Orientation::Vertical);
root->addChild(makeRef<Text>("Hello, GUT!"));
ctx.setRoot(root);

// Game loop
while (running) {
    ctx.processMouseMove(mouseX, mouseY);
    ctx.processMouseButton(MouseButton::Left, pressed);
    ctx.processMouseWheel(deltaX, deltaY);
    ctx.processKey(key, pressed, modifiers);
    ctx.processTextInput(character);

    ctx.update(deltaSec);               // seconds, not ms
    ctx.render(windowWidth, windowHeight, dpiScale);
}
```

## Property System

GUT_PROPERTY generates a getter, setter, and property accessor. **Setters are lowercase**:

```cpp
// GUT_PROPERTY(f32, width, NAN) generates:
element->width();                   // getter - returns f32
element->setwidth(200.0f);          // setter - lowercase!
element->widthProperty();           // Property<f32>& for animation/binding
```

This is a common gotcha: the setter is `setwidth`, not `setWidth`.

Property<T> has `.get()`, `.set()`, and `.changed()` signal:

```cpp
auto conn = element->opacityProperty().changed().connect(
    [](const f32& newVal, const f32& oldVal) { /* react */ }
);
```

## Creating Elements

Use `makeRef<T>(args...)` to create reference-counted elements:

```cpp
auto panel = makeRef<Panel>();
auto stack = makeRef<StackPanel>(Orientation::Horizontal);
auto text  = makeRef<Text>("Score: 100", 14.0f);
auto btn   = makeRef<Button>("Attack");
auto img   = makeRef<Image>(texture);
```

## Color

```cpp
Color::fromRgba8(255, 100, 50);         // u8 components
Color::fromRgba8(255, 100, 50, 128);    // with alpha
Color::fromHex(0xFF6432);               // hex RGB
Color::white();                          // predefined colors
Color::transparent();
Color(1.0f, 0.4f, 0.2f, 1.0f);         // f32 components
```

## Layout Elements

### StackPanel - Linear stacking

```cpp
auto vstack = makeRef<StackPanel>(Orientation::Vertical);
vstack->setspacing(8.0f);
vstack->addChild(child1);
vstack->addChild(child2);

// Also accepts initializer list:
auto hstack = makeRef<StackPanel>(Orientation::Horizontal, {child1, child2, child3});
```

### Canvas - Absolute positioning

```cpp
auto canvas = makeRef<Canvas>();
auto dot = makeRef<Panel>();
canvas->addChild(dot);
Canvas::setLeft(*dot, 100.0f);
Canvas::setTop(*dot, 50.0f);
```

### Grid - Rows and columns

```cpp
auto grid = makeRef<Grid>(2, 3); // 2 rows, 3 columns
grid->setRowDefinitions({GridLength::Pixel(40), GridLength::Star(1)});
grid->setColumnDefinitions({
    GridLength::Pixel(100),
    GridLength::Star(1),
    GridLength::Auto()
});

auto child = makeRef<Text>("Cell");
Grid::setRow(*child, 0);
Grid::setColumn(*child, 1);
Grid::setColumnSpan(*child, 2);
grid->addChild(child);
```

### DockPanel - Edge docking

```cpp
auto dock = makeRef<DockPanel>();

auto toolbar = makeRef<Panel>();
DockPanel::setDock(*toolbar, Dock::Top);
dock->addChild(toolbar);

auto sidebar = makeRef<Panel>();
DockPanel::setDock(*sidebar, Dock::Left);
dock->addChild(sidebar);

// Last child fills remaining space (Dock::Fill is default)
auto content = makeRef<Panel>();
dock->addChild(content);
```

### WrapPanel - Flow layout

```cpp
auto wrap = makeRef<WrapPanel>(Orientation::Horizontal);
wrap->setitemSpacing(8.0f);
wrap->setlineSpacing(8.0f);
```

### ScrollViewer - Scrollable container

```cpp
auto scroll = makeRef<ScrollViewer>();
scroll->sethorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
scroll->addChild(tallContent);
```

### ViewBox - Scale to fit

```cpp
auto viewbox = makeRef<ViewBox>();
viewbox->setwidth(400); viewbox->setheight(300);
// Child is scaled uniformly to fit within viewbox
viewbox->addChild(largeContent);
```

## Panel Styling

Panel is the base for all containers. It supports backgrounds, borders, shadows, and blur:

```cpp
auto card = makeRef<Panel>();
card->setwidth(300);
card->setheight(200);
card->setbackground(Color::fromRgba8(32, 35, 48));
card->setcornerRadius(8.0f);
card->setborderColor(Color::fromRgba8(50, 55, 75));
card->setborderWidth(1.0f);

// Gradient background
card->setbackgroundGradientTop(Color::fromRgba8(60, 60, 80));
card->setbackgroundGradientBottom(Color::fromRgba8(20, 20, 30));

// Drop shadow
card->setshadowColor(Color::fromRgba8(0, 0, 0, 200));
card->setshadowOffsetX(4.0f);
card->setshadowOffsetY(6.0f);
card->setshadowBlurRadius(12.0f);

// Inset shadow
card->setinsetShadowColor(Color::fromRgba8(0, 0, 0, 100));
card->setinsetShadowBlurRadius(8.0f);

// Backdrop blur (frosted glass)
card->setbackdropBlur(10.0f);
card->setbackdropTint(Color::fromRgba8(255, 255, 255, 30));

// Hover/press colors
card->sethoverBackground(Color::fromRgba8(45, 48, 60));
card->setpressedBackground(Color::fromRgba8(25, 28, 38));
card->setOnClick([]() { /* handle click */ });
```

## Common Element Properties

All elements share these properties:

```cpp
element->setwidth(200.0f);          // NAN = auto-size
element->setheight(100.0f);
element->setminWidth(50.0f);
element->setmaxWidth(400.0f);
element->setmargin(Thickness{8, 4, 8, 4});      // left, top, right, bottom
element->setpadding(Thickness{12, 8, 12, 8});
element->sethorizontalAlignment(HorizontalAlignment::Center);  // Left/Center/Right/Stretch
element->setverticalAlignment(VerticalAlignment::Top);          // Top/Center/Bottom/Stretch
element->setvisibility(Visibility::Collapsed);   // Visible/Hidden/Collapsed
element->setopacity(0.8f);
element->setzIndex(10);
element->setclipToBounds(true);
element->setisHitTestVisible(false);
element->setisEnabled(false);

// Transforms (around element center)
element->setscaleX(1.5f);
element->setscaleY(1.5f);
element->setrotation(0.5f);         // radians
element->setskewX(0.1f);
element->settranslateX(10.0f);      // pixel offset, useful for animation
element->settranslateY(-5.0f);
```

## Text

```cpp
auto text = makeRef<Text>("Hello World", 16.0f);
text->setforeground(Color::fromRgba8(255, 210, 100));
text->setfontWeight(FontWeight::Bold);
text->setfontStyle(FontStyle::Italic);
text->settextWrapping(Text::TextWrapping::Wrap);
text->settextTrimming(Text::TextTrimming::CharacterEllipsis);
text->settextAlignment(Text::TextAlignment::Center);
text->setlineHeight(1.4f);

// Text effects
text->settextShadowColor(Color::fromRgba8(0, 0, 0, 180));
text->settextShadowOffsetX(2.0f);
text->settextShadowOffsetY(2.0f);

text->settextStrokeColor(Color::fromRgba8(0, 0, 0));
text->settextStrokeWidth(2.0f);

text->settextGlowColor(Color::fromRgba8(255, 200, 50, 150));
text->settextGlowRadius(6.0f);
```

## Image

```cpp
auto texture = backend->createTexture(w, h, rgbaPixels);
// or: auto texture = backend->createTextureFromFile("icon.png");

auto img = makeRef<Image>(texture);
img->setstretch(Image::Stretch::Uniform);   // None/Fill/Uniform/UniformToFill/NineSlice
img->settint(Color::fromRgba8(255, 200, 200));

// Sprite sheet (sub-rect of texture)
img->setSourceRect(Rectf{0, 0, 32, 32});

// 9-slice for scalable borders
img->setstretch(Image::Stretch::NineSlice);
img->setsliceBorders(Thickness{12, 12, 12, 12});
```

## Controls

### Button

```cpp
auto btn = makeRef<Button>("Cast Fireball");
btn->setOnClick([]() {
    // handle click
});

// Styling
btn->setbackground(Color::fromRgba8(60, 130, 220));
btn->setforeground(Color::white());
btn->sethoverBackground(Color::fromRgba8(80, 150, 240));
btn->setpressedBackground(Color::fromRgba8(40, 110, 200));
btn->setcornerRadius(6.0f);

// Custom content instead of label
auto icon = makeRef<Image>(iconTexture);
btn->setContent(icon);
```

### CheckBox

```cpp
auto cb = makeRef<CheckBox>("Enable VSync");
cb->setisChecked(true);
cb->setOnCheckedChanged([](bool checked) {
    // react to toggle
});
```

### RadioButton

```cpp
auto r1 = makeRef<RadioButton>("Easy", "difficulty");
auto r2 = makeRef<RadioButton>("Hard", "difficulty");
r1->setisChecked(true);
// Selecting one auto-deselects siblings in same group
```

### Toggle / Switch

```cpp
auto toggle = makeRef<Toggle>("Fullscreen");
toggle->setisOn(false);
toggle->setOnToggled([](bool on) { /* ... */ });
```

### Slider

```cpp
auto slider = makeRef<Slider>(0.75f);
slider->setminimum(0.0f);
slider->setmaximum(1.0f);
slider->setstep(0.05f);          // 0 = continuous
slider->setshowValue(true);
slider->setOnValueChanged([](f32 val) { /* ... */ });
```

### ProgressBar

```cpp
auto bar = makeRef<ProgressBar>(65.0f);  // 0-100
bar->setshowLabel(true);
// or indeterminate:
bar->setisIndeterminate(true);
```

### TextBox

```cpp
auto input = makeRef<TextBox>("initial text");
input->setplaceholder("Enter name...");
input->setmaxLength(32);
input->setisPassword(true);
input->setOnTextChanged([](const String& text) { /* ... */ });
input->setOnReturnPressed([]() { /* submit */ });
```

### DropDown

```cpp
auto dd = makeRef<DropDown>();
dd->addItem("Low", 0);
dd->addItem("Medium", 1);
dd->addItem("High", 2);
dd->setSelectedIndex(1);
dd->setOnSelectionChanged([](int value) { /* ... */ });
```

### TabControl

```cpp
auto tabs = makeRef<TabControl>();
tabs->addTab("Inventory", inventoryPanel);
tabs->addTab("Skills", skillsPanel);
tabs->addTab("Map", mapPanel);
tabs->setSelectedTabIndex(0);
```

### ListView

```cpp
auto list = makeRef<ListView>();
std::vector<Ref<Element>> items;
for (auto& name : playerNames) {
    auto row = makeRef<Text>(name, 13.0f);
    row->setforeground(Color::white());
    items.push_back(row);
}
list->setItems(std::move(items));
list->setitemHeight(28.0f);
list->setOnItemSelected([](int index) { /* ... */ });
```

## Popups & Overlays

### Tooltip

```cpp
Tooltip::set(element, "Deals 150 fire damage");
Tooltip::set(element, "Slow tooltip", 1000.0f);  // custom delay ms
```

### ContextMenu

```cpp
auto menu = makeRef<ContextMenu>();
menu->addItem("Use", [&]() { useItem(); });
menu->addItem("Drop", [&]() { dropItem(); });
menu->addSeparator();
menu->addItem("Cancel", []() {}, false);  // disabled
menu->attachTo(element);  // shows on right-click
```

### Dialog

```cpp
auto dialog = makeRef<Dialog>();
dialog->settitle("Confirm");
dialog->setmessage("Delete this character?");
dialog->setbuttons(DialogButtons::YesNo);
dialog->setOnResult([](DialogResult r) {
    if (r == DialogResult::Yes) { /* delete */ }
});
ctx.setRoot(root);  // dialog must be child of root
root->addChild(dialog);
dialog->show();
```

### Toast

```cpp
Toast::show(&ctx, "Achievement Unlocked!", ToastPosition::TopCenter, 3.0f);
Toast::show(&ctx, "Item acquired");  // defaults: BottomRight, 3 sec
```

## Toolbar & MenuBar

```cpp
auto toolbar = makeRef<Toolbar>();
toolbar->addButton("Save", [&]() { save(); });
toolbar->addSeparator();
toolbar->addButton("Undo", [&]() { undo(); });

auto menubar = makeRef<MenuBar>();
menubar->addMenu("File");
menubar->addItemToMenu(0, "New", [&]() { newFile(); });
menubar->addItemToMenu(0, "Open", [&]() { openFile(); });
menubar->addMenu("Edit");
menubar->addItemToMenu(1, "Undo", [&]() { undo(); });
```

## Animation

### PropertyAnimation - Animate any property

```cpp
auto anim = makeRef<FloatAnimation>();              // PropertyAnimation<f32>
anim->setTargetProperty(&element->opacityProperty());
anim->setFrom(0.0f);
anim->setTo(1.0f);
anim->setduration(500.0f);                          // milliseconds
anim->setEasingFunction(easing::easeOutCubic);
anim->begin();                                       // adds to global timeline

// Auto-reverse and loop
anim->setautoReverse(true);
anim->setRepeatBehavior(RepeatBehavior::forever());

// Color animation
auto colorAnim = makeRef<ColorAnimation>();
colorAnim->setTargetProperty(&panel->backgroundProperty());
colorAnim->setFrom(Color::red());
colorAnim->setTo(Color::blue());
colorAnim->setduration(1000.0f);
colorAnim->begin();

// Completion callback
anim->completed.connect([]() { /* animation done */ });
```

**Important**: Keep a `Ref<>` to your animation alive (store it in a member, vector, or lambda capture). If the Ref goes out of scope, the animation object is destroyed.

### Easing Functions

```cpp
// Available in gut::easing namespace:
easing::linear
easing::easeInQuad,    easing::easeOutQuad,    easing::easeInOutQuad
easing::easeInCubic,   easing::easeOutCubic,   easing::easeInOutCubic
easing::easeInQuart,   easing::easeOutQuart,   easing::easeInOutQuart
easing::easeInQuint,   easing::easeOutQuint,   easing::easeInOutQuint
easing::easeInSine,    easing::easeOutSine,    easing::easeInOutSine
easing::easeInExpo,    easing::easeOutExpo,    easing::easeInOutExpo
easing::easeInCirc,    easing::easeOutCirc,    easing::easeInOutCirc
easing::easeInBack,    easing::easeOutBack,    easing::easeInOutBack
easing::easeInElastic, easing::easeOutElastic, easing::easeInOutElastic
easing::easeInBounce,  easing::easeOutBounce,  easing::easeInOutBounce

// Custom cubic bezier:
auto ease = CubicBezierEasing(0.25f, 0.1f, 0.25f, 1.0f);
anim->setEasingFunction(ease);
```

### SpringAnimation - Physics-based

```cpp
auto spring = makeRef<FloatSpringAnimation>();
spring->setTargetProperty(&dot->translateXProperty());
spring->setStiffness(170.0f);   // spring constant
spring->setDamping(26.0f);      // friction
spring->setMass(1.0f);
spring->setTargetValue(200.0f);
spring->begin();

// Retarget mid-flight (velocity preserved, no restart)
spring->retarget(350.0f);
```

### PathAnimation - Follow a curve

```cpp
auto pathAnim = makeRef<PathAnimation>();
pathAnim->setStartPoint({0, 50});
pathAnim->addLineTo({100, 50});
pathAnim->addCubicTo({200, 50}, {130, 0}, {170, 100});  // end, control1, control2
pathAnim->setTargetXProperty(&element->translateXProperty());
pathAnim->setTargetYProperty(&element->translateYProperty());
pathAnim->setduration(2000.0f);
pathAnim->setRepeatBehavior(RepeatBehavior::forever());
pathAnim->begin();
```

### Implicit Transitions - Auto-animate property changes

```cpp
// Any future change to opacity will animate over 300ms
element->addTransition(element->opacityProperty(), 300.0f, easing::easeInOutCubic);

// Now this triggers a smooth animation instead of a jump:
element->setopacity(0.5f);

// Works with any property type that supports lerp:
panel->addTransition(panel->backgroundProperty(), 500.0f, easing::easeOutCubic);
panel->setbackground(Color::red());  // smoothly transitions
```

### Layout Animation - Animate add/remove/reorder

```cpp
auto list = makeRef<StackPanel>(Orientation::Vertical);
list->setspacing(4.0f);

Panel::LayoutTransitionConfig config;
config.moveDurationMs = 300.0f;         // existing items slide to new positions
config.moveEasing = easing::easeInOutCubic;
config.entryDurationMs = 250.0f;        // new items fade+scale in
config.entryEasing = easing::easeOutCubic;
config.exitDurationMs = 200.0f;         // removed items fade+scale out
config.exitEasing = easing::easeInCubic;
list->setLayoutTransition(config);

// Now addChild/removeChild automatically animates
list->addChild(newItem);      // fades+scales in
list->removeChild(oldItem);   // fades out, siblings slide up
```

### Storyboard - Group animations

```cpp
auto storyboard = makeRef<Storyboard>();
storyboard->addAnimation(fadeAnim);
storyboard->addAnimation(slideAnim);
storyboard->addAnimation(colorAnim);
storyboard->begin();  // all play together
storyboard->completed.connect([]() { /* all done */ });
```

## Drag and Drop

```cpp
// Start a drag (typically from a mouse down handler)
ctx.dragDropManager().beginDrag(
    sourceElement,
    DragData{"item", itemId},
    [](RenderContext& ctx, Point2f pos) {
        // Draw drag preview
        ctx.fillRoundedRect({pos.x - 20, pos.y - 20, 40, 40}, 6, Color::white());
    }
);

// Make an element accept drops
target->setisDropTarget(true);
target->dragOver().connect([](DragDropEvent& e) {
    if (e.data.format == "item") {
        e.accepted = true;
        e.effect = DragDropEffect::Move;
    }
});
target->drop().connect([](DragDropEvent& e) {
    auto id = std::any_cast<int>(e.data.value);
    // Handle the drop
});
```

## Signals

```cpp
Signal<> onFired;
Signal<int, const String&> onMessage;

// Connect
auto conn = onFired.connect([]() { /* handler */ });
onMessage.connect([](int code, const String& msg) { /* handler */ });

// Emit
onFired.emit();
onMessage.emit(42, "hello");

// Disconnect
onFired.disconnect(conn);

// RAII disconnect (disconnects when ScopedConnection is destroyed)
ScopedConnection sc(onFired, onFired.connect([]() { /* ... */ }));
```

## Styles & Themes

```cpp
// Built-in themes
ctx.setTheme(Theme::defaultDark());
ctx.setTheme(Theme::defaultLight());
```

## Custom Drawing

Override `onRender` on a Panel subclass:

```cpp
class MiniMap : public Panel {
    GUT_OBJECT(MiniMap, Panel)
public:
    void onRender(RenderContext& ctx) override {
        Panel::onRender(ctx);  // draws background, border, etc.
        Rectf r = {0, 0, bounds().width, bounds().height};

        // Draw custom content
        ctx.fillRect({10, 10, 50, 50}, Color::red());
        ctx.fillRoundedRect({70, 10, 80, 40}, 6.0f, Color::blue());
        ctx.strokeRect(r, Color::white(), 2.0f);
        ctx.drawDropShadow(r, 0, Color::fromRgba8(0,0,0,128), 8.0f, 0, 4);
    }
};
```

Or use a callback-based approach without subclassing:

```cpp
class DrawCanvas : public Panel {
    GUT_OBJECT(DrawCanvas, Panel)
public:
    std::function<void(RenderContext&, Rectf)> onDraw;
    void onRender(RenderContext& ctx) override {
        Panel::onRender(ctx);
        if (onDraw) onDraw(ctx, {0, 0, bounds().width, bounds().height});
    }
};

auto canvas = makeRef<DrawCanvas>();
canvas->onDraw = [](RenderContext& ctx, Rectf r) {
    ctx.fillRect(r, Color::fromRgba8(20, 20, 30));
};
```

## Game UI Example: HUD

```cpp
Ref<Element> buildHUD() {
    auto root = makeRef<DockPanel>();

    // Health bar at top
    auto topBar = makeRef<StackPanel>(Orientation::Horizontal);
    topBar->setmargin(Thickness{10, 10, 10, 0});
    topBar->setspacing(8.0f);
    DockPanel::setDock(*topBar, Dock::Top);

    auto hpBg = makeRef<Panel>();
    hpBg->setwidth(200); hpBg->setheight(20);
    hpBg->setcornerRadius(4); hpBg->setbackground(Color::fromRgba8(60, 15, 15));

    auto hpFill = makeRef<Panel>();
    hpFill->setwidth(156); hpFill->setheight(20);    // 78% HP
    hpFill->setcornerRadius(4);
    hpFill->setbackgroundGradientTop(Color::fromRgba8(255, 100, 80));
    hpFill->setbackgroundGradientBottom(Color::fromRgba8(120, 10, 5));
    hpBg->addChild(hpFill);

    auto hpText = makeRef<Text>("15,432 / 19,780", 10.0f);
    hpText->setforeground(Color::white());
    hpText->setmargin(Thickness{60, 2, 0, 0});
    hpBg->addChild(hpText);

    topBar->addChild(hpBg);
    root->addChild(topBar);

    // Action bar at bottom
    auto actionBar = makeRef<StackPanel>(Orientation::Horizontal);
    actionBar->setspacing(4.0f);
    actionBar->sethorizontalAlignment(HorizontalAlignment::Center);
    actionBar->setmargin(Thickness{0, 0, 0, 20});
    DockPanel::setDock(*actionBar, Dock::Bottom);

    for (int i = 0; i < 8; i++) {
        auto slot = makeRef<Panel>();
        slot->setwidth(48); slot->setheight(48);
        slot->setcornerRadius(4);
        slot->setbackground(Color::fromRgba8(30, 30, 40, 200));
        slot->setborderColor(Color::fromRgba8(80, 75, 60));
        slot->setborderWidth(1.0f);
        actionBar->addChild(slot);
    }
    root->addChild(actionBar);

    return root;
}
```

## Common Gotchas

1. **Setter naming**: `setwidth()` not `setWidth()`. All GUT_PROPERTY setters are lowercase after "set".

2. **Property access**: Use `.get()` and `.set()` on Property<T>, not `.value()` or `.setValue()`.

3. **Keep animation Refs alive**: Animations are destroyed when their Ref goes out of scope. Store them in a vector, member variable, or lambda capture.

4. **Signal access**: `Element::clicked()` returns `Signal<>&` (protected). Use `setOnClick()` instead for buttons and panels. `Animation::completed` is a public member accessed directly.

5. **ScopedConnection**: Construct with `ScopedConnection(signal, connection)`, not by assigning a Connection directly.

6. **Context::update()** takes seconds, not milliseconds. Animation durations are in milliseconds.

7. **Thickness** is `{left, top, right, bottom}`, not TRBL.

8. **Children**: Only Panel (and subclasses) can have children. Element itself cannot.

9. **Layout**: Call `invalidateLayout()` when changing properties that affect size. GUT_PROPERTY does this automatically for width/height/margin/padding.

10. **Canvas children**: Must set `Canvas::setLeft()` and `Canvas::setTop()` as static calls with element reference, e.g. `Canvas::setLeft(*child, 100.0f)`.
