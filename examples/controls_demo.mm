/**
 * @file controls_demo.mm
 * @brief Clean demo showcasing Gut interactive controls — TextBox, Button, etc.
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CoreVideo.h>

#include "gut_single.h"

// Include the Metal backend implementation
#include "metal_backend.mm"

#include <memory>

// =============================================================================
// macOS key code -> gut Key mapping
// =============================================================================
static gut::Key macKeyCodeToGutKey(unsigned short keyCode) {
    switch (keyCode) {
        case 0:   return gut::Key::A;
        case 1:   return gut::Key::S;
        case 2:   return gut::Key::D;
        case 3:   return gut::Key::F;
        case 4:   return gut::Key::H;
        case 5:   return gut::Key::G;
        case 6:   return gut::Key::Z;
        case 7:   return gut::Key::X;
        case 8:   return gut::Key::C;
        case 9:   return gut::Key::V;
        case 11:  return gut::Key::B;
        case 12:  return gut::Key::Q;
        case 13:  return gut::Key::W;
        case 14:  return gut::Key::E;
        case 15:  return gut::Key::R;
        case 16:  return gut::Key::Y;
        case 17:  return gut::Key::T;
        case 18:  return gut::Key::Num1;
        case 19:  return gut::Key::Num2;
        case 20:  return gut::Key::Num3;
        case 21:  return gut::Key::Num4;
        case 22:  return gut::Key::Num6;
        case 23:  return gut::Key::Num5;
        case 24:  return gut::Key::Equals;
        case 25:  return gut::Key::Num9;
        case 26:  return gut::Key::Num7;
        case 27:  return gut::Key::Minus;
        case 28:  return gut::Key::Num8;
        case 29:  return gut::Key::Num0;
        case 30:  return gut::Key::RightBracket;
        case 31:  return gut::Key::O;
        case 32:  return gut::Key::U;
        case 33:  return gut::Key::LeftBracket;
        case 34:  return gut::Key::I;
        case 35:  return gut::Key::P;
        case 36:  return gut::Key::Return;
        case 37:  return gut::Key::L;
        case 38:  return gut::Key::J;
        case 39:  return gut::Key::Apostrophe;
        case 40:  return gut::Key::K;
        case 41:  return gut::Key::Semicolon;
        case 42:  return gut::Key::Backslash;
        case 43:  return gut::Key::Comma;
        case 44:  return gut::Key::Slash;
        case 45:  return gut::Key::N;
        case 46:  return gut::Key::M;
        case 47:  return gut::Key::Period;
        case 48:  return gut::Key::Tab;
        case 49:  return gut::Key::Space;
        case 50:  return gut::Key::Grave;
        case 51:  return gut::Key::Backspace;
        case 53:  return gut::Key::Escape;
        case 76:  return gut::Key::NumpadEnter;
        case 115: return gut::Key::Home;
        case 116: return gut::Key::PageUp;
        case 117: return gut::Key::Delete;
        case 119: return gut::Key::End;
        case 121: return gut::Key::PageDown;
        case 123: return gut::Key::Left;
        case 124: return gut::Key::Right;
        case 125: return gut::Key::Down;
        case 126: return gut::Key::Up;
        default:  return gut::Key::Unknown;
    }
}

static gut::ModifierKeys macModifiersToGut(NSEventModifierFlags flags) {
    gut::ModifierKeys mods = gut::ModifierKeys::None;
    if (flags & NSEventModifierFlagShift)   mods = mods | gut::ModifierKeys::Shift;
    // Map Cmd to Control (gut uses Control as the "command" modifier)
    if (flags & NSEventModifierFlagCommand) mods = mods | gut::ModifierKeys::Control;
    if (flags & NSEventModifierFlagOption)  mods = mods | gut::ModifierKeys::Alt;
    return mods;
}

// =============================================================================
// Metal View
// =============================================================================

@interface ControlsView : NSView <NSTextInputClient> {
    std::unique_ptr<gut::Context> _context;
    std::vector<gut::Ref<gut::FloatAnimation>> _animations;
}
@property (nonatomic, strong) CAMetalLayer* metalLayer;
@property (nonatomic) CVDisplayLinkRef displayLink;
@end

@implementation ControlsView

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        [self setupMetal];
        [self setupGut];
        [self setupDisplayLink];
    }
    return self;
}

- (void)dealloc {
    if (_displayLink) {
        CVDisplayLinkStop(_displayLink);
        CVDisplayLinkRelease(_displayLink);
    }
}

- (void)setupMetal {
    self.wantsLayer = YES;
    self.metalLayer = [CAMetalLayer layer];
    self.metalLayer.device = MTLCreateSystemDefaultDevice();
    self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self.metalLayer.framebufferOnly = NO;
    self.layer = self.metalLayer;
}

- (void)setupGut {
    // Create Metal backend
    auto backend = std::make_unique<gut::MetalRenderBackend>(self.metalLayer);
    _context = std::make_unique<gut::Context>(std::move(backend));

    // Disable 3D cube background for a clean look
    static_cast<gut::MetalRenderBackend&>(_context->renderBackend()).enableScene3D(false);

    // Load system font
    {
        NSString* fontPath = @"/System/Library/Fonts/Supplemental/Arial.ttf";
        NSData* fontData = [NSData dataWithContentsOfFile:fontPath];
        if (fontData) {
            _context->loadFont(static_cast<const gut::u8*>(fontData.bytes), fontData.length);
            NSLog(@"Loaded font: %@ (%lu bytes)", fontPath, (unsigned long)fontData.length);
        } else {
            NSLog(@"WARNING: Could not load font at %@", fontPath);
        }
    }

    using namespace gut;
    auto c = [](u8 r, u8 g, u8 b, u8 a = 255) { return Color::fromRgba8(r, g, b, a); };

    const f32 W = 800, H = 750;

    auto root = make<Canvas>();
    root->setwidth(W);
    root->setheight(H);
    root->setbackground(c(30, 30, 38));

    // =========================================================================
    // Title
    // =========================================================================
    {
        auto title = make<Text>("Gut Controls Demo", 20.0f);
        title->setforeground(c(220, 220, 240));
        title->setisHitTestVisible(false);
        root->addChild(title);
        Canvas::setLeft(*title, 30);
        Canvas::setTop(*title, 24);

        auto subtitle = make<Text>("Interactive control showcase  \u2014  TextBox, Button, CheckBox, RadioButton, Toggle, Slider, ProgressBar, DropDown, TabControl", 11.0f);
        subtitle->setforeground(c(130, 130, 150));
        subtitle->setisHitTestVisible(false);
        root->addChild(subtitle);
        Canvas::setLeft(*subtitle, 30);
        Canvas::setTop(*subtitle, 52);
    }

    // Horizontal line under title
    {
        auto line = make<Panel>();
        line->setwidth(W - 60);
        line->setheight(1);
        line->setbackground(c(50, 50, 62));
        root->addChild(line);
        Canvas::setLeft(*line, 30);
        Canvas::setTop(*line, 75);
    }

    // =========================================================================
    // TabControl — the main organizer
    // =========================================================================
    auto tabControl = make<TabControl>();
    tabControl->setwidth(W - 40);
    tabControl->setheight(H - 95);
    tabControl->settabBarBackground(c(35, 35, 45));
    tabControl->settabBarBorderColor(c(55, 55, 68));
    tabControl->settabForeground(c(140, 140, 160));
    tabControl->settabActiveForeground(c(230, 230, 250));
    tabControl->settabActiveIndicator(c(80, 150, 240));
    tabControl->settabHoverBackground(c(50, 50, 65, 180));
    tabControl->settabBarHeight(34.0f);
    tabControl->settabFontSize(12.5f);
    root->addChild(tabControl);
    Canvas::setLeft(*tabControl, 20);
    Canvas::setTop(*tabControl, 82);

    // =====================================================================
    // TAB 1 — "Controls"  (TextBox, Button, CheckBox, RadioButton)
    // =====================================================================
    {
        auto page = make<Canvas>();
        page->setwidth(W - 40);
        page->setheight(H - 95 - 34);

        // ----- left column -----
        // SECTION: Text Input
        {
            f32 sx = 10, sy = 15;

            auto sectionLabel = make<Text>("Text Input", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto lbl1 = make<Text>("Name", 11.0f);
            lbl1->setforeground(c(160, 160, 180));
            lbl1->setisHitTestVisible(false);
            page->addChild(lbl1);
            Canvas::setLeft(*lbl1, sx);
            Canvas::setTop(*lbl1, sy + 28);

            auto tb1 = make<TextBox>();
            tb1->settabIndex(1);
            tb1->setwidth(240);
            tb1->setplaceholder("Enter your name...");
            tb1->setbackground(c(45, 45, 56));
            tb1->setforeground(c(220, 220, 240));
            tb1->setborderColor(c(70, 70, 85));
            tb1->setfocusBorderColor(c(80, 140, 220));
            tb1->setcaretColor(c(80, 180, 255));
            tb1->setselectionColor(c(80, 140, 220, 60));
            tb1->setplaceholderColor(c(90, 90, 110));
            tb1->setcornerRadius(6.0f);
            tb1->setfontSize(13.0f);
            page->addChild(tb1);
            Canvas::setLeft(*tb1, sx);
            Canvas::setTop(*tb1, sy + 45);

            auto lbl2 = make<Text>("Email", 11.0f);
            lbl2->setforeground(c(160, 160, 180));
            lbl2->setisHitTestVisible(false);
            page->addChild(lbl2);
            Canvas::setLeft(*lbl2, sx);
            Canvas::setTop(*lbl2, sy + 82);

            auto tb2 = make<TextBox>();
            tb2->settabIndex(2);
            tb2->setwidth(240);
            tb2->setplaceholder("you@example.com");
            tb2->setbackground(c(45, 45, 56));
            tb2->setforeground(c(220, 220, 240));
            tb2->setborderColor(c(70, 70, 85));
            tb2->setfocusBorderColor(c(80, 140, 220));
            tb2->setcaretColor(c(80, 180, 255));
            tb2->setselectionColor(c(80, 140, 220, 60));
            tb2->setplaceholderColor(c(90, 90, 110));
            tb2->setcornerRadius(6.0f);
            tb2->setfontSize(13.0f);
            page->addChild(tb2);
            Canvas::setLeft(*tb2, sx);
            Canvas::setTop(*tb2, sy + 99);

            auto lbl3 = make<Text>("Search", 11.0f);
            lbl3->setforeground(c(160, 160, 180));
            lbl3->setisHitTestVisible(false);
            page->addChild(lbl3);
            Canvas::setLeft(*lbl3, sx);
            Canvas::setTop(*lbl3, sy + 136);

            auto tb3 = make<TextBox>("Hello, World!");
            tb3->settabIndex(3);
            tb3->setwidth(240);
            tb3->setbackground(c(45, 45, 56));
            tb3->setforeground(c(220, 220, 240));
            tb3->setborderColor(c(70, 70, 85));
            tb3->setfocusBorderColor(c(80, 140, 220));
            tb3->setcaretColor(c(80, 180, 255));
            tb3->setselectionColor(c(80, 140, 220, 60));
            tb3->setcornerRadius(6.0f);
            tb3->setfontSize(13.0f);
            page->addChild(tb3);
            Canvas::setLeft(*tb3, sx);
            Canvas::setTop(*tb3, sy + 153);

            auto lbl4 = make<Text>("Bio  (scroll test \u2014 type a lot)", 11.0f);
            lbl4->setforeground(c(160, 160, 180));
            lbl4->setisHitTestVisible(false);
            page->addChild(lbl4);
            Canvas::setLeft(*lbl4, sx);
            Canvas::setTop(*lbl4, sy + 190);

            auto tb4 = make<TextBox>("The quick brown fox jumps over the lazy dog");
            tb4->settabIndex(4);
            tb4->setwidth(240);
            tb4->setbackground(c(45, 45, 56));
            tb4->setforeground(c(220, 220, 240));
            tb4->setborderColor(c(70, 70, 85));
            tb4->setfocusBorderColor(c(80, 140, 220));
            tb4->setcaretColor(c(80, 180, 255));
            tb4->setselectionColor(c(80, 140, 220, 60));
            tb4->setcornerRadius(6.0f);
            tb4->setfontSize(13.0f);
            page->addChild(tb4);
            Canvas::setLeft(*tb4, sx);
            Canvas::setTop(*tb4, sy + 207);

            auto lbl5 = make<Text>("Read-only", 11.0f);
            lbl5->setforeground(c(160, 160, 180));
            lbl5->setisHitTestVisible(false);
            page->addChild(lbl5);
            Canvas::setLeft(*lbl5, sx);
            Canvas::setTop(*lbl5, sy + 244);

            auto tb5 = make<TextBox>("This text cannot be edited");
            tb5->settabIndex(5);
            tb5->setwidth(240);
            tb5->setreadOnly(true);
            tb5->setbackground(c(38, 38, 46));
            tb5->setforeground(c(140, 140, 160));
            tb5->setborderColor(c(55, 55, 65));
            tb5->setcornerRadius(6.0f);
            tb5->setfontSize(13.0f);
            page->addChild(tb5);
            Canvas::setLeft(*tb5, sx);
            Canvas::setTop(*tb5, sy + 261);
        }

        // SECTION: Buttons
        {
            f32 sx = 10, sy = 320;

            auto sectionLabel = make<Text>("Buttons", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto btn1 = make<Button>("Submit");
            btn1->setwidth(100);
            btn1->setheight(32);
            btn1->setbackground(c(60, 130, 220));
            btn1->setforeground(c(255, 255, 255));
            btn1->sethoverBackground(c(70, 140, 235));
            btn1->setpressedBackground(c(50, 110, 190));
            btn1->setborderColor(c(80, 150, 240));
            btn1->setcornerRadius(6.0f);
            page->addChild(btn1);
            Canvas::setLeft(*btn1, sx);
            Canvas::setTop(*btn1, sy + 28);

            auto btn2 = make<Button>("Cancel");
            btn2->setwidth(100);
            btn2->setheight(32);
            btn2->setbackground(c(55, 55, 65));
            btn2->setforeground(c(200, 200, 220));
            btn2->sethoverBackground(c(65, 65, 78));
            btn2->setpressedBackground(c(45, 45, 55));
            btn2->setborderColor(c(70, 70, 85));
            btn2->setcornerRadius(6.0f);
            page->addChild(btn2);
            Canvas::setLeft(*btn2, sx + 110);
            Canvas::setTop(*btn2, sy + 28);

            auto btn3 = make<Button>("Delete");
            btn3->setwidth(100);
            btn3->setheight(32);
            btn3->setbackground(c(180, 50, 50));
            btn3->setforeground(c(255, 255, 255));
            btn3->sethoverBackground(c(200, 60, 60));
            btn3->setpressedBackground(c(150, 40, 40));
            btn3->setborderColor(c(200, 70, 70));
            btn3->setcornerRadius(6.0f);
            page->addChild(btn3);
            Canvas::setLeft(*btn3, sx + 220);
            Canvas::setTop(*btn3, sy + 28);

            auto btn4 = make<Button>("Pill Style");
            btn4->setwidth(90);
            btn4->setheight(26);
            btn4->setbackground(c(70, 50, 120));
            btn4->setforeground(c(200, 180, 255));
            btn4->sethoverBackground(c(85, 60, 140));
            btn4->setpressedBackground(c(55, 40, 100));
            btn4->setborderColor(c(100, 70, 160));
            btn4->setcornerRadius(13.0f);
            page->addChild(btn4);
            Canvas::setLeft(*btn4, sx);
            Canvas::setTop(*btn4, sy + 70);

            auto btn5 = make<Button>("Disabled");
            btn5->setwidth(100);
            btn5->setheight(32);
            btn5->setisEnabled(false);
            btn5->setbackground(c(45, 45, 52));
            btn5->setforeground(c(100, 100, 110));
            btn5->setdisabledBackground(c(45, 45, 52));
            btn5->setborderColor(c(55, 55, 62));
            btn5->setcornerRadius(6.0f);
            page->addChild(btn5);
            Canvas::setLeft(*btn5, sx + 100);
            Canvas::setTop(*btn5, sy + 70);
        }

        // SECTION: CheckBox
        Ref<Text> cbStatus;
        {
            f32 sx = 10, sy = 425;

            auto sectionLabel = make<Text>("CheckBox", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto cb1 = make<CheckBox>("Enable notifications");
            cb1->settabIndex(30);
            page->addChild(cb1);
            Canvas::setLeft(*cb1, sx);
            Canvas::setTop(*cb1, sy + 26);

            auto cb2 = make<CheckBox>("Dark mode");
            cb2->settabIndex(31);
            cb2->setisChecked(true);
            page->addChild(cb2);
            Canvas::setLeft(*cb2, sx);
            Canvas::setTop(*cb2, sy + 50);

            auto cb3 = make<CheckBox>("Auto-save");
            cb3->settabIndex(32);
            page->addChild(cb3);
            Canvas::setLeft(*cb3, sx);
            Canvas::setTop(*cb3, sy + 74);

            auto cb4 = make<CheckBox>("Disabled option");
            cb4->setisEnabled(false);
            cb4->setisChecked(true);
            page->addChild(cb4);
            Canvas::setLeft(*cb4, sx + 170);
            Canvas::setTop(*cb4, sy + 26);

            auto statusLine = make<Text>("Toggle any checkbox...", 10.0f);
            statusLine->setforeground(c(110, 110, 130));
            statusLine->setisHitTestVisible(false);
            cbStatus = statusLine;
            page->addChild(statusLine);
            Canvas::setLeft(*statusLine, sx);
            Canvas::setTop(*statusLine, sy + 100);

            auto s = cbStatus;
            cb1->setOnCheckedChanged([s](bool checked) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext(checked ? "Notifications: ON" : "Notifications: OFF");
            });
            cb2->setOnCheckedChanged([s](bool checked) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext(checked ? "Dark mode: ON" : "Dark mode: OFF");
            });
            cb3->setOnCheckedChanged([s](bool checked) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext(checked ? "Auto-save: ON" : "Auto-save: OFF");
            });
        }

        // ----- right column -----

        // Vertical divider
        {
            auto vline = make<Panel>();
            vline->setwidth(1);
            vline->setheight(420);
            vline->setbackground(c(50, 50, 62));
            page->addChild(vline);
            Canvas::setLeft(*vline, 370);
            Canvas::setTop(*vline, 10);
        }

        // SECTION: RadioButton
        Ref<Text> rbStatus;
        {
            f32 sx = 395, sy = 15;

            auto sectionLabel = make<Text>("RadioButton", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto groupPanel = make<Canvas>();
            groupPanel->setwidth(320);
            groupPanel->setheight(90);
            page->addChild(groupPanel);
            Canvas::setLeft(*groupPanel, sx);
            Canvas::setTop(*groupPanel, sy + 24);

            const char* sizes[] = {"Small", "Medium", "Large", "Extra Large"};
            for (int i = 0; i < 4; i++) {
                auto rb = make<RadioButton>(sizes[i], "size");
                rb->settabIndex(40 + i);
                if (i == 1) rb->setisChecked(true);
                groupPanel->addChild(rb);
                Canvas::setLeft(*rb, 0);
                Canvas::setTop(*rb, (f32)(i * 22));
            }

            auto statusLine = make<Text>("Selected: Medium", 10.0f);
            statusLine->setforeground(c(110, 110, 130));
            statusLine->setisHitTestVisible(false);
            rbStatus = statusLine;
            page->addChild(statusLine);
            Canvas::setLeft(*statusLine, sx);
            Canvas::setTop(*statusLine, sy + 118);

            auto s = rbStatus;
            for (usize i = 0; i < groupPanel->childCount(); i++) {
                auto* rb = static_cast<gut::RadioButton*>(groupPanel->childAt(i));
                gut::String name = sizes[i];
                rb->setOnCheckedChanged([s, name](bool checked) {
                    if (checked) {
                        s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                        s->settext("Selected: " + name);
                    }
                });
            }
        }

        // SECTION: Live echo
        {
            f32 sx = 395, sy = 155;

            auto sectionLabel = make<Text>("Live Text Echo", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto echoLabel = make<Text>("Type below and see it echoed in real time:", 11.0f);
            echoLabel->setforeground(c(140, 140, 160));
            echoLabel->setisHitTestVisible(false);
            page->addChild(echoLabel);
            Canvas::setLeft(*echoLabel, sx);
            Canvas::setTop(*echoLabel, sy + 25);

            auto echoText = make<Text>("...", 16.0f);
            echoText->setforeground(c(80, 200, 255));
            echoText->setisHitTestVisible(false);
            page->addChild(echoText);
            Canvas::setLeft(*echoText, sx);
            Canvas::setTop(*echoText, sy + 80);

            auto echoBox = make<TextBox>();
            echoBox->settabIndex(20);
            echoBox->setwidth(320);
            echoBox->setplaceholder("Start typing...");
            echoBox->setbackground(c(45, 45, 56));
            echoBox->setforeground(c(220, 220, 240));
            echoBox->setborderColor(c(70, 70, 85));
            echoBox->setfocusBorderColor(c(80, 140, 220));
            echoBox->setcaretColor(c(80, 180, 255));
            echoBox->setselectionColor(c(80, 140, 220, 60));
            echoBox->setplaceholderColor(c(90, 90, 110));
            echoBox->setcornerRadius(6.0f);
            echoBox->setfontSize(14.0f);
            page->addChild(echoBox);
            Canvas::setLeft(*echoBox, sx);
            Canvas::setTop(*echoBox, sy + 47);

            echoBox->setOnTextChanged([echoText](const gut::String& newText) {
                if (newText.empty()) {
                    echoText->settext("...");
                } else {
                    echoText->settext(newText);
                }
            });
        }

        // SECTION: Keyboard hints (bottom of page)
        {
            f32 sy = 545;
            const char* hints[] = {
                "\xe2\x86\x90\xe2\x86\x92  Move caret",
                "Shift+Arrows  Select text",
                "\xe2\x8c\x98+Arrows  Word jump",
                "Home/End  Start/end",
                "\xe2\x8c\x98+A  Select all",
                "Dbl-click  Select word",
                "Tab  Next field",
            };
            f32 x = 10;
            for (auto& hint : hints) {
                auto t = make<Text>(hint, 9.5f);
                t->setforeground(c(100, 100, 120));
                t->setisHitTestVisible(false);
                page->addChild(t);
                Canvas::setLeft(*t, x);
                Canvas::setTop(*t, sy);
                x += 105;
            }
        }

        tabControl->addTab("Controls", page);
    }

    // =====================================================================
    // TAB 2 — "Selectors & Forms"  (DropDown, Login Form)
    // =====================================================================
    {
        auto page = make<Canvas>();
        page->setwidth(W - 40);
        page->setheight(H - 95 - 34);

        // ----- left column: DropDown -----
        Ref<Text> ddStatus;
        {
            f32 sx = 10, sy = 15;

            auto sectionLabel = make<Text>("DropDown", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            // Theme selector
            auto themeLabel = make<Text>("Theme:", 11.0f);
            themeLabel->setforeground(c(140, 140, 160));
            themeLabel->setisHitTestVisible(false);
            page->addChild(themeLabel);
            Canvas::setLeft(*themeLabel, sx);
            Canvas::setTop(*themeLabel, sy + 34);

            auto themeDrop = make<DropDown>();
            themeDrop->settabIndex(50);
            themeDrop->setwidth(180);
            themeDrop->addItem("Dark");
            themeDrop->addItem("Light");
            themeDrop->addItem("Solarized");
            themeDrop->addItem("Monokai");
            themeDrop->addItem("Nord");
            themeDrop->setselectedIndex(0);
            themeDrop->setplaceholder("Pick a theme...");
            page->addChild(themeDrop);
            Canvas::setLeft(*themeDrop, sx + 65);
            Canvas::setTop(*themeDrop, sy + 30);

            // Country selector — many items, scrollable
            auto countryLabel = make<Text>("Country:", 11.0f);
            countryLabel->setforeground(c(140, 140, 160));
            countryLabel->setisHitTestVisible(false);
            page->addChild(countryLabel);
            Canvas::setLeft(*countryLabel, sx);
            Canvas::setTop(*countryLabel, sy + 74);

            auto countryDrop = make<DropDown>();
            countryDrop->settabIndex(51);
            countryDrop->setwidth(220);
            countryDrop->setmaxDropHeight(160);
            const char* countries[] = {
                "Argentina", "Australia", "Brazil", "Canada", "Chile",
                "China", "Denmark", "Egypt", "Finland", "France",
                "Germany", "Greece", "India", "Ireland", "Italy",
                "Japan", "Kenya", "Mexico", "Netherlands", "New Zealand",
                "Norway", "Peru", "Poland", "Portugal", "South Korea",
                "Spain", "Sweden", "Switzerland", "Thailand", "United Kingdom",
                "United States", "Vietnam"
            };
            for (auto& name : countries) {
                countryDrop->addItem(name);
            }
            countryDrop->setplaceholder("Select a country...");
            page->addChild(countryDrop);
            Canvas::setLeft(*countryDrop, sx + 65);
            Canvas::setTop(*countryDrop, sy + 70);

            // Disabled dropdown
            auto disabledLabel = make<Text>("Locked:", 11.0f);
            disabledLabel->setforeground(c(140, 140, 160));
            disabledLabel->setisHitTestVisible(false);
            page->addChild(disabledLabel);
            Canvas::setLeft(*disabledLabel, sx);
            Canvas::setTop(*disabledLabel, sy + 114);

            auto disabledDrop = make<DropDown>();
            disabledDrop->addItem("Locked");
            disabledDrop->setselectedIndex(0);
            disabledDrop->setisEnabled(false);
            disabledDrop->setwidth(120);
            page->addChild(disabledDrop);
            Canvas::setLeft(*disabledDrop, sx + 65);
            Canvas::setTop(*disabledDrop, sy + 110);

            auto statusLine = make<Text>("Pick an item from a dropdown...", 10.0f);
            statusLine->setforeground(c(110, 110, 130));
            statusLine->setisHitTestVisible(false);
            ddStatus = statusLine;
            page->addChild(statusLine);
            Canvas::setLeft(*statusLine, sx);
            Canvas::setTop(*statusLine, sy + 150);

            auto s = ddStatus;
            themeDrop->setOnSelectionChanged([s, themeDrop](isize idx) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext("Theme: " + themeDrop->selectedItem());
            });
            countryDrop->setOnSelectionChanged([s, countryDrop](isize idx) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext("Country: " + countryDrop->selectedItem());
            });
        }

        // ----- right column: Login Form -----

        // Vertical divider
        {
            auto vline = make<Panel>();
            vline->setwidth(1);
            vline->setheight(530);
            vline->setbackground(c(50, 50, 62));
            page->addChild(vline);
            Canvas::setLeft(*vline, 370);
            Canvas::setTop(*vline, 10);
        }

        Ref<Text> statusText;
        {
            f32 sx = 395, sy = 15;

            auto sectionLabel = make<Text>("Login Form", 13.0f);
            sectionLabel->setforeground(c(180, 180, 200));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto card = make<Panel>();
            card->setwidth(320);
            card->setheight(220);
            card->setbackground(c(40, 40, 50));
            card->setborderColor(c(60, 60, 72));
            card->setborderWidth(1.0f);
            card->setcornerRadius(10.0f);
            card->setisHitTestVisible(false);
            page->addChild(card);
            Canvas::setLeft(*card, sx);
            Canvas::setTop(*card, sy + 25);

            auto uLbl = make<Text>("Username", 11.0f);
            uLbl->setforeground(c(160, 160, 180));
            uLbl->setisHitTestVisible(false);
            page->addChild(uLbl);
            Canvas::setLeft(*uLbl, sx + 20);
            Canvas::setTop(*uLbl, sy + 45);

            auto userBox = make<TextBox>();
            userBox->settabIndex(10);
            userBox->setwidth(280);
            userBox->setplaceholder("Enter username...");
            userBox->setbackground(c(50, 50, 62));
            userBox->setforeground(c(220, 220, 240));
            userBox->setborderColor(c(65, 65, 78));
            userBox->setfocusBorderColor(c(80, 140, 220));
            userBox->setcaretColor(c(80, 180, 255));
            userBox->setselectionColor(c(80, 140, 220, 60));
            userBox->setplaceholderColor(c(90, 90, 110));
            userBox->setcornerRadius(6.0f);
            userBox->setfontSize(13.0f);
            page->addChild(userBox);
            Canvas::setLeft(*userBox, sx + 20);
            Canvas::setTop(*userBox, sy + 62);

            auto pLbl = make<Text>("Password", 11.0f);
            pLbl->setforeground(c(160, 160, 180));
            pLbl->setisHitTestVisible(false);
            page->addChild(pLbl);
            Canvas::setLeft(*pLbl, sx + 20);
            Canvas::setTop(*pLbl, sy + 100);

            auto passBox = make<TextBox>();
            passBox->settabIndex(11);
            passBox->setisPassword(true);
            passBox->setwidth(280);
            passBox->setplaceholder("Enter password...");
            passBox->setbackground(c(50, 50, 62));
            passBox->setforeground(c(220, 220, 240));
            passBox->setborderColor(c(65, 65, 78));
            passBox->setfocusBorderColor(c(80, 140, 220));
            passBox->setcaretColor(c(80, 180, 255));
            passBox->setselectionColor(c(80, 140, 220, 60));
            passBox->setplaceholderColor(c(90, 90, 110));
            passBox->setcornerRadius(6.0f);
            passBox->setfontSize(13.0f);
            page->addChild(passBox);
            Canvas::setLeft(*passBox, sx + 20);
            Canvas::setTop(*passBox, sy + 117);

            auto loginBtn = make<Button>("Log In");
            loginBtn->settabIndex(12);
            loginBtn->setwidth(280);
            loginBtn->setheight(34);
            loginBtn->setbackground(c(60, 130, 220));
            loginBtn->setforeground(c(255, 255, 255));
            loginBtn->sethoverBackground(c(70, 140, 235));
            loginBtn->setpressedBackground(c(50, 110, 190));
            loginBtn->setborderColor(c(80, 150, 240));
            loginBtn->setcornerRadius(6.0f);
            page->addChild(loginBtn);
            Canvas::setLeft(*loginBtn, sx + 20);
            Canvas::setTop(*loginBtn, sy + 160);

            auto status = make<Text>("Tab between fields, type to enter text", 10.0f);
            status->setforeground(c(110, 110, 130));
            status->setisHitTestVisible(false);
            statusText = status;
            page->addChild(status);
            Canvas::setLeft(*status, sx + 20);
            Canvas::setTop(*status, sy + 205);

            loginBtn->setOnClick([userBox, passBox, status]() {
                if (userBox->text().empty()) {
                    status->setforeground(gut::Color::fromRgba8(220, 80, 80));
                    status->settext("Please enter a username");
                } else if (passBox->text().empty()) {
                    status->setforeground(gut::Color::fromRgba8(220, 80, 80));
                    status->settext("Please enter a password");
                } else {
                    status->setforeground(gut::Color::fromRgba8(80, 200, 120));
                    status->settext("Welcome, " + userBox->text() + "!");
                }
            });
        }

        tabControl->addTab("Selectors & Forms", page);
    }

    // =====================================================================
    // TAB 3 — "Toggle & Slider"
    // =====================================================================
    {
        auto page = make<Canvas>();
        page->setwidth(W - 40);
        page->setheight(H - 95 - 34);

        // ── Left column: Toggle / Switch ──────────────────────────────
        Ref<Text> tgStatus;
        {
            f32 sx = 20, sy = 15;

            auto sectionLabel = make<Text>("Toggle / Switch", 15.0f);
            sectionLabel->setforeground(c(200, 200, 220));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto desc = make<Text>("Pill-shaped on/off switches with color themes", 11.0f);
            desc->setforeground(c(120, 120, 140));
            desc->setisHitTestVisible(false);
            page->addChild(desc);
            Canvas::setLeft(*desc, sx);
            Canvas::setTop(*desc, sy + 24);

            // Toggle: Dark mode (green, default on)
            auto tg1 = make<Toggle>("Dark mode");
            tg1->settabIndex(60);
            tg1->setisOn(true);
            page->addChild(tg1);
            Canvas::setLeft(*tg1, sx);
            Canvas::setTop(*tg1, sy + 56);

            // Toggle: Notifications (off)
            auto tg2 = make<Toggle>("Notifications");
            tg2->settabIndex(61);
            page->addChild(tg2);
            Canvas::setLeft(*tg2, sx);
            Canvas::setTop(*tg2, sy + 92);

            // Toggle: Auto-update (blue, on)
            auto tg3 = make<Toggle>("Auto-update");
            tg3->settabIndex(62);
            tg3->setisOn(true);
            tg3->settrackOnBackground(c(80, 120, 220));
            tg3->settrackOnBorderColor(c(70, 110, 200));
            page->addChild(tg3);
            Canvas::setLeft(*tg3, sx);
            Canvas::setTop(*tg3, sy + 128);

            // Toggle: Sound (orange, on)
            auto tg4 = make<Toggle>("Sound effects");
            tg4->settabIndex(63);
            tg4->setisOn(true);
            tg4->settrackOnBackground(c(230, 140, 50));
            tg4->settrackOnBorderColor(c(210, 120, 40));
            page->addChild(tg4);
            Canvas::setLeft(*tg4, sx);
            Canvas::setTop(*tg4, sy + 164);

            // Toggle: Disabled (greyed)
            auto tg5 = make<Toggle>("Disabled switch");
            tg5->setisEnabled(false);
            tg5->setisOn(true);
            page->addChild(tg5);
            Canvas::setLeft(*tg5, sx);
            Canvas::setTop(*tg5, sy + 200);

            // Custom-sized toggle
            auto bigLabel = make<Text>("Large toggle:", 10.5f);
            bigLabel->setforeground(c(140, 140, 160));
            bigLabel->setisHitTestVisible(false);
            page->addChild(bigLabel);
            Canvas::setLeft(*bigLabel, sx);
            Canvas::setTop(*bigLabel, sy + 246);

            auto tgBig = make<Toggle>("Wi-Fi");
            tgBig->settabIndex(64);
            tgBig->settrackWidth(56.0f);
            tgBig->settrackHeight(30.0f);
            tgBig->setthumbInset(3.0f);
            tgBig->setfontSize(15.0f);
            tgBig->settrackOnBackground(c(60, 180, 200));
            tgBig->settrackOnBorderColor(c(50, 160, 180));
            page->addChild(tgBig);
            Canvas::setLeft(*tgBig, sx);
            Canvas::setTop(*tgBig, sy + 266);

            // Status line
            auto statusLine = make<Text>("Flip a switch...", 10.0f);
            statusLine->setforeground(c(110, 110, 130));
            statusLine->setisHitTestVisible(false);
            tgStatus = statusLine;
            page->addChild(statusLine);
            Canvas::setLeft(*statusLine, sx);
            Canvas::setTop(*statusLine, sy + 310);

            auto s = tgStatus;
            tg1->setOnToggled([s](bool on) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext(on ? "Dark mode: ON" : "Dark mode: OFF");
            });
            tg2->setOnToggled([s](bool on) {
                s->setforeground(gut::Color::fromRgba8(80, 200, 120));
                s->settext(on ? "Notifications: ON" : "Notifications: OFF");
            });
            tg3->setOnToggled([s](bool on) {
                s->setforeground(gut::Color::fromRgba8(100, 160, 240));
                s->settext(on ? "Auto-update: ON" : "Auto-update: OFF");
            });
            tg4->setOnToggled([s](bool on) {
                s->setforeground(gut::Color::fromRgba8(230, 160, 80));
                s->settext(on ? "Sound effects: ON" : "Sound effects: OFF");
            });
            tgBig->setOnToggled([s](bool on) {
                s->setforeground(gut::Color::fromRgba8(60, 200, 220));
                s->settext(on ? "Wi-Fi: ON" : "Wi-Fi: OFF");
            });
        }

        // Vertical divider
        {
            auto vline = make<Panel>();
            vline->setwidth(1);
            vline->setheight(520);
            vline->setbackground(c(50, 50, 62));
            page->addChild(vline);
            Canvas::setLeft(*vline, 350);
            Canvas::setTop(*vline, 10);
        }

        // ── Right column: Slider ──────────────────────────────────────
        Ref<Text> sliderStatus;
        {
            f32 sx = 375, sy = 15;

            auto sectionLabel = make<Text>("Slider", 15.0f);
            sectionLabel->setforeground(c(200, 200, 220));
            sectionLabel->setisHitTestVisible(false);
            page->addChild(sectionLabel);
            Canvas::setLeft(*sectionLabel, sx);
            Canvas::setTop(*sectionLabel, sy);

            auto desc = make<Text>("Drag or use arrow keys. Home/End jump to extremes.", 11.0f);
            desc->setforeground(c(120, 120, 140));
            desc->setisHitTestVisible(false);
            page->addChild(desc);
            Canvas::setLeft(*desc, sx);
            Canvas::setTop(*desc, sy + 24);

            // Volume (0-100, step 1, green)
            auto volLabel = make<Text>("Volume", 11.0f);
            volLabel->setforeground(c(160, 160, 180));
            volLabel->setisHitTestVisible(false);
            page->addChild(volLabel);
            Canvas::setLeft(*volLabel, sx);
            Canvas::setTop(*volLabel, sy + 56);

            auto vol = make<Slider>(75.0f);
            vol->settabIndex(70);
            vol->setminimum(0.0f);
            vol->setmaximum(100.0f);
            vol->setstep(1.0f);
            vol->setshowValue(true);
            vol->setpreferredWidth(280.0f);
            vol->settrackFillColor(c(100, 180, 80));
            page->addChild(vol);
            Canvas::setLeft(*vol, sx);
            Canvas::setTop(*vol, sy + 76);

            // Brightness (0-100, step 5, yellow)
            auto briLabel = make<Text>("Brightness", 11.0f);
            briLabel->setforeground(c(160, 160, 180));
            briLabel->setisHitTestVisible(false);
            page->addChild(briLabel);
            Canvas::setLeft(*briLabel, sx);
            Canvas::setTop(*briLabel, sy + 108);

            auto bri = make<Slider>(50.0f);
            bri->settabIndex(71);
            bri->setminimum(0.0f);
            bri->setmaximum(100.0f);
            bri->setstep(5.0f);
            bri->setshowValue(true);
            bri->setpreferredWidth(280.0f);
            bri->settrackFillColor(c(240, 200, 60));
            page->addChild(bri);
            Canvas::setLeft(*bri, sx);
            Canvas::setTop(*bri, sy + 128);

            // Opacity (0.0-1.0, continuous, blue)
            auto opLabel = make<Text>("Opacity  (continuous)", 11.0f);
            opLabel->setforeground(c(160, 160, 180));
            opLabel->setisHitTestVisible(false);
            page->addChild(opLabel);
            Canvas::setLeft(*opLabel, sx);
            Canvas::setTop(*opLabel, sy + 160);

            auto op = make<Slider>(0.5f);
            op->settabIndex(72);
            op->setshowValue(true);
            op->setpreferredWidth(280.0f);
            page->addChild(op);
            Canvas::setLeft(*op, sx);
            Canvas::setTop(*op, sy + 180);

            // Temperature (–20 to 50, step 1, red)
            auto tempLabel = make<Text>("Temperature  (\u2013 20 \u2026 50)", 11.0f);
            tempLabel->setforeground(c(160, 160, 180));
            tempLabel->setisHitTestVisible(false);
            page->addChild(tempLabel);
            Canvas::setLeft(*tempLabel, sx);
            Canvas::setTop(*tempLabel, sy + 212);

            auto temp = make<Slider>(22.0f);
            temp->settabIndex(73);
            temp->setminimum(-20.0f);
            temp->setmaximum(50.0f);
            temp->setstep(1.0f);
            temp->setshowValue(true);
            temp->setpreferredWidth(280.0f);
            temp->settrackFillColor(c(220, 80, 80));
            page->addChild(temp);
            Canvas::setLeft(*temp, sx);
            Canvas::setTop(*temp, sy + 232);

            // Disabled slider
            auto disLabel = make<Text>("Disabled", 11.0f);
            disLabel->setforeground(c(120, 120, 140));
            disLabel->setisHitTestVisible(false);
            page->addChild(disLabel);
            Canvas::setLeft(*disLabel, sx);
            Canvas::setTop(*disLabel, sy + 270);

            auto dis = make<Slider>(30.0f);
            dis->setminimum(0.0f);
            dis->setmaximum(100.0f);
            dis->setstep(1.0f);
            dis->setshowValue(true);
            dis->setpreferredWidth(280.0f);
            dis->setisEnabled(false);
            page->addChild(dis);
            Canvas::setLeft(*dis, sx);
            Canvas::setTop(*dis, sy + 290);

            // Status line
            auto sLine = make<Text>("Drag a slider...", 10.0f);
            sLine->setforeground(c(110, 110, 130));
            sLine->setisHitTestVisible(false);
            sliderStatus = sLine;
            page->addChild(sLine);
            Canvas::setLeft(*sLine, sx);
            Canvas::setTop(*sLine, sy + 325);

            auto ss = sliderStatus;
            vol->setOnValueChanged([ss](f32 v) {
                char buf[64]; std::snprintf(buf, sizeof(buf), "Volume: %.0f", v);
                ss->setforeground(gut::Color::fromRgba8(100, 180, 80));
                ss->settext(buf);
            });
            bri->setOnValueChanged([ss](f32 v) {
                char buf[64]; std::snprintf(buf, sizeof(buf), "Brightness: %.0f", v);
                ss->setforeground(gut::Color::fromRgba8(240, 200, 60));
                ss->settext(buf);
            });
            op->setOnValueChanged([ss](f32 v) {
                char buf[64]; std::snprintf(buf, sizeof(buf), "Opacity: %.2f", v);
                ss->setforeground(gut::Color::fromRgba8(100, 160, 240));
                ss->settext(buf);
            });
            temp->setOnValueChanged([ss](f32 v) {
                char buf[64]; std::snprintf(buf, sizeof(buf), "Temperature: %.0f \u00b0C", v);
                ss->setforeground(gut::Color::fromRgba8(220, 100, 100));
                ss->settext(buf);
            });
        }

        tabControl->addTab("Toggle & Slider", page);
    }

    // =====================================================================
    // TAB 4 — "ProgressBar"
    // =====================================================================
    {
        auto page = make<Canvas>();
        page->setwidth(W - 40);
        page->setheight(H - 95 - 34);

        f32 sx = 20, sy = 15;

        auto sectionLabel = make<Text>("ProgressBar", 15.0f);
        sectionLabel->setforeground(c(200, 200, 220));
        sectionLabel->setisHitTestVisible(false);
        page->addChild(sectionLabel);
        Canvas::setLeft(*sectionLabel, sx);
        Canvas::setTop(*sectionLabel, sy);

        auto desc = make<Text>("Determinate and indeterminate progress indicators", 11.0f);
        desc->setforeground(c(120, 120, 140));
        desc->setisHitTestVisible(false);
        page->addChild(desc);
        Canvas::setLeft(*desc, sx);
        Canvas::setTop(*desc, sy + 24);

        // 1. Basic progress bar at 0%
        auto lbl1 = make<Text>("Download  (0%)", 11.0f);
        lbl1->setforeground(c(160, 160, 180));
        lbl1->setisHitTestVisible(false);
        page->addChild(lbl1);
        Canvas::setLeft(*lbl1, sx);
        Canvas::setTop(*lbl1, sy + 60);

        auto pb1 = make<ProgressBar>(0.0f);
        pb1->setpreferredWidth(350.0f);
        pb1->setshowLabel(true);
        page->addChild(pb1);
        Canvas::setLeft(*pb1, sx);
        Canvas::setTop(*pb1, sy + 80);

        // 2. 35% — blue with label below
        auto lbl2 = make<Text>("Uploading...", 11.0f);
        lbl2->setforeground(c(160, 160, 180));
        lbl2->setisHitTestVisible(false);
        page->addChild(lbl2);
        Canvas::setLeft(*lbl2, sx);
        Canvas::setTop(*lbl2, sy + 114);

        auto pb2 = make<ProgressBar>(35.0f);
        pb2->setpreferredWidth(350.0f);
        pb2->setshowLabel(true);
        page->addChild(pb2);
        Canvas::setLeft(*pb2, sx);
        Canvas::setTop(*pb2, sy + 134);

        // 3. 72% — green fill, % inside bar
        auto lbl3 = make<Text>("Installing...", 11.0f);
        lbl3->setforeground(c(160, 160, 180));
        lbl3->setisHitTestVisible(false);
        page->addChild(lbl3);
        Canvas::setLeft(*lbl3, sx);
        Canvas::setTop(*lbl3, sy + 168);

        auto pb3 = make<ProgressBar>(72.0f);
        pb3->setpreferredWidth(350.0f);
        pb3->setbarHeight(18.0f);
        pb3->setcornerRadius(9.0f);
        pb3->setfillColor(c(60, 180, 120));
        pb3->setcompletedFillColor(c(60, 190, 80));
        pb3->setshowPercentInBar(true);
        pb3->setfontSize(11.0f);
        page->addChild(pb3);
        Canvas::setLeft(*pb3, sx);
        Canvas::setTop(*pb3, sy + 188);

        // 4. 100% complete
        auto lbl4 = make<Text>("Complete!", 11.0f);
        lbl4->setforeground(c(100, 200, 120));
        lbl4->setisHitTestVisible(false);
        page->addChild(lbl4);
        Canvas::setLeft(*lbl4, sx);
        Canvas::setTop(*lbl4, sy + 224);

        auto pb4 = make<ProgressBar>(100.0f);
        pb4->setpreferredWidth(350.0f);
        pb4->setbarHeight(12.0f);
        pb4->setcornerRadius(6.0f);
        pb4->setshowLabel(true);
        page->addChild(pb4);
        Canvas::setLeft(*pb4, sx);
        Canvas::setTop(*pb4, sy + 244);

        // --- Right column: Interactive slider-driven progress ---

        f32 rx = 420, ry = 60;

        auto interLabel = make<Text>("Interactive  \u2014  drive with slider", 13.0f);
        interLabel->setforeground(c(200, 200, 220));
        interLabel->setisHitTestVisible(false);
        page->addChild(interLabel);
        Canvas::setLeft(*interLabel, rx);
        Canvas::setTop(*interLabel, sy);

        // Tall themed progress bar
        auto pb5 = make<ProgressBar>(50.0f);
        pb5->setpreferredWidth(300.0f);
        pb5->setbarHeight(22.0f);
        pb5->setcornerRadius(11.0f);
        pb5->setfillColor(c(200, 120, 240));
        pb5->setcompletedFillColor(c(120, 220, 80));
        pb5->setshowPercentInBar(true);
        pb5->setfontSize(12.0f);
        page->addChild(pb5);
        Canvas::setLeft(*pb5, rx);
        Canvas::setTop(*pb5, ry);

        // Slider to drive it
        auto driver = make<Slider>(50.0f);
        driver->settabIndex(80);
        driver->setminimum(0.0f);
        driver->setmaximum(100.0f);
        driver->setstep(1.0f);
        driver->setshowValue(true);
        driver->setpreferredWidth(300.0f);
        driver->settrackFillColor(c(200, 120, 240));
        page->addChild(driver);
        Canvas::setLeft(*driver, rx);
        Canvas::setTop(*driver, ry + 36);

        auto statusText = make<Text>("Drag the slider to change progress", 10.0f);
        statusText->setforeground(c(110, 110, 130));
        statusText->setisHitTestVisible(false);
        page->addChild(statusText);
        Canvas::setLeft(*statusText, rx);
        Canvas::setTop(*statusText, ry + 66);

        driver->setOnValueChanged([pb5, statusText](f32 v) {
            pb5->setvalue(v);
            pb5->invalidateRender();
            char buf[64];
            if (v >= 100.0f) {
                std::snprintf(buf, sizeof(buf), "\xe2\x9c\x93 Complete!");
                statusText->setforeground(gut::Color::fromRgba8(100, 220, 100));
            } else {
                std::snprintf(buf, sizeof(buf), "Progress: %.0f%%", v);
                statusText->setforeground(gut::Color::fromRgba8(200, 140, 240));
            }
            statusText->settext(buf);
        });

        // Themed variants — thin bars
        auto themeLabel = make<Text>("Themed variants", 11.0f);
        themeLabel->setforeground(c(160, 160, 180));
        themeLabel->setisHitTestVisible(false);
        page->addChild(themeLabel);
        Canvas::setLeft(*themeLabel, rx);
        Canvas::setTop(*themeLabel, ry + 100);

        // Red / warning
        auto pbRed = make<ProgressBar>(85.0f);
        pbRed->setpreferredWidth(300.0f);
        pbRed->setbarHeight(6.0f);
        pbRed->setcornerRadius(3.0f);
        pbRed->setfillColor(c(220, 70, 70));
        pbRed->setcompletedFillColor(c(220, 70, 70));
        page->addChild(pbRed);
        Canvas::setLeft(*pbRed, rx);
        Canvas::setTop(*pbRed, ry + 120);

        // Orange
        auto pbOrange = make<ProgressBar>(60.0f);
        pbOrange->setpreferredWidth(300.0f);
        pbOrange->setbarHeight(6.0f);
        pbOrange->setcornerRadius(3.0f);
        pbOrange->setfillColor(c(230, 160, 50));
        page->addChild(pbOrange);
        Canvas::setLeft(*pbOrange, rx);
        Canvas::setTop(*pbOrange, ry + 140);

        // Cyan
        auto pbCyan = make<ProgressBar>(45.0f);
        pbCyan->setpreferredWidth(300.0f);
        pbCyan->setbarHeight(6.0f);
        pbCyan->setcornerRadius(3.0f);
        pbCyan->setfillColor(c(60, 200, 220));
        page->addChild(pbCyan);
        Canvas::setLeft(*pbCyan, rx);
        Canvas::setTop(*pbCyan, ry + 160);

        tabControl->addTab("ProgressBar", page);
    }

    _context->setRoot(root);
}

static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
                                     const CVTimeStamp* now,
                                     const CVTimeStamp* outputTime,
                                     CVOptionFlags flagsIn,
                                     CVOptionFlags* flagsOut,
                                     void* ctx)
{
    @autoreleasepool {
        ControlsView* view = (__bridge ControlsView*)ctx;
        dispatch_async(dispatch_get_main_queue(), ^{
            [view render];
        });
    }
    return kCVReturnSuccess;
}

- (void)setupDisplayLink {
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, (__bridge void*)self);
    CVDisplayLinkStart(_displayLink);
}

- (void)render {
    if (!_context) return;

    NSSize size = self.bounds.size;
    CGFloat scale = self.window.backingScaleFactor;

    self.metalLayer.drawableSize = CGSizeMake(size.width * scale, size.height * scale);

    _context->update(16.6f);
    _context->render(size.width, size.height, scale);
}

- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    CGFloat scale = self.window.backingScaleFactor;
    self.metalLayer.drawableSize = CGSizeMake(newSize.width * scale, newSize.height * scale);
}

// =============================================================================
// Mouse input
// =============================================================================

- (void)mouseDown:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Left, true);
}

- (void)mouseUp:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Left, false);
}

- (void)mouseMoved:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
}

- (void)mouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)rightMouseDown:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Right, true);
}

- (void)rightMouseUp:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
    _context->processMouseMove(location.x, location.y);
    _context->processMouseButton(gut::MouseButton::Right, false);
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)scrollWheel:(NSEvent*)event {
    CGFloat dx = event.scrollingDeltaX;
    CGFloat dy = event.scrollingDeltaY;
    if (!event.hasPreciseScrollingDeltas) {
        dx *= 40.0;
        dy *= 40.0;
    }
    _context->processMouseWheel(dx, dy);
}

// =============================================================================
// Keyboard input — the key part for TextBox!
// =============================================================================

- (void)keyDown:(NSEvent*)event {
    // First dispatch the key press (for arrows, backspace, etc.)
    gut::Key key = macKeyCodeToGutKey(event.keyCode);
    gut::ModifierKeys mods = macModifiersToGut(event.modifierFlags);
    _context->processKey(key, true, mods);

    // Then let the input system handle text input via insertText:
    [self interpretKeyEvents:@[event]];
}

- (void)keyUp:(NSEvent*)event {
    gut::Key key = macKeyCodeToGutKey(event.keyCode);
    gut::ModifierKeys mods = macModifiersToGut(event.modifierFlags);
    _context->processKey(key, false, mods);
}

- (void)flagsChanged:(NSEvent*)event {
    // Modifier-only key changes (Shift, Cmd, etc.)
    // Not strictly needed but good for completeness
}

// =============================================================================
// NSTextInputClient — receives composed text from macOS input system
// =============================================================================

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    NSString* str = [string isKindOfClass:[NSAttributedString class]]
        ? [(NSAttributedString*)string string]
        : (NSString*)string;

    for (NSUInteger i = 0; i < str.length; i++) {
        unichar ch = [str characterAtIndex:i];
        _context->processTextInput(static_cast<char32_t>(ch));
    }
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
    // For IME composition — minimal stub
}

- (void)unmarkText {
}

- (NSRange)selectedRange {
    return NSMakeRange(NSNotFound, 0);
}

- (NSRange)markedRange {
    return NSMakeRange(NSNotFound, 0);
}

- (BOOL)hasMarkedText {
    return NO;
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    return nil;
}

- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText {
    return @[];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    return NSZeroRect;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return NSNotFound;
}

- (void)doCommandBySelector:(SEL)selector {
    // Swallow commands we don't handle (e.g., moveDown:, insertNewline:)
    // The key events are already processed via processKey
}

// =============================================================================
// Tracking & first responder
// =============================================================================

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    for (NSTrackingArea* area in self.trackingAreas) {
        [self removeTrackingArea:area];
    }
    NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
        initWithRect:self.bounds
        options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow |
                 NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited)
        owner:self
        userInfo:nil];
    [self addTrackingArea:trackingArea];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end

// =============================================================================
// App Delegate & main
// =============================================================================

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow* window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    NSRect frame = NSMakeRect(0, 0, 800, 750);

    self.window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(NSWindowStyleMaskTitled |
                  NSWindowStyleMaskClosable |
                  NSWindowStyleMaskMiniaturizable |
                  NSWindowStyleMaskResizable)
        backing:NSBackingStoreBuffered
        defer:NO];

    self.window.title = @"Gut Controls Demo";
    [self.window center];

    ControlsView* view = [[ControlsView alloc] initWithFrame:frame];
    self.window.contentView = view;

    [self.window makeKeyAndOrderFront:nil];
    [self.window makeFirstResponder:view];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

@end

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        AppDelegate* delegate = [[AppDelegate alloc] init];
        app.delegate = delegate;

        [app activateIgnoringOtherApps:YES];
        [app run];
    }
    return 0;
}
