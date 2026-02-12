/**
 * @file metal_demo.mm
 * @brief Simple macOS app demonstrating Gut with Metal backend
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CoreVideo.h>

#include "gut_single.h"

// Include the Metal backend implementation
#include "metal_backend.mm"

#include <memory>

@interface MetalView : NSView {
    std::unique_ptr<gut::Context> _context;
    std::vector<gut::Ref<gut::FloatAnimation>> _animations;
}
@property (nonatomic, strong) CAMetalLayer* metalLayer;
@property (nonatomic) CVDisplayLinkRef displayLink;
@end

@implementation MetalView

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
    
    using namespace gut;
    auto& rb = _context->renderBackend();
    auto c = [](u8 r, u8 g, u8 b, u8 a = 255) { return Color::fromRgba8(r, g, b, a); };
    
    // =====================================================================
    // Procedural pixel-art texture generator
    // =====================================================================
    auto makeTex = [&](u32 w, u32 h, std::function<void(u8* px, u32 w, u32 h)> gen) -> Ref<Texture> {
        std::vector<u8> px(w * h * 4, 0);
        gen(px.data(), w, h);
        return rb.createTexture(w, h, px.data());
    };
    
    // Pixel setter helper
    auto putPx = [](u8* px, u32 stride, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a = 255) {
        u32 i = (y * stride + x) * 4;
        px[i] = r; px[i+1] = g; px[i+2] = b; px[i+3] = a;
    };
    
    // Fill rect helper
    auto fillRect = [](u8* px, u32 stride, u32 x0, u32 y0, u32 w, u32 h, u8 r, u8 g, u8 b, u8 a = 255) {
        for (u32 y = y0; y < y0 + h; y++)
            for (u32 x = x0; x < x0 + w; x++) {
                u32 i = (y * stride + x) * 4;
                px[i] = r; px[i+1] = g; px[i+2] = b; px[i+3] = a;
            }
    };
    
    // --- Player portrait: helmeted warrior face (32x32) ---
    auto portraitTex = makeTex(32, 32, [&](u8* px, u32 w, u32 h) {
        // Background - dark skin tone
        fillRect(px, w, 0, 0, 32, 32, 40, 30, 25);
        // Face oval
        fillRect(px, w, 8, 10, 16, 18, 210, 170, 130);
        // Eyes
        fillRect(px, w, 11, 16, 3, 3, 255, 255, 255);
        fillRect(px, w, 18, 16, 3, 3, 255, 255, 255);
        putPx(px, w, 12, 17, 40, 80, 120);  // left pupil
        putPx(px, w, 19, 17, 40, 80, 120);  // right pupil
        // Mouth
        fillRect(px, w, 13, 23, 6, 2, 180, 100, 90);
        // Helmet (gold/steel)
        fillRect(px, w, 6, 4, 20, 8, 160, 140, 60);
        fillRect(px, w, 8, 2, 16, 4, 180, 160, 80);
        // Helmet nose guard
        fillRect(px, w, 14, 8, 4, 8, 140, 130, 60);
        // Helmet plume (red)
        fillRect(px, w, 12, 0, 8, 4, 180, 30, 30);
        fillRect(px, w, 14, 0, 4, 2, 220, 50, 40);
        // Ears
        fillRect(px, w, 6, 14, 3, 6, 195, 155, 120);
        fillRect(px, w, 23, 14, 3, 6, 195, 155, 120);
    });
    
    // --- Boss portrait: demon face (32x32) ---
    auto bossTex = makeTex(32, 32, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 32, 32, 30, 10, 15);
        // Face - purple/red demon
        fillRect(px, w, 6, 8, 20, 20, 140, 50, 60);
        // Horns
        fillRect(px, w, 4, 2, 4, 10, 80, 30, 30);
        fillRect(px, w, 24, 2, 4, 10, 80, 30, 30);
        fillRect(px, w, 2, 0, 4, 4, 100, 40, 35);
        fillRect(px, w, 26, 0, 4, 4, 100, 40, 35);
        // Glowing eyes
        fillRect(px, w, 10, 14, 4, 3, 255, 200, 0);
        fillRect(px, w, 18, 14, 4, 3, 255, 200, 0);
        putPx(px, w, 11, 15, 255, 100, 0);
        putPx(px, w, 19, 15, 255, 100, 0);
        // Fangs
        putPx(px, w, 12, 22, 255, 255, 240);
        putPx(px, w, 13, 23, 255, 255, 240);
        putPx(px, w, 19, 22, 255, 255, 240);
        putPx(px, w, 18, 23, 255, 255, 240);
        // Mouth
        fillRect(px, w, 11, 21, 10, 3, 100, 20, 30);
        // Jaw ridges
        fillRect(px, w, 6, 24, 20, 2, 120, 40, 50);
    });
    
    // --- Minimap texture (128x128) - top-down terrain ---
    auto minimapTex = makeTex(128, 128, [&](u8* px, u32 w, u32 h) {
        // Base grass
        for (u32 y = 0; y < h; y++)
            for (u32 x = 0; x < w; x++) {
                u8 noise = ((x * 7 + y * 13) ^ (x * y)) & 0x1F;
                fillRect(px, w, x, y, 1, 1, 30 + noise, 55 + noise, 25 + noise/2);
            }
        // Dirt path (diagonal)
        for (u32 i = 0; i < 128; i++) {
            u32 px_ = 20 + i * 88 / 128;
            u32 py_ = i;
            for (int d = -2; d <= 2; d++)
                if (px_ + d < w)
                    fillRect(px, w, px_ + d, py_, 1, 1, 110, 90, 55);
        }
        // Cross path (horizontal)
        for (u32 x = 0; x < 128; x++) {
            u32 py_ = 60 + (x % 7 > 3 ? 1 : 0);
            for (int d = -1; d <= 1; d++)
                if (py_ + d < h)
                    fillRect(px, w, x, py_ + d, 1, 1, 100, 85, 50);
        }
        // Lake
        for (u32 y = 75; y < 105; y++)
            for (u32 x = 30; x < 70; x++) {
                f32 dx = (f32)x - 50, dy = (f32)y - 90;
                if (dx*dx/400 + dy*dy/225 < 1.0f) {
                    u8 wave = ((x + y) & 3) * 5;
                    fillRect(px, w, x, y, 1, 1, 20 + wave, 40 + wave, 100 + wave);
                }
            }
        // Forest (dark green blobs)
        auto tree = [&](u32 tx, u32 ty) {
            for (int dy = -3; dy <= 3; dy++)
                for (int dx = -3; dx <= 3; dx++)
                    if (dx*dx + dy*dy <= 9 && tx+dx < w && ty+dy < h)
                        fillRect(px, w, tx+dx, ty+dy, 1, 1, 15, 40, 15);
        };
        tree(90, 25); tree(95, 30); tree(100, 20); tree(85, 35);
        tree(15, 45); tree(20, 50); tree(10, 55);
        // Buildings (stone grey)
        fillRect(px, w, 55, 45, 12, 10, 120, 115, 100);
        fillRect(px, w, 72, 50, 8, 8, 110, 105, 90);
        fillRect(px, w, 48, 55, 6, 6, 100, 95, 85);
        // Player arrow (bright yellow dot with direction)
        fillRect(px, w, 62, 62, 4, 4, 255, 255, 60);
        fillRect(px, w, 63, 60, 2, 2, 255, 255, 60); // arrow tip
    });
    
    // --- Ability icons (24x24 each) ---
    // Fireball
    auto iconFireball = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 60, 20, 10);
        for (u32 y = 0; y < 24; y++)
            for (u32 x = 0; x < 24; x++) {
                f32 dx = (f32)x - 12, dy = (f32)y - 13;
                f32 dist = dx*dx + dy*dy;
                if (dist < 36) fillRect(px, w, x, y, 1, 1, 255, 240, 200);
                else if (dist < 64) fillRect(px, w, x, y, 1, 1, 255, 160, 30);
                else if (dist < 100) fillRect(px, w, x, y, 1, 1, 200, 60, 10);
            }
    });
    
    // Frostbolt
    auto iconFrost = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 10, 20, 50);
        for (u32 y = 0; y < 24; y++)
            for (u32 x = 0; x < 24; x++) {
                f32 dx = (f32)x - 12, dy = (f32)y - 12;
                f32 dist = dx*dx + dy*dy;
                if (dist < 30) fillRect(px, w, x, y, 1, 1, 220, 240, 255);
                else if (dist < 60) fillRect(px, w, x, y, 1, 1, 100, 180, 255);
                else if (dist < 90) fillRect(px, w, x, y, 1, 1, 40, 100, 200);
            }
        // Crystal shards
        fillRect(px, w, 10, 4, 4, 8, 200, 230, 255);
        fillRect(px, w, 6, 8, 3, 6, 180, 220, 255);
        fillRect(px, w, 16, 7, 3, 7, 180, 220, 255);
    });
    
    // Lightning bolt
    auto iconLightning = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 15, 10, 40);
        // Bolt shape
        fillRect(px, w, 14, 2, 4, 4, 255, 255, 100);
        fillRect(px, w, 12, 5, 4, 3, 255, 255, 100);
        fillRect(px, w, 10, 7, 6, 3, 255, 255, 100);
        fillRect(px, w, 12, 10, 4, 3, 255, 240, 80);
        fillRect(px, w, 10, 12, 4, 3, 255, 240, 80);
        fillRect(px, w, 8, 14, 6, 3, 255, 220, 60);
        fillRect(px, w, 10, 17, 4, 3, 255, 200, 40);
        fillRect(px, w, 8, 19, 4, 4, 255, 180, 30);
        // Glow
        for (u32 y = 0; y < 24; y++)
            for (u32 x = 0; x < 24; x++) {
                u32 i = (y * w + x) * 4;
                if (px[i+3] > 0 && px[i] < 100)
                    px[i+2] = std::min(255, (int)px[i+2] + 30);
            }
    });
    
    // Heal (green cross)
    auto iconHeal = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 15, 35, 15);
        fillRect(px, w, 9, 4, 6, 16, 80, 220, 80);
        fillRect(px, w, 4, 9, 16, 6, 80, 220, 80);
        // Bright center
        fillRect(px, w, 10, 10, 4, 4, 180, 255, 180);
        // Sparkles
        putPx(px, w, 5, 5, 200, 255, 200);
        putPx(px, w, 18, 5, 200, 255, 200);
        putPx(px, w, 5, 18, 200, 255, 200);
        putPx(px, w, 18, 18, 200, 255, 200);
    });
    
    // Shield (defense)
    auto iconShield = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 20, 20, 35);
        // Shield body
        for (u32 y = 3; y < 21; y++)
            for (u32 x = 4; x < 20; x++) {
                f32 narrow = 1.0f - (f32)(y - 3) / 18.0f * 0.6f;
                f32 cx = 12.0f;
                if (std::abs((f32)x - cx) < 8.0f * narrow)
                    fillRect(px, w, x, y, 1, 1, 140, 150, 180);
            }
        // Shield cross emblem
        fillRect(px, w, 11, 6, 2, 12, 200, 180, 60);
        fillRect(px, w, 7, 10, 10, 2, 200, 180, 60);
        // Edge highlight
        for (u32 y = 3; y < 19; y++) {
            putPx(px, w, 5, y, 180, 190, 210);
            putPx(px, w, 18, y, 100, 110, 140);
        }
    });
    
    // Sword (attack)
    auto iconSword = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 30, 15, 10);
        // Blade
        for (int i = 0; i < 14; i++) {
            fillRect(px, w, 16 - i, 3 + i, 3, 2, 200, 210, 220);
        }
        // Hilt
        fillRect(px, w, 5, 16, 8, 2, 160, 130, 50);
        // Grip
        fillRect(px, w, 3, 18, 3, 4, 80, 50, 30);
        fillRect(px, w, 4, 19, 2, 3, 100, 70, 40);
        // Blade gleam
        for (int i = 0; i < 10; i++)
            putPx(px, w, 17 - i, 4 + i, 240, 245, 255);
    });
    
    // Potion (red)
    auto iconPotion = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 25, 15, 15);
        // Bottle body
        fillRect(px, w, 7, 10, 10, 11, 160, 30, 30);
        // Bottle neck
        fillRect(px, w, 10, 5, 4, 6, 180, 40, 40);
        // Cork
        fillRect(px, w, 10, 3, 4, 3, 140, 110, 60);
        // Liquid highlight
        fillRect(px, w, 8, 12, 3, 6, 220, 60, 60);
        // Bottle shine
        putPx(px, w, 14, 11, 255, 200, 200);
        putPx(px, w, 14, 12, 255, 180, 180);
        // Label
        fillRect(px, w, 8, 16, 8, 3, 200, 190, 150);
    });
    
    // Skull (debuff)
    auto iconSkull = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 30, 15, 35);
        // Cranium
        for (u32 y = 4; y < 16; y++)
            for (u32 x = 6; x < 18; x++) {
                f32 dx = (f32)x - 12, dy = (f32)y - 10;
                if (dx*dx/36 + dy*dy/36 < 1.0f)
                    fillRect(px, w, x, y, 1, 1, 220, 210, 190);
            }
        // Eye sockets
        fillRect(px, w, 8, 9, 3, 3, 20, 10, 25);
        fillRect(px, w, 13, 9, 3, 3, 20, 10, 25);
        // Nose
        putPx(px, w, 11, 13, 40, 30, 35);
        putPx(px, w, 12, 13, 40, 30, 35);
        // Teeth
        for (u32 x = 9; x < 15; x++)
            fillRect(px, w, x, 15, 1, 3, (x & 1) ? 200 : 30, (x & 1) ? 195 : 15, (x & 1) ? 180 : 25);
    });
    
    // Treasure bag
    auto iconBag = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 25, 20, 10);
        // Bag body
        fillRect(px, w, 5, 8, 14, 13, 140, 100, 50);
        fillRect(px, w, 6, 9, 12, 11, 160, 120, 60);
        // Bag opening
        fillRect(px, w, 7, 6, 10, 4, 120, 85, 40);
        // Drawstring
        fillRect(px, w, 8, 5, 8, 2, 100, 70, 30);
        // Gold coins peeking out
        fillRect(px, w, 9, 7, 3, 2, 255, 220, 50);
        fillRect(px, w, 13, 7, 2, 2, 255, 210, 40);
        // Bag buckle
        fillRect(px, w, 10, 14, 4, 3, 200, 180, 60);
        fillRect(px, w, 11, 15, 2, 1, 160, 140, 40);
    });
    
    // Buff icons (16x16 each)
    auto makeBuffIcon = [&](u8 r, u8 g, u8 b, auto shape) {
        return makeTex(16, 16, [&, r, g, b, shape](u8* px, u32 w, u32 h) {
            // Border
            for (u32 y = 0; y < 16; y++)
                for (u32 x = 0; x < 16; x++) {
                    if (x == 0 || x == 15 || y == 0 || y == 15)
                        fillRect(px, w, x, y, 1, 1, r/2, g/2, b/2);
                    else
                        fillRect(px, w, x, y, 1, 1, r/4, g/4, b/4);
                }
            shape(px, w);
        });
    };
    
    auto buffSword = makeBuffIcon(200, 100, 50, [&](u8* px, u32 w) {
        for (int i = 0; i < 10; i++) fillRect(px, w, 11-i, 3+i, 2, 1, 220, 210, 200);
        fillRect(px, w, 3, 11, 6, 1, 200, 170, 50);
    });
    auto buffShield = makeBuffIcon(50, 100, 200, [&](u8* px, u32 w) {
        fillRect(px, w, 5, 3, 6, 9, 100, 160, 230);
        fillRect(px, w, 6, 4, 4, 7, 120, 180, 240);
    });
    auto buffHaste = makeBuffIcon(200, 200, 50, [&](u8* px, u32 w) {
        fillRect(px, w, 8, 3, 2, 10, 255, 255, 100);
        fillRect(px, w, 5, 6, 2, 6, 200, 200, 80);
        fillRect(px, w, 11, 5, 2, 7, 200, 200, 80);
    });
    auto buffRegen = makeBuffIcon(50, 200, 80, [&](u8* px, u32 w) {
        fillRect(px, w, 6, 4, 4, 8, 80, 230, 100);
        fillRect(px, w, 4, 6, 8, 4, 80, 230, 100);
    });
    auto buffMana = makeBuffIcon(80, 80, 220, [&](u8* px, u32 w) {
        for (u32 y = 4; y < 12; y++)
            for (u32 x = 4; x < 12; x++) {
                f32 d = std::abs((f32)x-8) + std::abs((f32)y-8);
                if (d < 5) fillRect(px, w, x, y, 1, 1, 120, 150, 255);
            }
    });
    auto buffFire = makeBuffIcon(220, 80, 30, [&](u8* px, u32 w) {
        fillRect(px, w, 6, 8, 4, 5, 255, 160, 30);
        fillRect(px, w, 7, 5, 2, 4, 255, 200, 60);
        putPx(px, w, 7, 4, 255, 240, 100);
    });
    auto buffShadow = makeBuffIcon(120, 40, 160, [&](u8* px, u32 w) {
        for (u32 y = 3; y < 13; y++)
            for (u32 x = 3; x < 13; x++) {
                f32 d = ((f32)x-8)*((f32)x-8) + ((f32)y-8)*((f32)y-8);
                if (d < 20) fillRect(px, w, x, y, 1, 1, 160, 60, 200);
            }
    });
    auto buffArmor = makeBuffIcon(160, 160, 180, [&](u8* px, u32 w) {
        fillRect(px, w, 5, 4, 6, 8, 190, 190, 210);
        fillRect(px, w, 4, 5, 8, 6, 170, 170, 195);
    });
    
    Ref<Texture> buffTextures[] = {buffSword, buffShield, buffHaste, buffRegen, buffMana, buffFire, buffShadow, buffArmor};
    
    // Store ability icons in an array for the action bar
    Ref<Texture> abilityIcons[] = {
        iconFireball, iconFrost, iconLightning, iconHeal, iconShield,
        iconSword, iconPotion, iconSkull, iconFireball, iconFrost,
        iconLightning, iconHeal
    };
    Ref<Texture> bar2Icons[] = {
        iconSword, iconHeal, iconShield, iconPotion, iconSkull, iconBag,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
    };
    
    // =====================================================================
    // MMORPG HUD — 1024x768 game window
    // =====================================================================
    const f32 W = 1024, H = 768;
    
    auto root = make<Canvas>();
    root->setwidth(W);
    root->setheight(H);
    root->setbackground(c(15, 15, 25));  // Dark game world background
    
    // Helper: dark panel with border feel
    auto frame = [&](f32 w, f32 h, Color bg = Color::fromRgba8(20, 20, 30, 220)) {
        auto p = make<Panel>();
        p->setwidth(w);
        p->setheight(h);
        p->setbackground(bg);
        return p;
    };
    
    // Helper: colored bar (health, mana, xp, etc.)
    auto bar = [&](f32 w, f32 h, f32 fill, Color fillColor, Color bgColor) {
        auto container = make<Canvas>();
        container->setwidth(w);
        container->setheight(h);
        
        auto bg = make<Panel>();
        bg->setwidth(w);
        bg->setheight(h);
        bg->setbackground(bgColor);
        container->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        auto fg = make<Panel>();
        fg->setwidth(w * fill);
        fg->setheight(h);
        fg->setbackground(fillColor);
        container->addChild(fg);
        Canvas::setLeft(*fg, 0);
        Canvas::setTop(*fg, 0);
        
        return container;
    };
    
    // Helper: create an Image element from a texture
    auto img = [&](Ref<Texture> tex, f32 w, f32 h) {
        auto i = make<Image>(tex);
        i->setwidth(w);
        i->setheight(h);
        i->setstretch(Image::Stretch::Fill);
        return i;
    };
    
    // =====================================================================
    // PLAYER FRAME (top-left)
    // =====================================================================
    {
        auto playerFrame = make<Canvas>();
        playerFrame->setwidth(260);
        playerFrame->setheight(70);
        root->addChild(playerFrame);
        Canvas::setLeft(*playerFrame, 10);
        Canvas::setTop(*playerFrame, 10);
        
        // Frame background
        auto bg = frame(260, 70, c(25, 22, 18, 230));
        playerFrame->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        // Portrait box
        auto portrait = frame(50, 50, c(80, 60, 45));
        playerFrame->addChild(portrait);
        Canvas::setLeft(*portrait, 8);
        Canvas::setTop(*portrait, 10);
        
        // Portrait image
        auto face = img(portraitTex, 46, 46);
        playerFrame->addChild(face);
        Canvas::setLeft(*face, 10);
        Canvas::setTop(*face, 12);
        
        // Player name label bg
        auto nameBg = frame(185, 16, c(35, 30, 25, 200));
        playerFrame->addChild(nameBg);
        Canvas::setLeft(*nameBg, 65);
        Canvas::setTop(*nameBg, 6);
        
        // Name text
        auto nameText = make<Text>("Aelindra", 11.0f);
        nameText->setforeground(c(255, 210, 100));
        playerFrame->addChild(nameText);
        Canvas::setLeft(*nameText, 70);
        Canvas::setTop(*nameText, 7);
        
        // Level badge
        auto lvlBg = frame(22, 22, c(60, 50, 20));
        playerFrame->addChild(lvlBg);
        Canvas::setLeft(*lvlBg, 232);
        Canvas::setTop(*lvlBg, 5);
        
        auto lvlText = make<Text>("60", 10.0f);
        lvlText->setforeground(c(255, 220, 80));
        playerFrame->addChild(lvlText);
        Canvas::setLeft(*lvlText, 236);
        Canvas::setTop(*lvlText, 8);
        
        // Health bar
        auto hp = bar(185, 14, 0.78f, c(180, 30, 30), c(60, 15, 15));
        playerFrame->addChild(hp);
        Canvas::setLeft(*hp, 65);
        Canvas::setTop(*hp, 25);
        
        // HP text overlay
        auto hpText = make<Text>("15,432 / 19,780", 9.0f);
        hpText->setforeground(c(255, 255, 255));
        playerFrame->addChild(hpText);
        Canvas::setLeft(*hpText, 100);
        Canvas::setTop(*hpText, 26);
        
        // Mana bar
        auto mp = bar(185, 14, 0.45f, c(30, 60, 180), c(15, 20, 60));
        playerFrame->addChild(mp);
        Canvas::setLeft(*mp, 65);
        Canvas::setTop(*mp, 42);
        
        // MP text overlay
        auto mpText = make<Text>("4,210 / 9,350", 9.0f);
        mpText->setforeground(c(255, 255, 255));
        playerFrame->addChild(mpText);
        Canvas::setLeft(*mpText, 105);
        Canvas::setTop(*mpText, 43);
        
        // Frame border (gold trim)
        auto borderTop = frame(260, 2, c(120, 100, 50));
        playerFrame->addChild(borderTop);
        Canvas::setLeft(*borderTop, 0);
        Canvas::setTop(*borderTop, 0);
        
        auto borderBot = frame(260, 2, c(80, 65, 30));
        playerFrame->addChild(borderBot);
        Canvas::setLeft(*borderBot, 0);
        Canvas::setTop(*borderBot, 68);
        
        auto borderL = frame(2, 70, c(100, 85, 40));
        playerFrame->addChild(borderL);
        Canvas::setLeft(*borderL, 0);
        Canvas::setTop(*borderL, 0);
        
        auto borderR = frame(2, 70, c(100, 85, 40));
        playerFrame->addChild(borderR);
        Canvas::setLeft(*borderR, 258);
        Canvas::setTop(*borderR, 0);
    }
    
    // =====================================================================
    // TARGET FRAME (top-left, below player)
    // =====================================================================
    {
        auto targetFrame = make<Canvas>();
        targetFrame->setwidth(260);
        targetFrame->setheight(60);
        root->addChild(targetFrame);
        Canvas::setLeft(*targetFrame, 10);
        Canvas::setTop(*targetFrame, 85);
        
        auto bg = frame(260, 60, c(30, 18, 18, 230));
        targetFrame->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        // Target portrait
        auto portrait = frame(42, 42, c(90, 30, 30));
        targetFrame->addChild(portrait);
        Canvas::setLeft(*portrait, 8);
        Canvas::setTop(*portrait, 9);
        
        auto face = img(bossTex, 38, 38);
        targetFrame->addChild(face);
        Canvas::setLeft(*face, 10);
        Canvas::setTop(*face, 11);
        
        // Target name
        auto nameText = make<Text>("Dreadlord Malachar", 10.0f);
        nameText->setforeground(c(255, 100, 100));
        targetFrame->addChild(nameText);
        Canvas::setLeft(*nameText, 58);
        Canvas::setTop(*nameText, 6);
        
        // Skull icon (boss marker — just a colored square)
        auto skull = frame(12, 12, c(255, 200, 0));
        targetFrame->addChild(skull);
        Canvas::setLeft(*skull, 240);
        Canvas::setTop(*skull, 5);
        
        // Target health
        auto hp = bar(192, 12, 0.62f, c(200, 40, 40), c(60, 15, 15));
        targetFrame->addChild(hp);
        Canvas::setLeft(*hp, 58);
        Canvas::setTop(*hp, 22);
        
        auto hpText = make<Text>("62%", 8.0f);
        hpText->setforeground(c(255, 255, 255));
        targetFrame->addChild(hpText);
        Canvas::setLeft(*hpText, 140);
        Canvas::setTop(*hpText, 23);
        
        // Target mana/power
        auto mp = bar(192, 10, 0.85f, c(160, 80, 200), c(50, 20, 60));
        targetFrame->addChild(mp);
        Canvas::setLeft(*mp, 58);
        Canvas::setTop(*mp, 37);
        
        // Border
        auto bTop = frame(260, 2, c(120, 40, 40));
        targetFrame->addChild(bTop);
        Canvas::setLeft(*bTop, 0);
        Canvas::setTop(*bTop, 0);
        
        auto bBot = frame(260, 1, c(80, 30, 30));
        targetFrame->addChild(bBot);
        Canvas::setLeft(*bBot, 0);
        Canvas::setTop(*bBot, 59);
    }
    
    // =====================================================================
    // PARTY FRAMES (left side)
    // =====================================================================
    Ref<Canvas> brokkFrame;  // promoted for animation (low HP pulse)
    {
        f32 partyY = 160;
        struct PartyMember { const char* name; f32 hp; f32 mp; Color nameColor; };
        PartyMember members[] = {
            {"Thornguard",    0.95f, 0.30f, c(255, 255, 255)},
            {"Sylvaris",      0.60f, 0.72f, c(150, 220, 255)},
            {"Brokk",         0.35f, 0.55f, c(255, 200, 100)},
            {"Luminia",       0.88f, 0.15f, c(200, 255, 200)},
        };
        
        int idx = 0;
        for (auto& m : members) {
            auto pf = make<Canvas>();
            pf->setwidth(140);
            pf->setheight(36);
            root->addChild(pf);
            Canvas::setLeft(*pf, 10);
            Canvas::setTop(*pf, partyY);
            
            if (idx == 2) brokkFrame = pf;  // Brokk
            
            auto bg = frame(140, 36, c(20, 20, 30, 200));
            pf->addChild(bg);
            Canvas::setLeft(*bg, 0);
            Canvas::setTop(*bg, 0);
            
            auto nameT = make<Text>(m.name, 9.0f);
            nameT->setforeground(m.nameColor);
            pf->addChild(nameT);
            Canvas::setLeft(*nameT, 5);
            Canvas::setTop(*nameT, 2);
            
            auto hpBar = bar(130, 8, m.hp, c(40, 160, 40), c(20, 50, 20));
            pf->addChild(hpBar);
            Canvas::setLeft(*hpBar, 5);
            Canvas::setTop(*hpBar, 15);
            
            auto mpBar = bar(130, 6, m.mp, c(40, 80, 200), c(15, 25, 60));
            pf->addChild(mpBar);
            Canvas::setLeft(*mpBar, 5);
            Canvas::setTop(*mpBar, 26);
            
            partyY += 40;
            idx++;
        }
    }
    
    // =====================================================================
    // MINIMAP (top-right)
    // =====================================================================
    {
        auto minimapFrame = make<Canvas>();
        minimapFrame->setwidth(180);
        minimapFrame->setheight(200);
        root->addChild(minimapFrame);
        Canvas::setLeft(*minimapFrame, W - 190);
        Canvas::setTop(*minimapFrame, 10);
        
        // Ornate border bg
        auto outerBorder = frame(180, 200, c(60, 50, 30));
        minimapFrame->addChild(outerBorder);
        Canvas::setLeft(*outerBorder, 0);
        Canvas::setTop(*outerBorder, 0);
        
        // Map area — rendered minimap texture
        auto mapImg = img(minimapTex, 170, 170);
        minimapFrame->addChild(mapImg);
        Canvas::setLeft(*mapImg, 5);
        Canvas::setTop(*mapImg, 5);
        
        // Zone name area
        auto zoneBar = frame(170, 20, c(30, 25, 15, 220));
        minimapFrame->addChild(zoneBar);
        Canvas::setLeft(*zoneBar, 5);
        Canvas::setTop(*zoneBar, 175);
        
        auto zoneText = make<Text>("Shadowfang Keep", 9.0f);
        zoneText->setforeground(c(200, 180, 130));
        minimapFrame->addChild(zoneText);
        Canvas::setLeft(*zoneText, 40);
        Canvas::setTop(*zoneText, 178);
        
        // Minimap buttons (zoom, track, etc.)
        f32 bx = 10;
        for (int i = 0; i < 4; i++) {
            auto mbtn = frame(16, 16, c(45, 40, 30));
            minimapFrame->addChild(mbtn);
            Canvas::setLeft(*mbtn, bx);
            Canvas::setTop(*mbtn, 178);
            
            auto mbtnInner = frame(10, 10, c(80, 70, 50));
            minimapFrame->addChild(mbtnInner);
            Canvas::setLeft(*mbtnInner, bx + 3);
            Canvas::setTop(*mbtnInner, 181);
            bx += 19;
        }
    }
    
    // =====================================================================
    // BUFF BAR (below minimap)
    // =====================================================================
    {
        f32 bx = W - 190;
        for (int i = 0; i < 8; i++) {
            auto buffImg = img(buffTextures[i], 20, 20);
            root->addChild(buffImg);
            Canvas::setLeft(*buffImg, bx);
            Canvas::setTop(*buffImg, 215);
            
            // Timer bar under buff
            auto timer = frame(20, 3, c(0, 0, 0, 150));
            root->addChild(timer);
            Canvas::setLeft(*timer, bx);
            Canvas::setTop(*timer, 236);
            
            auto timerFill = frame(20.0f * (0.3f + 0.7f * i / 8.0f), 3, c(255, 255, 255, 150));
            root->addChild(timerFill);
            Canvas::setLeft(*timerFill, bx);
            Canvas::setTop(*timerFill, 236);
            
            bx += 23;
        }
    }
    
    // =====================================================================
    // CHAT WINDOW (bottom-left)
    // =====================================================================
    {
        auto chatFrame = make<Canvas>();
        chatFrame->setwidth(340);
        chatFrame->setheight(180);
        root->addChild(chatFrame);
        Canvas::setLeft(*chatFrame, 10);
        Canvas::setTop(*chatFrame, H - 195);
        
        // Chat background
        auto bg = frame(340, 180, c(10, 10, 15, 200));
        chatFrame->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        // Chat tabs
        const char* tabs[] = {"General", "Combat", "Loot", "Guild"};
        Color tabColors[] = {c(50, 45, 35), c(35, 35, 45), c(35, 35, 45), c(35, 35, 45)};
        f32 tx = 0;
        for (int i = 0; i < 4; i++) {
            auto tab = frame(80, 18, tabColors[i]);
            chatFrame->addChild(tab);
            Canvas::setLeft(*tab, tx);
            Canvas::setTop(*tab, 0);
            
            auto tabText = make<Text>(tabs[i], 9.0f);
            tabText->setforeground(i == 0 ? c(255, 200, 100) : c(150, 150, 150));
            chatFrame->addChild(tabText);
            Canvas::setLeft(*tabText, tx + 12);
            Canvas::setTop(*tabText, 3);
            
            tx += 83;
        }
        
        // Chat messages
        struct ChatMsg { const char* text; Color color; };
        ChatMsg messages[] = {
            {"[Thornguard]: Pulling boss in 5 seconds!",    c(255, 180, 100)},
            {"[Sylvaris]: Ready. Buffs are up.",            c(150, 220, 255)},
            {"[Brokk]: LEEEEROOOY...",                      c(255, 200, 100)},
            {"[Luminia]: Please don't.",                     c(200, 255, 200)},
            {"Dreadlord Malachar casts Shadow Nova!",       c(255, 80, 80)},
            {"[Thornguard]: Interrupt it!! NOW!",           c(255, 180, 100)},
            {"You interrupted Shadow Nova.",                 c(255, 255, 100)},
            {"[Guild] Moonshadow: gz on the kill guys",     c(100, 255, 100)},
            {"[Luminia]: Brokk is dead again...",           c(200, 255, 200)},
            {"[Brokk]: Worth it.",                          c(255, 200, 100)},
        };
        f32 msgY = 22;
        for (auto& msg : messages) {
            auto msgText = make<Text>(msg.text, 9.0f);
            msgText->setforeground(msg.color);
            chatFrame->addChild(msgText);
            Canvas::setLeft(*msgText, 8);
            Canvas::setTop(*msgText, msgY);
            msgY += 14;
        }
        
        // Chat input bar
        auto inputBar = frame(340, 22, c(25, 25, 30, 240));
        chatFrame->addChild(inputBar);
        Canvas::setLeft(*inputBar, 0);
        Canvas::setTop(*inputBar, 158);
        
        auto inputText = make<Text>("Say something...", 9.0f);
        inputText->setforeground(c(100, 100, 100));
        chatFrame->addChild(inputText);
        Canvas::setLeft(*inputText, 8);
        Canvas::setTop(*inputText, 162);
        
        // Scrollbar
        auto scrollBg = frame(8, 136, c(20, 20, 25));
        chatFrame->addChild(scrollBg);
        Canvas::setLeft(*scrollBg, 330);
        Canvas::setTop(*scrollBg, 18);
        
        auto scrollThumb = frame(6, 40, c(80, 70, 50));
        chatFrame->addChild(scrollThumb);
        Canvas::setLeft(*scrollThumb, 331);
        Canvas::setTop(*scrollThumb, 100);
        
        // Border
        auto bTop = frame(340, 1, c(60, 50, 30));
        chatFrame->addChild(bTop);
        Canvas::setLeft(*bTop, 0);
        Canvas::setTop(*bTop, 0);
        
        auto bBot = frame(340, 1, c(40, 35, 20));
        chatFrame->addChild(bBot);
        Canvas::setLeft(*bBot, 0);
        Canvas::setTop(*bBot, 179);
    }
    
    // =====================================================================
    // XP BAR (full width, near bottom)
    // =====================================================================
    {
        auto xpBg = frame(W - 20, 10, c(15, 10, 30));
        root->addChild(xpBg);
        Canvas::setLeft(*xpBg, 10);
        Canvas::setTop(*xpBg, H - 210);
        
        auto xpFill = frame((W - 20) * 0.67f, 10, c(100, 50, 180));
        root->addChild(xpFill);
        Canvas::setLeft(*xpFill, 10);
        Canvas::setTop(*xpFill, H - 210);
        
        // Rested XP (lighter section)
        auto xpRested = frame((W - 20) * 0.12f, 10, c(70, 40, 140, 150));
        root->addChild(xpRested);
        Canvas::setLeft(*xpRested, 10 + (W - 20) * 0.67f);
        Canvas::setTop(*xpRested, H - 210);
        
        auto xpText = make<Text>("67% — 234,500 / 350,000 XP", 7.0f);
        xpText->setforeground(c(200, 180, 255));
        root->addChild(xpText);
        Canvas::setLeft(*xpText, W / 2 - 80);
        Canvas::setTop(*xpText, H - 210);
    }
    
    // =====================================================================
    // ACTION BARS (bottom-center)
    // =====================================================================
    std::vector<Ref<Panel>> procGlows;  // promoted for animation
    {
        // Main action bar
        f32 barWidth = 12 * 42;
        f32 barX = (W - barWidth) / 2;
        f32 barY = H - 55;
        
        auto abBg = frame(barWidth + 16, 50, c(20, 18, 15, 230));
        root->addChild(abBg);
        Canvas::setLeft(*abBg, barX - 8);
        Canvas::setTop(*abBg, barY - 4);
        
        // Ornate end-caps
        auto capL = frame(6, 50, c(80, 65, 35));
        root->addChild(capL);
        Canvas::setLeft(*capL, barX - 8);
        Canvas::setTop(*capL, barY - 4);
        
        auto capR = frame(6, 50, c(80, 65, 35));
        root->addChild(capR);
        Canvas::setLeft(*capR, barX + barWidth + 2);
        Canvas::setTop(*capR, barY - 4);
        
        const char* slotKeys[] = {"1","2","3","4","5","6","7","8","9","0","-","="};
        
        for (int i = 0; i < 12; i++) {
            f32 sx = barX + i * 42;
            
            // Slot background
            auto slot = frame(38, 38, c(30, 28, 22));
            root->addChild(slot);
            Canvas::setLeft(*slot, sx);
            Canvas::setTop(*slot, barY);
            
            // Ability icon
            if (i < 10) {
                auto icon = img(abilityIcons[i], 32, 32);
                root->addChild(icon);
                Canvas::setLeft(*icon, sx + 3);
                Canvas::setTop(*icon, barY + 3);
            }
            
            // Cooldown overlay on some slots
            if (i == 2 || i == 5) {
                auto cdOverlay = frame(32, 32, c(0, 0, 0, 150));
                root->addChild(cdOverlay);
                Canvas::setLeft(*cdOverlay, sx + 3);
                Canvas::setTop(*cdOverlay, barY + 3);
                
                auto cdText = make<Text>(i == 2 ? "3.2" : "12", 10.0f);
                cdText->setforeground(c(255, 255, 100));
                root->addChild(cdText);
                Canvas::setLeft(*cdText, sx + 12);
                Canvas::setTop(*cdText, barY + 14);
            }
            
            // "Proc ready" glow overlay on slots 0 (Fireball) and 3 (Heal)
            if (i == 0 || i == 3) {
                auto glow = make<Panel>();
                glow->setwidth(38);
                glow->setheight(38);
                glow->setbackground(c(255, 255, 100, 80));
                glow->setopacity(0.0f);  // starts invisible, animated
                root->addChild(glow);
                Canvas::setLeft(*glow, sx);
                Canvas::setTop(*glow, barY);
                procGlows.push_back(glow);
            }
            
            // Keybind text
            auto keyText = make<Text>(slotKeys[i], 8.0f);
            keyText->setforeground(c(200, 200, 200, 180));
            root->addChild(keyText);
            Canvas::setLeft(*keyText, sx + 2);
            Canvas::setTop(*keyText, barY + 1);
        }
        
        // ---- Secondary action bar (above main) ----
        f32 bar2Y = barY - 42;
        auto ab2Bg = frame(barWidth + 16, 40, c(15, 14, 12, 180));
        root->addChild(ab2Bg);
        Canvas::setLeft(*ab2Bg, barX - 8);
        Canvas::setTop(*ab2Bg, bar2Y);
        
        for (int i = 0; i < 12; i++) {
            f32 sx = barX + i * 42;
            auto slot = frame(38, 34, c(25, 23, 18));
            root->addChild(slot);
            Canvas::setLeft(*slot, sx);
            Canvas::setTop(*slot, bar2Y + 3);
            
            if (i < 6 && bar2Icons[i]) {
                auto icon = img(bar2Icons[i], 30, 26);
                root->addChild(icon);
                Canvas::setLeft(*icon, sx + 4);
                Canvas::setTop(*icon, bar2Y + 7);
            }
        }
    }
    
    // =====================================================================
    // BAG BUTTONS (bottom-right)
    // =====================================================================
    {
        f32 bagX = W - 10;
        for (int i = 0; i < 5; i++) {
            bagX -= 32;
            auto bag = frame(28, 28, c(50, 40, 25));
            root->addChild(bag);
            Canvas::setLeft(*bag, bagX);
            Canvas::setTop(*bag, H - 38);
            
            auto bagImg = img(iconBag, 24, 24);
            root->addChild(bagImg);
            Canvas::setLeft(*bagImg, bagX + 2);
            Canvas::setTop(*bagImg, H - 36);
        }
    }
    
    // =====================================================================
    // MICRO MENU (bottom-right, above bags)
    // =====================================================================
    {
        const char* menuLabels[] = {"C", "S", "T", "M", "G", "?"};
        f32 mx = W - 10;
        for (int i = 5; i >= 0; i--) {
            mx -= 26;
            auto btn = frame(22, 18, c(40, 35, 25));
            root->addChild(btn);
            Canvas::setLeft(*btn, mx);
            Canvas::setTop(*btn, H - 68);
            
            auto btnText = make<Text>(menuLabels[i], 8.0f);
            btnText->setforeground(c(180, 160, 120));
            root->addChild(btnText);
            Canvas::setLeft(*btnText, mx + 6);
            Canvas::setTop(*btnText, H - 66);
        }
    }
    
    // =====================================================================
    // CASTING BAR (center, slightly below middle)
    // =====================================================================
    Ref<Panel> castBarFill;  // promoted for animation
    {
        f32 castW = 280;
        auto castFrame = make<Canvas>();
        castFrame->setwidth(castW);
        castFrame->setheight(28);
        root->addChild(castFrame);
        Canvas::setLeft(*castFrame, (W - castW) / 2);
        Canvas::setTop(*castFrame, H / 2 + 80);
        
        auto bg = frame(castW, 28, c(15, 15, 20, 220));
        castFrame->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        // Cast bar background track
        auto castBg = frame(castW - 8, 16, c(40, 30, 10));
        castFrame->addChild(castBg);
        Canvas::setLeft(*castBg, 4);
        Canvas::setTop(*castBg, 4);
        
        // Cast bar fill (animated width)
        castBarFill = make<Panel>();
        castBarFill->setwidth(0);
        castBarFill->setheight(16);
        castBarFill->setbackground(c(255, 200, 50, 200));
        castFrame->addChild(castBarFill);
        Canvas::setLeft(*castBarFill, 4);
        Canvas::setTop(*castBarFill, 4);
        
        auto castText = make<Text>("Fireball", 10.0f);
        castText->setforeground(c(255, 255, 255));
        castFrame->addChild(castText);
        Canvas::setLeft(*castText, castW / 2 - 20);
        Canvas::setTop(*castText, 6);
        
        auto castTime = make<Text>("1.8s", 9.0f);
        castTime->setforeground(c(255, 255, 200));
        castFrame->addChild(castTime);
        Canvas::setLeft(*castTime, castW - 30);
        Canvas::setTop(*castTime, 7);
        
        // Border
        auto bT = frame(castW, 1, c(100, 80, 30));
        castFrame->addChild(bT);
        Canvas::setLeft(*bT, 0);
        Canvas::setTop(*bT, 0);
        
        auto bB = frame(castW, 1, c(80, 60, 20));
        castFrame->addChild(bB);
        Canvas::setLeft(*bB, 0);
        Canvas::setTop(*bB, 27);
    }
    
    // =====================================================================
    // COMBAT TEXT (floating damage numbers)
    // =====================================================================
    std::vector<Ref<Text>> combatTexts;  // promoted for animation
    {
        struct DmgText { const char* text; f32 x; f32 y; Color color; f32 size; };
        DmgText dmgTexts[] = {
            {"-4,832",  480, 200, c(255, 255, 100), 16.0f},
            {"CRIT",    490, 185, c(255, 100, 100), 12.0f},
            {"-1,204",  520, 250, c(255, 255, 255), 12.0f},
            {"-892",    450, 280, c(255, 255, 255), 11.0f},
            {"+2,150",  350, 220, c(100, 255, 100), 13.0f},
            {"DODGE",   400, 310, c(255, 255, 255), 10.0f},
            {"-3,621",  550, 230, c(255, 200, 50),  14.0f},
        };
        for (auto& d : dmgTexts) {
            auto t = make<Text>(d.text, d.size);
            t->setforeground(d.color);
            root->addChild(t);
            Canvas::setLeft(*t, d.x);
            Canvas::setTop(*t, d.y);
            combatTexts.push_back(t);
        }
    }
    
    // =====================================================================
    // BOSS HEALTH BAR (top center — raid boss)
    // =====================================================================
    {
        f32 bossW = 350;
        auto bossFrame = make<Canvas>();
        bossFrame->setwidth(bossW);
        bossFrame->setheight(32);
        root->addChild(bossFrame);
        Canvas::setLeft(*bossFrame, (W - bossW) / 2);
        Canvas::setTop(*bossFrame, 10);
        
        auto bg = frame(bossW, 32, c(25, 15, 15, 230));
        bossFrame->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        auto hpBg = frame(bossW - 10, 16, c(50, 15, 15));
        bossFrame->addChild(hpBg);
        Canvas::setLeft(*hpBg, 5);
        Canvas::setTop(*hpBg, 12);
        
        auto hpFill = frame((bossW - 10) * 0.62f, 16, c(180, 25, 25));
        bossFrame->addChild(hpFill);
        Canvas::setLeft(*hpFill, 5);
        Canvas::setTop(*hpFill, 12);
        
        auto bossName = make<Text>("Dreadlord Malachar — Phase 2", 10.0f);
        bossName->setforeground(c(255, 100, 100));
        bossFrame->addChild(bossName);
        Canvas::setLeft(*bossName, 70);
        Canvas::setTop(*bossName, 0);
        
        auto bossHp = make<Text>("62%", 10.0f);
        bossHp->setforeground(c(255, 255, 255));
        bossFrame->addChild(bossHp);
        Canvas::setLeft(*bossHp, bossW / 2 - 10);
        Canvas::setTop(*bossHp, 13);
        
        // Border
        auto bT = frame(bossW, 2, c(150, 40, 40));
        bossFrame->addChild(bT);
        Canvas::setLeft(*bT, 0);
        Canvas::setTop(*bT, 0);
        
        auto bB = frame(bossW, 1, c(100, 30, 30));
        bossFrame->addChild(bB);
        Canvas::setLeft(*bB, 0);
        Canvas::setTop(*bB, 31);
    }
    
    // =====================================================================
    // TOOLTIP (hovering near cursor area)
    // =====================================================================
    Ref<Canvas> tooltipElem;  // promoted for animation
    {
        auto tooltip = make<Canvas>();
        tooltipElem = tooltip;
        tooltip->setwidth(220);
        tooltip->setheight(130);
        tooltip->setopacity(0.0f);  // starts hidden, fades in
        root->addChild(tooltip);
        Canvas::setLeft(*tooltip, 600);
        Canvas::setTop(*tooltip, 350);
        
        auto bg = frame(220, 130, c(15, 12, 20, 240));
        tooltip->addChild(bg);
        Canvas::setLeft(*bg, 0);
        Canvas::setTop(*bg, 0);
        
        auto title = make<Text>("Inferno Blast", 12.0f);
        title->setforeground(c(255, 128, 0));
        tooltip->addChild(title);
        Canvas::setLeft(*title, 10);
        Canvas::setTop(*title, 8);
        
        auto rank = make<Text>("Rank 8 — Fire", 9.0f);
        rank->setforeground(c(200, 200, 200));
        tooltip->addChild(rank);
        Canvas::setLeft(*rank, 10);
        Canvas::setTop(*rank, 26);
        
        auto mana = make<Text>("245 Mana", 9.0f);
        mana->setforeground(c(150, 150, 255));
        tooltip->addChild(mana);
        Canvas::setLeft(*mana, 10);
        Canvas::setTop(*mana, 42);
        
        auto range = make<Text>("35 yd range", 9.0f);
        range->setforeground(c(255, 255, 255));
        tooltip->addChild(range);
        Canvas::setLeft(*range, 10);
        Canvas::setTop(*range, 56);
        
        auto cast = make<Text>("2.5 sec cast", 9.0f);
        cast->setforeground(c(255, 255, 255));
        tooltip->addChild(cast);
        Canvas::setLeft(*cast, 130);
        Canvas::setTop(*cast, 56);
        
        auto desc = make<Text>("Hurls a fiery ball that causes", 9.0f);
        desc->setforeground(c(255, 210, 80));
        tooltip->addChild(desc);
        Canvas::setLeft(*desc, 10);
        Canvas::setTop(*desc, 76);
        
        auto desc2 = make<Text>("1,420 to 1,780 Fire damage.", 9.0f);
        desc2->setforeground(c(255, 210, 80));
        tooltip->addChild(desc2);
        Canvas::setLeft(*desc2, 10);
        Canvas::setTop(*desc2, 90);
        
        auto cd = make<Text>("8 sec cooldown", 9.0f);
        cd->setforeground(c(255, 255, 255));
        tooltip->addChild(cd);
        Canvas::setLeft(*cd, 10);
        Canvas::setTop(*cd, 110);
        
        // Gold border
        auto btop = frame(220, 1, c(120, 100, 50));
        tooltip->addChild(btop);
        Canvas::setLeft(*btop, 0);
        Canvas::setTop(*btop, 0);
        
        auto bbot = frame(220, 1, c(80, 65, 30));
        tooltip->addChild(bbot);
        Canvas::setLeft(*bbot, 0);
        Canvas::setTop(*bbot, 129);
        
        auto bleft = frame(1, 130, c(100, 85, 40));
        tooltip->addChild(bleft);
        Canvas::setLeft(*bleft, 0);
        Canvas::setTop(*bleft, 0);
        
        auto bright = frame(1, 130, c(100, 85, 40));
        tooltip->addChild(bright);
        Canvas::setLeft(*bright, 219);
        Canvas::setTop(*bright, 0);
    }
    
    // =====================================================================
    // ANIMATIONS — stored in _animations to keep Refs alive
    // =====================================================================
    
    // 1. Cast bar fill: 0 → full width over 2.5s, loops forever
    //    easeInOutCubic gives a smooth accelerate-then-decelerate cast feel
    {
        auto anim = make<FloatAnimation>();
        anim->setTargetProperty(&castBarFill->widthProperty());
        anim->setFrom(0.0f);
        anim->setTo(272.0f);  // castW(280) - 8px padding
        anim->setduration(2500.0f);
        anim->setEasingFunction(easing::easeInOutCubic);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        _animations.push_back(anim);
    }
    
    // 2. Proc glow pulse on action bar slots (Fireball & Heal ready)
    //    easeInOutSine gives a gentle, organic glow pulse
    for (auto& glow : procGlows) {
        auto anim = make<FloatAnimation>();
        anim->setTargetProperty(&glow->opacityProperty());
        anim->setFrom(0.0f);
        anim->setTo(1.0f);
        anim->setduration(600.0f);
        anim->setEasingFunction(easing::easeInOutSine);
        anim->setautoReverse(true);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        _animations.push_back(anim);
    }
    
    // 3. Low-HP party member pulse (Brokk)
    //    easeInOutQuad for urgent but smooth danger pulse
    if (brokkFrame) {
        auto anim = make<FloatAnimation>();
        anim->setTargetProperty(&brokkFrame->opacityProperty());
        anim->setFrom(1.0f);
        anim->setTo(0.4f);
        anim->setduration(800.0f);
        anim->setEasingFunction(easing::easeInOutQuad);
        anim->setautoReverse(true);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        _animations.push_back(anim);
    }
    
    // 4. Combat text staggered fade-out and back in
    //    easeOutQuart — quick appear, slow fade like floating damage text
    for (size_t i = 0; i < combatTexts.size(); i++) {
        auto anim = make<FloatAnimation>();
        anim->setTargetProperty(&combatTexts[i]->opacityProperty());
        anim->setFrom(1.0f);
        anim->setTo(0.0f);
        anim->setduration(2000.0f);
        anim->setEasingFunction(easing::easeOutQuart);
        anim->setbeginTime(static_cast<f32>(i) * 400.0f);  // staggered
        anim->setautoReverse(true);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        _animations.push_back(anim);
    }
    
    // 5. Tooltip fade-in then gentle breathing
    if (tooltipElem) {
        // Fade-in: easeOutExpo for a snappy reveal
        auto fadeIn = make<FloatAnimation>();
        fadeIn->setTargetProperty(&tooltipElem->opacityProperty());
        fadeIn->setFrom(0.0f);
        fadeIn->setTo(1.0f);
        fadeIn->setduration(1500.0f);
        fadeIn->setEasingFunction(easing::easeOutExpo);
        fadeIn->setbeginTime(500.0f);  // delay before appearing
        fadeIn->begin();
        _animations.push_back(fadeIn);
        
        // Breathing: easeInOutSine for a natural inhale-exhale rhythm
        auto breathe = make<FloatAnimation>();
        breathe->setTargetProperty(&tooltipElem->opacityProperty());
        breathe->setFrom(0.85f);
        breathe->setTo(1.0f);
        breathe->setduration(2000.0f);
        breathe->setEasingFunction(easing::easeInOutSine);
        breathe->setbeginTime(2200.0f);  // after fade-in finishes
        breathe->setautoReverse(true);
        breathe->setRepeatBehavior(RepeatBehavior::forever());
        breathe->begin();
        _animations.push_back(breathe);
    }
    
    _context->setRoot(root);
}

static CVReturn DisplayLinkCallback(
    CVDisplayLinkRef displayLink,
    const CVTimeStamp* now,
    const CVTimeStamp* outputTime,
    CVOptionFlags flagsIn,
    CVOptionFlags* flagsOut,
    void* displayLinkContext)
{
    @autoreleasepool {
        MetalView* view = (__bridge MetalView*)displayLinkContext;
        [view render];
    }
    return kCVReturnSuccess;
}

- (void)setupDisplayLink {
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, (__bridge void*)self);
    CVDisplayLinkStart(_displayLink);
}

- (void)render {
    if (!_context) {
        NSLog(@"render called but context is nil");
        return;
    }
    
    NSSize size = self.bounds.size;
    CGFloat scale = self.window.backingScaleFactor;
    
    self.metalLayer.drawableSize = CGSizeMake(size.width * scale, size.height * scale);
    
    // Update animations
    _context->update(16.6f);  // ~60fps
    
    // Render
    _context->render(size.width, size.height, scale);
}

- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    CGFloat scale = self.window.backingScaleFactor;
    self.metalLayer.drawableSize = CGSizeMake(newSize.width * scale, newSize.height * scale);
}

- (void)mouseDown:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;  // Flip Y
    _context->processMouseButton(gut::MouseButton::Left, true);
}

- (void)mouseUp:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    location.y = self.bounds.size.height - location.y;
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

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow* window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    NSRect frame = NSMakeRect(0, 0, 1024, 768);
    
    self.window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(NSWindowStyleMaskTitled |
                  NSWindowStyleMaskClosable |
                  NSWindowStyleMaskMiniaturizable |
                  NSWindowStyleMaskResizable)
        backing:NSBackingStoreBuffered
        defer:NO];
    
    self.window.title = @"Realm of Shadows \u2014 Gut + Metal";
    [self.window center];
    
    MetalView* view = [[MetalView alloc] initWithFrame:frame];
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
