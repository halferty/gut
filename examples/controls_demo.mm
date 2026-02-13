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

    const f32 W = 800, H = 600;

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

        auto subtitle = make<Text>("Interactive control showcase  \u2014  TextBox, Button, and more", 11.0f);
        subtitle->setforeground(c(130, 130, 150));
        subtitle->setisHitTestVisible(false);
        root->addChild(subtitle);
        Canvas::setLeft(*subtitle, 30);
        Canvas::setTop(*subtitle, 52);
    }

    // =========================================================================
    // SECTION 1 — Text Input
    // =========================================================================
    {
        f32 sx = 30, sy = 95;

        auto sectionLabel = make<Text>("Text Input", 13.0f);
        sectionLabel->setforeground(c(180, 180, 200));
        sectionLabel->setisHitTestVisible(false);
        root->addChild(sectionLabel);
        Canvas::setLeft(*sectionLabel, sx);
        Canvas::setTop(*sectionLabel, sy);

        // --- Basic text box ---
        auto lbl1 = make<Text>("Name", 11.0f);
        lbl1->setforeground(c(160, 160, 180));
        lbl1->setisHitTestVisible(false);
        root->addChild(lbl1);
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
        root->addChild(tb1);
        Canvas::setLeft(*tb1, sx);
        Canvas::setTop(*tb1, sy + 45);

        // --- Email field ---
        auto lbl2 = make<Text>("Email", 11.0f);
        lbl2->setforeground(c(160, 160, 180));
        lbl2->setisHitTestVisible(false);
        root->addChild(lbl2);
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
        root->addChild(tb2);
        Canvas::setLeft(*tb2, sx);
        Canvas::setTop(*tb2, sy + 99);

        // --- Search field with pre-filled text ---
        auto lbl3 = make<Text>("Search", 11.0f);
        lbl3->setforeground(c(160, 160, 180));
        lbl3->setisHitTestVisible(false);
        root->addChild(lbl3);
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
        root->addChild(tb3);
        Canvas::setLeft(*tb3, sx);
        Canvas::setTop(*tb3, sy + 153);

        // --- Wider text box for long input ---
        auto lbl4 = make<Text>("Bio  (scroll test — type a lot)", 11.0f);
        lbl4->setforeground(c(160, 160, 180));
        lbl4->setisHitTestVisible(false);
        root->addChild(lbl4);
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
        root->addChild(tb4);
        Canvas::setLeft(*tb4, sx);
        Canvas::setTop(*tb4, sy + 207);

        // --- Read-only field ---
        auto lbl5 = make<Text>("Read-only", 11.0f);
        lbl5->setforeground(c(160, 160, 180));
        lbl5->setisHitTestVisible(false);
        root->addChild(lbl5);
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
        root->addChild(tb5);
        Canvas::setLeft(*tb5, sx);
        Canvas::setTop(*tb5, sy + 261);
    }

    // =========================================================================
    // SECTION 2 — Buttons
    // =========================================================================
    {
        f32 sx = 30, sy = 410;

        auto sectionLabel = make<Text>("Buttons", 13.0f);
        sectionLabel->setforeground(c(180, 180, 200));
        sectionLabel->setisHitTestVisible(false);
        root->addChild(sectionLabel);
        Canvas::setLeft(*sectionLabel, sx);
        Canvas::setTop(*sectionLabel, sy);

        // Primary button
        auto btn1 = make<Button>("Submit");
        btn1->setwidth(100);
        btn1->setheight(32);
        btn1->setbackground(c(60, 130, 220));
        btn1->setforeground(c(255, 255, 255));
        btn1->sethoverBackground(c(70, 140, 235));
        btn1->setpressedBackground(c(50, 110, 190));
        btn1->setborderColor(c(80, 150, 240));
        btn1->setcornerRadius(6.0f);
        root->addChild(btn1);
        Canvas::setLeft(*btn1, sx);
        Canvas::setTop(*btn1, sy + 28);

        // Secondary button
        auto btn2 = make<Button>("Cancel");
        btn2->setwidth(100);
        btn2->setheight(32);
        btn2->setbackground(c(55, 55, 65));
        btn2->setforeground(c(200, 200, 220));
        btn2->sethoverBackground(c(65, 65, 78));
        btn2->setpressedBackground(c(45, 45, 55));
        btn2->setborderColor(c(70, 70, 85));
        btn2->setcornerRadius(6.0f);
        root->addChild(btn2);
        Canvas::setLeft(*btn2, sx + 110);
        Canvas::setTop(*btn2, sy + 28);

        // Danger button
        auto btn3 = make<Button>("Delete");
        btn3->setwidth(100);
        btn3->setheight(32);
        btn3->setbackground(c(180, 50, 50));
        btn3->setforeground(c(255, 255, 255));
        btn3->sethoverBackground(c(200, 60, 60));
        btn3->setpressedBackground(c(150, 40, 40));
        btn3->setborderColor(c(200, 70, 70));
        btn3->setcornerRadius(6.0f);
        root->addChild(btn3);
        Canvas::setLeft(*btn3, sx + 220);
        Canvas::setTop(*btn3, sy + 28);

        // Small pill button
        auto btn4 = make<Button>("Pill Style");
        btn4->setwidth(90);
        btn4->setheight(26);
        btn4->setbackground(c(70, 50, 120));
        btn4->setforeground(c(200, 180, 255));
        btn4->sethoverBackground(c(85, 60, 140));
        btn4->setpressedBackground(c(55, 40, 100));
        btn4->setborderColor(c(100, 70, 160));
        btn4->setcornerRadius(13.0f);
        root->addChild(btn4);
        Canvas::setLeft(*btn4, sx);
        Canvas::setTop(*btn4, sy + 70);

        // Disabled button
        auto btn5 = make<Button>("Disabled");
        btn5->setwidth(100);
        btn5->setheight(32);
        btn5->setisEnabled(false);
        btn5->setbackground(c(45, 45, 52));
        btn5->setforeground(c(100, 100, 110));
        btn5->setdisabledBackground(c(45, 45, 52));
        btn5->setborderColor(c(55, 55, 62));
        btn5->setcornerRadius(6.0f);
        root->addChild(btn5);
        Canvas::setLeft(*btn5, sx + 100);
        Canvas::setTop(*btn5, sy + 70);
    }

    // =========================================================================
    // SECTION 3 — Form Example (right side)
    // =========================================================================
    Ref<Text> statusText;
    {
        f32 sx = 420, sy = 95;

        auto sectionLabel = make<Text>("Login Form", 13.0f);
        sectionLabel->setforeground(c(180, 180, 200));
        sectionLabel->setisHitTestVisible(false);
        root->addChild(sectionLabel);
        Canvas::setLeft(*sectionLabel, sx);
        Canvas::setTop(*sectionLabel, sy);

        // Form card background
        auto card = make<Panel>();
        card->setwidth(320);
        card->setheight(220);
        card->setbackground(c(40, 40, 50));
        card->setborderColor(c(60, 60, 72));
        card->setborderWidth(1.0f);
        card->setcornerRadius(10.0f);
        card->setisHitTestVisible(false);  // decorative only — don't block clicks on children
        root->addChild(card);
        Canvas::setLeft(*card, sx);
        Canvas::setTop(*card, sy + 25);

        // Username
        auto uLbl = make<Text>("Username", 11.0f);
        uLbl->setforeground(c(160, 160, 180));
        uLbl->setisHitTestVisible(false);
        root->addChild(uLbl);
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
        root->addChild(userBox);
        Canvas::setLeft(*userBox, sx + 20);
        Canvas::setTop(*userBox, sy + 62);

        // Password
        auto pLbl = make<Text>("Password", 11.0f);
        pLbl->setforeground(c(160, 160, 180));
        pLbl->setisHitTestVisible(false);
        root->addChild(pLbl);
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
        root->addChild(passBox);
        Canvas::setLeft(*passBox, sx + 20);
        Canvas::setTop(*passBox, sy + 117);

        // Login button
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
        root->addChild(loginBtn);
        Canvas::setLeft(*loginBtn, sx + 20);
        Canvas::setTop(*loginBtn, sy + 160);

        // Status text
        auto status = make<Text>("Tab between fields, type to enter text", 10.0f);
        status->setforeground(c(110, 110, 130));
        status->setisHitTestVisible(false);
        statusText = status;
        root->addChild(status);
        Canvas::setLeft(*status, sx + 20);
        Canvas::setTop(*status, sy + 205);

        // Wire up the login button
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

    // =========================================================================
    // SECTION 4 — Live echo (shows text as you type)
    // =========================================================================
    {
        f32 sx = 420, sy = 365;

        auto sectionLabel = make<Text>("Live Text Echo", 13.0f);
        sectionLabel->setforeground(c(180, 180, 200));
        sectionLabel->setisHitTestVisible(false);
        root->addChild(sectionLabel);
        Canvas::setLeft(*sectionLabel, sx);
        Canvas::setTop(*sectionLabel, sy);

        auto echoLabel = make<Text>("Type below and see it echoed in real time:", 11.0f);
        echoLabel->setforeground(c(140, 140, 160));
        echoLabel->setisHitTestVisible(false);
        root->addChild(echoLabel);
        Canvas::setLeft(*echoLabel, sx);
        Canvas::setTop(*echoLabel, sy + 25);

        auto echoText = make<Text>("...", 16.0f);
        echoText->setforeground(c(80, 200, 255));
        echoText->setisHitTestVisible(false);
        root->addChild(echoText);
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
        root->addChild(echoBox);
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

    // =========================================================================
    // SECTION 5 — Keyboard shortcuts hint
    // =========================================================================
    {
        f32 sy = 530;
        const char* hints[] = {
            "\xe2\x86\x90\xe2\x86\x92  Move caret",           // ←→
            "Shift+Arrows  Select text",
            "\xe2\x8c\x98+Arrows  Word jump",                  // ⌘
            "Home/End  Start/end of line",
            "\xe2\x8c\x98+A  Select all",                     // ⌘
            "Double-click  Select word",
            "Tab  Next field",
        };
        f32 x = 30;
        for (auto& hint : hints) {
            auto t = make<Text>(hint, 9.5f);
            t->setforeground(c(100, 100, 120));
            t->setisHitTestVisible(false);
            root->addChild(t);
            Canvas::setLeft(*t, x);
            Canvas::setTop(*t, sy);
            x += 112;
        }
    }

    // =========================================================================
    // Separator lines
    // =========================================================================
    {
        // Horizontal line under title
        auto line = make<Panel>();
        line->setwidth(W - 60);
        line->setheight(1);
        line->setbackground(c(50, 50, 62));
        root->addChild(line);
        Canvas::setLeft(*line, 30);
        Canvas::setTop(*line, 75);

        // Vertical divider between left and right columns
        auto vline = make<Panel>();
        vline->setwidth(1);
        vline->setheight(430);
        vline->setbackground(c(50, 50, 62));
        root->addChild(vline);
        Canvas::setLeft(*vline, 390);
        Canvas::setTop(*vline, 85);

        // Horizontal line above hints
        auto hline = make<Panel>();
        hline->setwidth(W - 60);
        hline->setheight(1);
        hline->setbackground(c(50, 50, 62));
        root->addChild(hline);
        Canvas::setLeft(*hline, 30);
        Canvas::setTop(*hline, 520);
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
    NSRect frame = NSMakeRect(0, 0, 800, 600);

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
