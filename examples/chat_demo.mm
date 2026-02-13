/**
 * @file chat_demo.mm
 * @brief MMORPG-style chat window demo.
 *
 * Features:
 *   - Semi-transparent chat overlay at bottom-left of a dark "game" screen
 *   - Scrolling message log with channel-coloured text lines
 *   - Typeable input with Enter-to-send
 *   - Mouse-wheel scroll, draggable scroll thumb, thin scrollbar
 *   - Capped at 200 messages (oldest removed automatically)
 *   - Simulated other players and system messages for atmosphere
 *   - Fake HUD elements (HP/MP, minimap frame, action bar) for flavour
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CoreVideo.h>

#include "gut_single.h"
#include "metal_backend.mm"

#include <memory>

// =============================================================================
// Constants
// =============================================================================

static constexpr int MAX_MESSAGES = 200;

// Chat channels
static constexpr int CH_GENERAL = 0;
static constexpr int CH_PARTY   = 1;
static constexpr int CH_GUILD   = 2;
static constexpr int CH_SYSTEM  = 3;
static constexpr int CH_WHISPER = 4;
static constexpr int CH_COMBAT  = 5;
static constexpr int CH_EMOTE   = 6;
static constexpr int CH_SAY     = 7;

static gut::Color channelColor(int ch) {
    using namespace gut;
    switch (ch) {
        case CH_GENERAL: return Color::fromRgba8(255, 255, 255);       // white
        case CH_PARTY:   return Color::fromRgba8(105, 185, 255);       // sky blue
        case CH_GUILD:   return Color::fromRgba8(80, 220, 100);        // green
        case CH_SYSTEM:  return Color::fromRgba8(255, 215, 60);        // gold
        case CH_WHISPER: return Color::fromRgba8(210, 130, 210);       // pink
        case CH_COMBAT:  return Color::fromRgba8(255, 90, 90);         // red
        case CH_EMOTE:   return Color::fromRgba8(255, 160, 60);        // orange
        case CH_SAY:     return Color::fromRgba8(210, 210, 220);       // light grey
        default:         return Color::fromRgba8(200, 200, 200);
    }
}

static const char* channelTag(int ch) {
    switch (ch) {
        case CH_GENERAL: return "[General] ";
        case CH_PARTY:   return "[Party] ";
        case CH_GUILD:   return "[Guild] ";
        case CH_SYSTEM:  return "";             // system has no tag
        case CH_WHISPER: return "[Whisper] ";
        case CH_COMBAT:  return "";             // combat log has no tag
        case CH_EMOTE:   return "";
        case CH_SAY:     return "[Say] ";
        default:         return "";
    }
}

// =============================================================================
// Simulated NPC / other-player chat lines
// =============================================================================

struct AmbientLine {
    int channel;
    const char* sender;   // nullptr for system/combat/emote
    const char* text;
};

static const AmbientLine ambientPool[] = {
    {CH_GENERAL, "Thornblade",   "Anyone up for the Shadow Citadel raid tonight?"},
    {CH_GENERAL, "Lyracast",     "Need a healer, we have tank + 2 DPS ready."},
    {CH_GENERAL, "Duskwalker",   "WTS Enchanted Mithril Greatsword, 450g PST"},
    {CH_GENERAL, "Ashenveil",    "Has anyone seen the wandering merchant? Not at his usual spot."},
    {CH_PARTY,   "Ironforge",    "Buffing up, one sec."},
    {CH_PARTY,   "Silkthread",   "I'll CC the left pack, focus right."},
    {CH_GUILD,   "Guildmaster",  "Guild raid signup is open -- check the board!"},
    {CH_GUILD,   "Starweaver",   "Deposited 200 iron ingots in the vault."},
    {CH_SYSTEM,  nullptr,        "Server restart in 45 minutes."},
    {CH_COMBAT,  nullptr,        "Thornblade hits Skeletal Knight for 347 damage."},
    {CH_COMBAT,  nullptr,        "Lyracast casts Healing Light on Thornblade for 520 HP."},
    {CH_EMOTE,   nullptr,        "Duskwalker flexes menacingly."},
    {CH_GENERAL, "Frostpine",    "LFG Moonwell dungeon, ilvl 40+"},
    {CH_WHISPER, "Silkthread",   "Hey, got a spare health potion?"},
    {CH_GENERAL, "Emberclaw",    "Just hit level 60! Finally!"},
    {CH_SAY,     "Thornblade",   "Good fight everyone."},
    {CH_PARTY,   "Ironforge",    "Boss incoming, spread out!"},
    {CH_GUILD,   "Starweaver",   "Anyone have Alchemy 300+? Need some flasks."},
    {CH_GENERAL, "Ashenveil",    "The fishing tournament starts in 10 mins at Mirrorlake."},
    {CH_COMBAT,  nullptr,        "You dodge the Skeletal Knight's attack!"},
    {CH_SYSTEM,  nullptr,        "Double XP weekend begins Friday at 18:00 server time!"},
    {CH_EMOTE,   nullptr,        "Frostpine sits down and starts eating."},
    {CH_GENERAL, "Lyracast",     "Anyone know where to get the Flamecrest recipe?"},
    {CH_PARTY,   "Silkthread",   "Rez pls, I got cleaved."},
};

static constexpr int NUM_AMBIENT = sizeof(ambientPool) / sizeof(ambientPool[0]);

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
    if (flags & NSEventModifierFlagCommand) mods = mods | gut::ModifierKeys::Control;
    if (flags & NSEventModifierFlagOption)  mods = mods | gut::ModifierKeys::Alt;
    return mods;
}

// =============================================================================
// Metal View
// =============================================================================

@interface ChatView : NSView <NSTextInputClient> {
    std::unique_ptr<gut::Context> _context;

    gut::Ref<gut::StackPanel> _messageStack;
    gut::Ref<gut::ScrollViewer> _scrollViewer;
    gut::Ref<gut::TextBox>      _inputBox;

    int _messageCount;
    int _ambientIdx;
}
@property (nonatomic, strong) CAMetalLayer* metalLayer;
@property (nonatomic) CVDisplayLinkRef displayLink;
@end

@implementation ChatView

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        _messageCount = 0;
        _ambientIdx = 0;
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

// -------------------------------------------------------------------------
// Post a chat message (channel-coloured text line)
// -------------------------------------------------------------------------

- (void)postChannel:(int)ch sender:(const char*)sender text:(const gut::String&)text {
    using namespace gut;

    gut::String line;
    line += channelTag(ch);
    if (sender) {
        line += sender;
        line += ": ";
    }
    line += text;

    auto msg = make<Text>(line, 12.0f);
    msg->setforeground(channelColor(ch));
    msg->settextWrapping(Text::TextWrapping::Wrap);
    msg->setisHitTestVisible(false);

    _messageStack->addChild(msg);
    _messageCount++;

    while (_messageCount > MAX_MESSAGES) {
        _messageStack->removeChildAt(0);
        _messageCount--;
    }

    _scrollViewer->scrollToBottom();
}

- (void)sendUserMessage {
    if (!_inputBox) return;
    gut::String text = _inputBox->text();
    if (text.empty()) return;

    _inputBox->settext("");
    _inputBox->setCaretPosition(0);
    _inputBox->clearSelection();

    // User message in General
    [self postChannel:CH_GENERAL sender:"You" text:text];

    // Simulate an ambient reply from another player
    const auto& a = ambientPool[_ambientIdx % NUM_AMBIENT];
    _ambientIdx++;
    [self postChannel:a.channel sender:a.sender text:a.text];
}

// -------------------------------------------------------------------------
// Build the UI
// -------------------------------------------------------------------------

- (void)setupGut {
    auto backend = std::make_unique<gut::MetalRenderBackend>(self.metalLayer);
    _context = std::make_unique<gut::Context>(std::move(backend));
    static_cast<gut::MetalRenderBackend&>(_context->renderBackend()).enableScene3D(false);

    // Load font
    {
        NSString* fontPath = @"/System/Library/Fonts/Supplemental/Arial.ttf";
        NSData* fontData = [NSData dataWithContentsOfFile:fontPath];
        if (fontData)
            _context->loadFont(static_cast<const gut::u8*>(fontData.bytes), fontData.length);
    }

    using namespace gut;
    auto c = [](u8 r, u8 g, u8 b, u8 a = 255) { return Color::fromRgba8(r, g, b, a); };

    const f32 W = 900, H = 620;

    auto root = make<Canvas>();
    root->setwidth(W);
    root->setheight(H);
    root->setbackground(c(10, 14, 18));

    // =================================================================
    // Fake "game world" background
    // =================================================================
    {
        // Ground band
        auto ground = make<Panel>();
        ground->setwidth(W);
        ground->setheight(220);
        ground->setbackground(c(14, 20, 14));
        ground->setisHitTestVisible(false);
        root->addChild(ground);
        Canvas::setLeft(*ground, 0);
        Canvas::setTop(*ground, H - 220);

        // Horizon line
        auto horizon = make<Panel>();
        horizon->setwidth(W);
        horizon->setheight(1);
        horizon->setbackground(c(30, 40, 30));
        horizon->setisHitTestVisible(false);
        root->addChild(horizon);
        Canvas::setLeft(*horizon, 0);
        Canvas::setTop(*horizon, H - 220);

        // Stars
        int starPos[][2] = {
            {120,40},{310,70},{480,25},{650,55},{780,35},
            {200,90},{560,80},{70,110},{400,130},{720,100},
            {850,60},{160,150},{620,140},{340,160},{50,50},
        };
        for (auto& sp : starPos) {
            auto star = make<Panel>();
            star->setwidth(1); star->setheight(1);
            star->setbackground(c(180, 185, 200, (u8)(80 + (sp[0] * 7) % 120)));
            star->setisHitTestVisible(false);
            root->addChild(star);
            Canvas::setLeft(*star, (f32)sp[0]);
            Canvas::setTop(*star, (f32)sp[1]);
        }
    }

    // =================================================================
    // HUD — top-right: HP / MP / Level
    // =================================================================
    {
        f32 hx = W - 200, hy = 14;

        // HP bar bg
        auto hpBg = make<Panel>();
        hpBg->setwidth(170); hpBg->setheight(14);
        hpBg->setbackground(c(40, 15, 15, 200));
        hpBg->setcornerRadius(2); hpBg->setborderWidth(1);
        hpBg->setborderColor(c(80, 30, 30, 200));
        hpBg->setisHitTestVisible(false);
        root->addChild(hpBg);
        Canvas::setLeft(*hpBg, hx); Canvas::setTop(*hpBg, hy);

        // HP fill
        auto hpFill = make<Panel>();
        hpFill->setwidth(170 * 0.89f); hpFill->setheight(14);
        hpFill->setbackground(c(180, 40, 40, 220));
        hpFill->setcornerRadius(2);
        hpFill->setisHitTestVisible(false);
        root->addChild(hpFill);
        Canvas::setLeft(*hpFill, hx); Canvas::setTop(*hpFill, hy);

        auto hpText = make<Text>("2847 / 3200", 9.0f);
        hpText->setforeground(c(255, 220, 220));
        hpText->setisHitTestVisible(false);
        root->addChild(hpText);
        Canvas::setLeft(*hpText, hx + 42); Canvas::setTop(*hpText, hy + 1);

        // MP bar
        f32 my = hy + 18;
        auto mpBg = make<Panel>();
        mpBg->setwidth(170); mpBg->setheight(14);
        mpBg->setbackground(c(15, 15, 45, 200));
        mpBg->setcornerRadius(2); mpBg->setborderWidth(1);
        mpBg->setborderColor(c(30, 30, 90, 200));
        mpBg->setisHitTestVisible(false);
        root->addChild(mpBg);
        Canvas::setLeft(*mpBg, hx); Canvas::setTop(*mpBg, my);

        auto mpFill = make<Panel>();
        mpFill->setwidth(170 * 0.87f); mpFill->setheight(14);
        mpFill->setbackground(c(40, 60, 190, 220));
        mpFill->setcornerRadius(2);
        mpFill->setisHitTestVisible(false);
        root->addChild(mpFill);
        Canvas::setLeft(*mpFill, hx); Canvas::setTop(*mpFill, my);

        auto mpText = make<Text>("1560 / 1800", 9.0f);
        mpText->setforeground(c(210, 210, 255));
        mpText->setisHitTestVisible(false);
        root->addChild(mpText);
        Canvas::setLeft(*mpText, hx + 42); Canvas::setTop(*mpText, my + 1);

        // Level / class label
        auto lvl = make<Text>("Lv.54 Paladin", 10.0f);
        lvl->setforeground(c(200, 200, 210, 200));
        lvl->setisHitTestVisible(false);
        root->addChild(lvl);
        Canvas::setLeft(*lvl, hx); Canvas::setTop(*lvl, my + 18);
    }

    // =================================================================
    // HUD — minimap (top-right circle)
    // =================================================================
    {
        f32 mx = W - 108, my2 = 60;

        auto frame = make<Panel>();
        frame->setwidth(100); frame->setheight(100);
        frame->setbackground(c(18, 22, 18, 200));
        frame->setcornerRadius(50);
        frame->setborderWidth(2);
        frame->setborderColor(c(80, 70, 50, 200));
        frame->setisHitTestVisible(false);
        root->addChild(frame);
        Canvas::setLeft(*frame, mx); Canvas::setTop(*frame, my2);

        auto mLabel = make<Text>("Aethermoor", 8.0f);
        mLabel->setforeground(c(160, 150, 120, 180));
        mLabel->setisHitTestVisible(false);
        root->addChild(mLabel);
        Canvas::setLeft(*mLabel, mx + 22); Canvas::setTop(*mLabel, my2 + 104);

        // Player dot
        auto dot = make<Panel>();
        dot->setwidth(4); dot->setheight(4);
        dot->setbackground(c(100, 255, 100));
        dot->setcornerRadius(2);
        dot->setisHitTestVisible(false);
        root->addChild(dot);
        Canvas::setLeft(*dot, mx + 48); Canvas::setTop(*dot, my2 + 48);
    }

    // =================================================================
    // HUD — action bar (bottom-centre)
    // =================================================================
    {
        const int SLOTS = 10;
        const f32 sz = 36, gap = 3;
        const f32 barW = SLOTS * (sz + gap) - gap;
        const f32 bx = (W - barW) * 0.5f;
        const f32 by = H - 46;

        auto barBg = make<Panel>();
        barBg->setwidth(barW + 12); barBg->setheight(sz + 10);
        barBg->setbackground(c(0, 0, 0, 140));
        barBg->setcornerRadius(4);
        barBg->setborderWidth(1);
        barBg->setborderColor(c(50, 45, 35, 180));
        barBg->setisHitTestVisible(false);
        root->addChild(barBg);
        Canvas::setLeft(*barBg, bx - 6); Canvas::setTop(*barBg, by - 5);

        Color slotC[] = {
            c(180,50,50), c(50,120,200), c(200,160,40), c(50,170,70),
            c(140,60,180), c(50,120,200), c(180,50,50), c(200,160,40),
            c(50,170,70), c(100,100,110),
        };
        const char* keys[] = {"1","2","3","4","5","6","7","8","9","0"};

        for (int i = 0; i < SLOTS; i++) {
            f32 sx = bx + i * (sz + gap);

            auto slot = make<Panel>();
            slot->setwidth(sz); slot->setheight(sz);
            slot->setbackground(c(slotC[i].r, slotC[i].g, slotC[i].b, 120));
            slot->setcornerRadius(3);
            slot->setborderWidth(1);
            slot->setborderColor(c(70, 65, 55, 200));
            slot->setisHitTestVisible(false);
            root->addChild(slot);
            Canvas::setLeft(*slot, sx); Canvas::setTop(*slot, by);

            auto kl = make<Text>(keys[i], 8.0f);
            kl->setforeground(c(200, 190, 170, 160));
            kl->setisHitTestVisible(false);
            root->addChild(kl);
            Canvas::setLeft(*kl, sx + 2); Canvas::setTop(*kl, by + 1);
        }
    }

    // =================================================================
    // Chat window — semi-transparent overlay, bottom-left
    // =================================================================
    const f32 chatW = 480, chatH = 230;
    const f32 chatX = 8, chatY = H - chatH - 52;   // above action bar
    const f32 inputH = 26;
    const f32 tabH = 18;
    const f32 logH = chatH - inputH - tabH - 6;

    // Chat background
    {
        auto bg = make<Panel>();
        bg->setwidth(chatW); bg->setheight(chatH);
        bg->setbackground(c(0, 0, 0, 150));
        bg->setcornerRadius(4);
        bg->setborderWidth(1);
        bg->setborderColor(c(55, 50, 42, 180));
        bg->setisHitTestVisible(false);
        root->addChild(bg);
        Canvas::setLeft(*bg, chatX); Canvas::setTop(*bg, chatY);
    }

    // Channel tabs
    {
        const char* tabs[] = {"General", "Combat", "Party", "System"};
        Color tabCol[] = {
            channelColor(CH_GENERAL), channelColor(CH_COMBAT),
            channelColor(CH_PARTY),   channelColor(CH_SYSTEM),
        };
        f32 tx = chatX + 6;
        for (int i = 0; i < 4; i++) {
            if (i == 0) {
                auto tabBg = make<Panel>();
                tabBg->setwidth(52); tabBg->setheight(tabH);
                tabBg->setbackground(c(40, 40, 50, 180));
                tabBg->setcornerRadius(2);
                tabBg->setisHitTestVisible(false);
                root->addChild(tabBg);
                Canvas::setLeft(*tabBg, tx - 4); Canvas::setTop(*tabBg, chatY + 3);
            }
            auto t = make<Text>(tabs[i], 9.5f);
            t->setforeground(i == 0
                ? tabCol[i]
                : Color::fromRgba8(tabCol[i].r, tabCol[i].g, tabCol[i].b, 140));
            t->setisHitTestVisible(false);
            root->addChild(t);
            Canvas::setLeft(*t, tx); Canvas::setTop(*t, chatY + 5);
            tx += 60;
        }

        // Separator under tabs
        auto sep = make<Panel>();
        sep->setwidth(chatW - 8); sep->setheight(1);
        sep->setbackground(c(60, 55, 45, 120));
        sep->setisHitTestVisible(false);
        root->addChild(sep);
        Canvas::setLeft(*sep, chatX + 4); Canvas::setTop(*sep, chatY + tabH + 3);
    }

    // Chat log — ScrollViewer + StackPanel
    {
        _messageStack = make<StackPanel>(Orientation::Vertical);
        _messageStack->setspacing(1.0f);

        _scrollViewer = make<ScrollViewer>();
        _scrollViewer->setwidth(chatW - 4);
        _scrollViewer->setheight(logH);
        _scrollViewer->setbackground(c(0, 0, 0, 0));  // transparent
        _scrollViewer->setverticalScrollBarVisibility(ScrollBarVisibility::Auto);
        _scrollViewer->sethorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);

        // Thin, unobtrusive scrollbar
        _scrollViewer->setscrollBarWidth(6.0f);
        _scrollViewer->setscrollBarBackground(c(0, 0, 0, 40));
        _scrollViewer->setscrollBarThumb(c(120, 115, 100, 140));
        _scrollViewer->setscrollBarThumbHover(c(160, 155, 140, 180));
        _scrollViewer->setscrollBarThumbDrag(c(200, 195, 180, 220));
        _scrollViewer->setscrollArrowSize(0.0f);  // no arrow buttons, just thumb

        _scrollViewer->addChild(_messageStack);
        root->addChild(_scrollViewer);
        Canvas::setLeft(*_scrollViewer, chatX + 2);
        Canvas::setTop(*_scrollViewer, chatY + tabH + 5);
    }

    // Input line
    {
        _inputBox = make<TextBox>();
        _inputBox->settabIndex(1);
        _inputBox->setwidth(chatW - 8);
        _inputBox->setplaceholder("Press Enter to chat...");
        _inputBox->setbackground(c(10, 10, 14, 200));
        _inputBox->setforeground(c(230, 225, 215));
        _inputBox->setborderColor(c(60, 55, 45, 160));
        _inputBox->setfocusBorderColor(c(100, 90, 60, 200));
        _inputBox->setcaretColor(c(255, 240, 200));
        _inputBox->setselectionColor(c(80, 70, 40, 100));
        _inputBox->setplaceholderColor(c(90, 85, 75));
        _inputBox->setcornerRadius(2.0f);
        _inputBox->setfontSize(12.0f);

        _inputBox->setOnSubmit([self]() {
            [self sendUserMessage];
        });

        root->addChild(_inputBox);
        Canvas::setLeft(*_inputBox, chatX + 4);
        Canvas::setTop(*_inputBox, chatY + chatH - inputH - 3);
    }

    _context->setRoot(root);

    // =================================================================
    // Seed the chat log with atmosphere
    // =================================================================
    [self postChannel:CH_SYSTEM  sender:nullptr        text:"Welcome to the Realm of Aethermoor!"];
    [self postChannel:CH_SYSTEM  sender:nullptr        text:"Type a message and press Enter to speak in General chat."];
    [self postChannel:CH_GENERAL sender:"Thornblade"   text:"Anyone up for the Shadow Citadel raid tonight?"];
    [self postChannel:CH_GENERAL sender:"Lyracast"     text:"Need a healer, we have tank + 2 DPS ready."];
    [self postChannel:CH_PARTY   sender:"Ironforge"    text:"Buffing up, one sec."];
    [self postChannel:CH_GUILD   sender:"Guildmaster"  text:"Guild raid signup is open -- check the board!"];
    [self postChannel:CH_COMBAT  sender:nullptr        text:"Thornblade hits Skeletal Knight for 347 damage."];
    [self postChannel:CH_GENERAL sender:"Duskwalker"   text:"WTS Enchanted Mithril Greatsword, 450g PST"];
    [self postChannel:CH_SYSTEM  sender:nullptr        text:"Server restart in 45 minutes."];
    [self postChannel:CH_GENERAL sender:"Ashenveil"    text:"Has anyone seen the wandering merchant? Not at his usual spot."];
    [self postChannel:CH_EMOTE   sender:nullptr        text:"Frostpine sits down and starts eating."];
    [self postChannel:CH_WHISPER sender:"Silkthread"   text:"Hey, got a spare health potion?"];
}

// =========================================================================
// Display link
// =========================================================================

static CVReturn DisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*,
                                     const CVTimeStamp*, CVOptionFlags,
                                     CVOptionFlags*, void* ctx)
{
    @autoreleasepool {
        dispatch_async(dispatch_get_main_queue(), ^{
            [(__bridge ChatView*)ctx render];
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

// =========================================================================
// Mouse
// =========================================================================

- (void)mouseDown:(NSEvent*)e {
    NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    p.y = self.bounds.size.height - p.y;
    _context->processMouseMove(p.x, p.y);
    _context->processMouseButton(gut::MouseButton::Left, true);
}

- (void)mouseUp:(NSEvent*)e {
    NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    p.y = self.bounds.size.height - p.y;
    _context->processMouseMove(p.x, p.y);
    _context->processMouseButton(gut::MouseButton::Left, false);
}

- (void)mouseMoved:(NSEvent*)e {
    NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    p.y = self.bounds.size.height - p.y;
    _context->processMouseMove(p.x, p.y);
}

- (void)mouseDragged:(NSEvent*)e { [self mouseMoved:e]; }

- (void)rightMouseDown:(NSEvent*)e {
    NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    p.y = self.bounds.size.height - p.y;
    _context->processMouseMove(p.x, p.y);
    _context->processMouseButton(gut::MouseButton::Right, true);
}

- (void)rightMouseUp:(NSEvent*)e {
    NSPoint p = [self convertPoint:e.locationInWindow fromView:nil];
    p.y = self.bounds.size.height - p.y;
    _context->processMouseMove(p.x, p.y);
    _context->processMouseButton(gut::MouseButton::Right, false);
}

- (void)rightMouseDragged:(NSEvent*)e { [self mouseMoved:e]; }

- (void)scrollWheel:(NSEvent*)e {
    CGFloat dx = e.scrollingDeltaX;
    CGFloat dy = e.scrollingDeltaY;
    if (!e.hasPreciseScrollingDeltas) {
        // Mouse wheel: convert line units to pixels
        dx *= 40.0;
        dy *= 40.0;
    }
    _context->processMouseWheel(dx, dy);
}

// =========================================================================
// Keyboard
// =========================================================================

- (void)keyDown:(NSEvent*)e {
    gut::Key key = macKeyCodeToGutKey(e.keyCode);
    gut::ModifierKeys mods = macModifiersToGut(e.modifierFlags);
    _context->processKey(key, true, mods);
    [self interpretKeyEvents:@[e]];
}

- (void)keyUp:(NSEvent*)e {
    gut::Key key = macKeyCodeToGutKey(e.keyCode);
    gut::ModifierKeys mods = macModifiersToGut(e.modifierFlags);
    _context->processKey(key, false, mods);
}

- (void)flagsChanged:(NSEvent*)e {}

// =========================================================================
// NSTextInputClient
// =========================================================================

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    NSString* str = [string isKindOfClass:[NSAttributedString class]]
        ? [(NSAttributedString*)string string]
        : (NSString*)string;
    for (NSUInteger i = 0; i < str.length; i++) {
        _context->processTextInput(static_cast<char32_t>([str characterAtIndex:i]));
    }
}

- (void)setMarkedText:(id)s selectedRange:(NSRange)sr replacementRange:(NSRange)rr {}
- (void)unmarkText {}
- (NSRange)selectedRange { return NSMakeRange(NSNotFound, 0); }
- (NSRange)markedRange { return NSMakeRange(NSNotFound, 0); }
- (BOOL)hasMarkedText { return NO; }
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)r actualRange:(NSRangePointer)a { return nil; }
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText { return @[]; }
- (NSRect)firstRectForCharacterRange:(NSRange)r actualRange:(NSRangePointer)a { return NSZeroRect; }
- (NSUInteger)characterIndexForPoint:(NSPoint)p { return NSNotFound; }
- (void)doCommandBySelector:(SEL)sel {}

// =========================================================================
// Tracking & first responder
// =========================================================================

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    for (NSTrackingArea* a in self.trackingAreas) [self removeTrackingArea:a];
    [self addTrackingArea:[[NSTrackingArea alloc]
        initWithRect:self.bounds
        options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow |
                 NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited)
        owner:self userInfo:nil]];
}

- (BOOL)acceptsFirstResponder { return YES; }

@end

// =========================================================================
// App Delegate & main
// =========================================================================

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow* window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)n {
    NSRect frame = NSMakeRect(0, 0, 900, 620);
    self.window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                  NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
        backing:NSBackingStoreBuffered defer:NO];
    self.window.title = @"Realm of Aethermoor";
    self.window.backgroundColor = [NSColor blackColor];
    [self.window center];

    ChatView* view = [[ChatView alloc] initWithFrame:frame];
    self.window.contentView = view;
    [self.window makeKeyAndOrderFront:nil];
    [self.window makeFirstResponder:view];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)s { return YES; }

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
