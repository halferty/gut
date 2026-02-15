#pragma once
/**
 * @file demo_ui.h
 * @brief Shared UI building code for all Gut demo backends (D3D11, D3D12, Metal)
 *
 * Include this header and call demo::buildDemoUI() from each platform shell.
 * All UI construction is platform-independent; only font loading, clipboard,
 * and windowing stay in the per-backend demo files.
 */

#include "gut_single.h"
#include <functional>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace demo {

// =========================================================================
// Configuration passed in from each backend
// =========================================================================
struct DemoConfig {
    gut::u32 width  = 1100;
    gut::u32 height = 720;
    const char* backendName  = "Unknown";   // e.g. "Direct3D 11"
    const char* backendShort = "??";        // e.g. "D3D11"
    const char* shaderModel  = "N/A";
    const char* extraInfo    = "";          // optional line
};

// =========================================================================
// DrawCanvas — custom panel with render callback (used by Drawing tab)
// =========================================================================
class DrawCanvas : public gut::Panel {
    GUT_OBJECT(DrawCanvas, gut::Panel)
public:
    using DrawFn = std::function<void(gut::RenderContext&, gut::Rectf)>;
    DrawFn drawCallback;

    void onRender(gut::RenderContext& ctx) override {
        Panel::onRender(ctx);
        if (drawCallback) {
            gut::Rectf r = {0, 0, bounds().width, bounds().height};
            drawCallback(ctx, r);
        }
    }
};

// =========================================================================
// GameTextures — procedural textures for the Game UI tab
// =========================================================================
struct GameTextures {
    gut::Ref<gut::Texture> portrait;
    gut::Ref<gut::Texture> boss;
    gut::Ref<gut::Texture> minimap;
    gut::Ref<gut::Texture> iconFireball, iconFrost, iconLightning, iconHeal;
    gut::Ref<gut::Texture> iconShield, iconSword, iconPotion, iconSkull, iconBag;
    gut::Ref<gut::Texture> buffSword, buffShield, buffHaste, buffRegen;
    gut::Ref<gut::Texture> buffMana, buffFire, buffShadow, buffArmor;
    gut::Ref<gut::Texture> nineSlice;
};

// =========================================================================
// DemoResult — returned from buildDemoUI()
// =========================================================================
struct DemoResult {
    gut::Ref<gut::Element> root;
    std::vector<gut::Ref<gut::FloatAnimation>> animations;
};

// =========================================================================
// Helpers
// =========================================================================
inline gut::Color cc(gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) {
    return gut::Color::fromRgba8(r, g, b, a);
}

inline gut::Ref<gut::Panel> makeCard(gut::f32 w = 0) {
    auto card = gut::makeRef<gut::Panel>();
    if (w > 0) card->setwidth(w);
    card->setbackground(cc(32, 35, 48));
    card->setcornerRadius(8.0f);
    card->setborderColor(cc(50, 55, 75));
    card->setborderWidth(1.0f);
    return card;
}

inline gut::Ref<gut::Text> makeHeading(const char* text) {
    auto h = gut::makeRef<gut::Text>();
    h->settext(text);
    h->setfontSize(15.0f);
    h->setfontWeight(gut::FontWeight::Bold);
    h->setforeground(cc(200, 205, 220));
    h->setmargin(gut::Thickness{0, 0, 0, 10});
    return h;
}

inline gut::Ref<gut::Text> makeLabel(const char* text, gut::f32 size = 13.0f) {
    auto lbl = gut::makeRef<gut::Text>();
    lbl->settext(text);
    lbl->setfontSize(size);
    lbl->setforeground(cc(160, 165, 185));
    return lbl;
}

// =========================================================================
// Procedural texture generation
// =========================================================================
inline GameTextures createGameTextures(gut::RenderBackend& rb) {
    using namespace gut;
    GameTextures tex;

    auto makeTex = [&](u32 w, u32 h, std::function<void(u8* px, u32 w, u32 h)> gen) -> Ref<Texture> {
        std::vector<u8> px(w * h * 4, 0);
        gen(px.data(), w, h);
        return rb.createTexture(w, h, px.data());
    };

    auto fillRect = [](u8* px, u32 stride, u32 x0, u32 y0, u32 w, u32 h, u8 r, u8 g, u8 b, u8 a = 255) {
        for (u32 y = y0; y < y0 + h; y++)
            for (u32 x = x0; x < x0 + w; x++) {
                u32 i = (y * stride + x) * 4;
                px[i] = r; px[i+1] = g; px[i+2] = b; px[i+3] = a;
            }
    };

    auto putPx = [](u8* px, u32 stride, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a = 255) {
        u32 i = (y * stride + x) * 4;
        px[i] = r; px[i+1] = g; px[i+2] = b; px[i+3] = a;
    };

    // --- 9-slice texture (48x48) ---
    {
        constexpr int S = 48, R = 10, B = 4;
        std::vector<u8> pixels(S * S * 4, 0);
        for (int y = 0; y < S; ++y)
        for (int x = 0; x < S; ++x) {
            float cx_ = (float)x - S * 0.5f + 0.5f;
            float cy_ = (float)y - S * 0.5f + 0.5f;
            float hx = S * 0.5f - (float)R;
            float hy = S * 0.5f - (float)R;
            float dx = std::max(std::abs(cx_) - hx, 0.0f);
            float dy = std::max(std::abs(cy_) - hy, 0.0f);
            float dist = std::sqrtf(dx*dx + dy*dy) - (float)R;
            u8* p = &pixels[(y * S + x) * 4];
            if      (dist < -B)   { p[0]=60; p[1]=70;  p[2]=100; p[3]=255; }
            else if (dist < 0.5f) { p[0]=100;p[1]=160; p[2]=255; p[3]=255; }
            else                  { p[0]=0;  p[1]=0;   p[2]=0;   p[3]=0;   }
        }
        tex.nineSlice = rb.createTexture(S, S, pixels.data());
    }

    // --- Player portrait (32x32) ---
    tex.portrait = makeTex(32, 32, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 32, 32, 40, 30, 25);
        fillRect(px, w, 8, 10, 16, 18, 210, 170, 130);
        fillRect(px, w, 11, 16, 3, 3, 255, 255, 255);
        fillRect(px, w, 18, 16, 3, 3, 255, 255, 255);
        putPx(px, w, 12, 17, 40, 80, 120);
        putPx(px, w, 19, 17, 40, 80, 120);
        fillRect(px, w, 13, 23, 6, 2, 180, 100, 90);
        fillRect(px, w, 6, 4, 20, 8, 160, 140, 60);
        fillRect(px, w, 8, 2, 16, 4, 180, 160, 80);
        fillRect(px, w, 14, 8, 4, 8, 140, 130, 60);
        fillRect(px, w, 12, 0, 8, 4, 180, 30, 30);
        fillRect(px, w, 14, 0, 4, 2, 220, 50, 40);
        fillRect(px, w, 6, 14, 3, 6, 195, 155, 120);
        fillRect(px, w, 23, 14, 3, 6, 195, 155, 120);
    });

    // --- Boss portrait (32x32) ---
    tex.boss = makeTex(32, 32, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 32, 32, 30, 10, 15);
        fillRect(px, w, 6, 8, 20, 20, 140, 50, 60);
        fillRect(px, w, 4, 2, 4, 10, 80, 30, 30);
        fillRect(px, w, 24, 2, 4, 10, 80, 30, 30);
        fillRect(px, w, 2, 0, 4, 4, 100, 40, 35);
        fillRect(px, w, 26, 0, 4, 4, 100, 40, 35);
        fillRect(px, w, 10, 14, 4, 3, 255, 200, 0);
        fillRect(px, w, 18, 14, 4, 3, 255, 200, 0);
        putPx(px, w, 11, 15, 255, 100, 0);
        putPx(px, w, 19, 15, 255, 100, 0);
        putPx(px, w, 12, 22, 255, 255, 240);
        putPx(px, w, 13, 23, 255, 255, 240);
        putPx(px, w, 19, 22, 255, 255, 240);
        putPx(px, w, 18, 23, 255, 255, 240);
        fillRect(px, w, 11, 21, 10, 3, 100, 20, 30);
        fillRect(px, w, 6, 24, 20, 2, 120, 40, 50);
    });

    // --- Minimap (128x128) ---
    tex.minimap = makeTex(128, 128, [&](u8* px, u32 w, u32 h) {
        for (u32 y = 0; y < h; y++)
            for (u32 x = 0; x < w; x++) {
                u8 noise = ((x * 7 + y * 13) ^ (x * y)) & 0x1F;
                fillRect(px, w, x, y, 1, 1, 30 + noise, 55 + noise, 25 + noise/2);
            }
        for (u32 i = 0; i < 128; i++) {
            u32 px_ = 20 + i * 88 / 128;
            u32 py_ = i;
            for (int d = -2; d <= 2; d++)
                if (px_ + d < w)
                    fillRect(px, w, px_ + d, py_, 1, 1, 110, 90, 55);
        }
        for (u32 x = 0; x < 128; x++) {
            u32 py_ = 60 + (x % 7 > 3 ? 1 : 0);
            for (int d = -1; d <= 1; d++)
                if (py_ + d < h)
                    fillRect(px, w, x, py_ + d, 1, 1, 100, 85, 50);
        }
        for (u32 y = 75; y < 105; y++)
            for (u32 x = 30; x < 70; x++) {
                f32 dx = (f32)x - 50, dy = (f32)y - 90;
                if (dx*dx/400 + dy*dy/225 < 1.0f) {
                    u8 wave = ((x + y) & 3) * 5;
                    fillRect(px, w, x, y, 1, 1, 20 + wave, 40 + wave, 100 + wave);
                }
            }
        auto tree = [&](u32 tx, u32 ty) {
            for (int dy = -3; dy <= 3; dy++)
                for (int dx = -3; dx <= 3; dx++)
                    if (dx*dx + dy*dy <= 9 && tx+dx < w && ty+dy < h)
                        fillRect(px, w, tx+dx, ty+dy, 1, 1, 15, 40, 15);
        };
        tree(90, 25); tree(95, 30); tree(100, 20); tree(85, 35);
        tree(15, 45); tree(20, 50); tree(10, 55);
        fillRect(px, w, 55, 45, 12, 10, 120, 115, 100);
        fillRect(px, w, 72, 50, 8, 8, 110, 105, 90);
        fillRect(px, w, 48, 55, 6, 6, 100, 95, 85);
        fillRect(px, w, 62, 62, 4, 4, 255, 255, 60);
        fillRect(px, w, 63, 60, 2, 2, 255, 255, 60);
    });

    // --- Ability icons (24x24) ---
    tex.iconFireball = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
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
    tex.iconFrost = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 10, 20, 50);
        for (u32 y = 0; y < 24; y++)
            for (u32 x = 0; x < 24; x++) {
                f32 dx = (f32)x - 12, dy = (f32)y - 12;
                f32 dist = dx*dx + dy*dy;
                if (dist < 30) fillRect(px, w, x, y, 1, 1, 220, 240, 255);
                else if (dist < 60) fillRect(px, w, x, y, 1, 1, 100, 180, 255);
                else if (dist < 90) fillRect(px, w, x, y, 1, 1, 40, 100, 200);
            }
        fillRect(px, w, 10, 4, 4, 8, 200, 230, 255);
        fillRect(px, w, 6, 8, 3, 6, 180, 220, 255);
        fillRect(px, w, 16, 7, 3, 7, 180, 220, 255);
    });
    tex.iconLightning = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 15, 10, 40);
        fillRect(px, w, 14, 2, 4, 4, 255, 255, 100);
        fillRect(px, w, 12, 5, 4, 3, 255, 255, 100);
        fillRect(px, w, 10, 7, 6, 3, 255, 255, 100);
        fillRect(px, w, 12, 10, 4, 3, 255, 240, 80);
        fillRect(px, w, 10, 12, 4, 3, 255, 240, 80);
        fillRect(px, w, 8, 14, 6, 3, 255, 220, 60);
        fillRect(px, w, 10, 17, 4, 3, 255, 200, 40);
        fillRect(px, w, 8, 19, 4, 4, 255, 180, 30);
        for (u32 y = 0; y < 24; y++)
            for (u32 x = 0; x < 24; x++) {
                u32 i = (y * w + x) * 4;
                if (px[i+3] > 0 && px[i] < 100)
                    px[i+2] = (u8)std::min(255, (int)px[i+2] + 30);
            }
    });
    tex.iconHeal = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 15, 35, 15);
        fillRect(px, w, 9, 4, 6, 16, 80, 220, 80);
        fillRect(px, w, 4, 9, 16, 6, 80, 220, 80);
        fillRect(px, w, 10, 10, 4, 4, 180, 255, 180);
        putPx(px, w, 5, 5, 200, 255, 200);
        putPx(px, w, 18, 5, 200, 255, 200);
        putPx(px, w, 5, 18, 200, 255, 200);
        putPx(px, w, 18, 18, 200, 255, 200);
    });
    tex.iconShield = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 20, 20, 35);
        for (u32 y = 3; y < 21; y++)
            for (u32 x = 4; x < 20; x++) {
                f32 narrow = 1.0f - (f32)(y - 3) / 18.0f * 0.6f;
                f32 cx_ = 12.0f;
                if (std::abs((f32)x - cx_) < 8.0f * narrow)
                    fillRect(px, w, x, y, 1, 1, 140, 150, 180);
            }
        fillRect(px, w, 11, 6, 2, 12, 200, 180, 60);
        fillRect(px, w, 7, 10, 10, 2, 200, 180, 60);
        for (u32 y = 3; y < 19; y++) {
            putPx(px, w, 5, y, 180, 190, 210);
            putPx(px, w, 18, y, 100, 110, 140);
        }
    });
    tex.iconSword = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 30, 15, 10);
        for (int i = 0; i < 14; i++)
            fillRect(px, w, 16 - i, 3 + i, 3, 2, 200, 210, 220);
        fillRect(px, w, 5, 16, 8, 2, 160, 130, 50);
        fillRect(px, w, 3, 18, 3, 4, 80, 50, 30);
        fillRect(px, w, 4, 19, 2, 3, 100, 70, 40);
        for (int i = 0; i < 10; i++)
            putPx(px, w, 17 - i, 4 + i, 240, 245, 255);
    });
    tex.iconPotion = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 25, 15, 15);
        fillRect(px, w, 7, 10, 10, 11, 160, 30, 30);
        fillRect(px, w, 10, 5, 4, 6, 180, 40, 40);
        fillRect(px, w, 10, 3, 4, 3, 140, 110, 60);
        fillRect(px, w, 8, 12, 3, 6, 220, 60, 60);
        putPx(px, w, 14, 11, 255, 200, 200);
        putPx(px, w, 14, 12, 255, 180, 180);
        fillRect(px, w, 8, 16, 8, 3, 200, 190, 150);
    });
    tex.iconSkull = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 30, 15, 35);
        for (u32 y = 4; y < 16; y++)
            for (u32 x = 6; x < 18; x++) {
                f32 dx = (f32)x - 12, dy = (f32)y - 10;
                if (dx*dx/36 + dy*dy/36 < 1.0f)
                    fillRect(px, w, x, y, 1, 1, 220, 210, 190);
            }
        fillRect(px, w, 8, 9, 3, 3, 20, 10, 25);
        fillRect(px, w, 13, 9, 3, 3, 20, 10, 25);
        putPx(px, w, 11, 13, 40, 30, 35);
        putPx(px, w, 12, 13, 40, 30, 35);
        for (u32 x = 9; x < 15; x++)
            fillRect(px, w, x, 15, 1, 3, (x & 1) ? 200 : 30, (x & 1) ? 195 : 15, (x & 1) ? 180 : 25);
    });
    tex.iconBag = makeTex(24, 24, [&](u8* px, u32 w, u32 h) {
        fillRect(px, w, 0, 0, 24, 24, 25, 20, 10);
        fillRect(px, w, 5, 8, 14, 13, 140, 100, 50);
        fillRect(px, w, 6, 9, 12, 11, 160, 120, 60);
        fillRect(px, w, 7, 6, 10, 4, 120, 85, 40);
        fillRect(px, w, 8, 5, 8, 2, 100, 70, 30);
        fillRect(px, w, 9, 7, 3, 2, 255, 220, 50);
        fillRect(px, w, 13, 7, 2, 2, 255, 210, 40);
        fillRect(px, w, 10, 14, 4, 3, 200, 180, 60);
        fillRect(px, w, 11, 15, 2, 1, 160, 140, 40);
    });

    // --- Buff icons (16x16) ---
    auto makeBuffIcon = [&](u8 r, u8 g, u8 b, std::function<void(u8*, u32)> shape) {
        return makeTex(16, 16, [&, r, g, b, shape](u8* px, u32 w, u32 h) {
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
    tex.buffSword = makeBuffIcon(200, 100, 50, [&](u8* px, u32 w) {
        for (int i = 0; i < 10; i++) fillRect(px, w, 11-i, 3+i, 2, 1, 220, 210, 200);
        fillRect(px, w, 3, 11, 6, 1, 200, 170, 50);
    });
    tex.buffShield = makeBuffIcon(50, 100, 200, [&](u8* px, u32 w) {
        fillRect(px, w, 5, 3, 6, 9, 100, 160, 230);
        fillRect(px, w, 6, 4, 4, 7, 120, 180, 240);
    });
    tex.buffHaste = makeBuffIcon(200, 200, 50, [&](u8* px, u32 w) {
        fillRect(px, w, 8, 3, 2, 10, 255, 255, 100);
        fillRect(px, w, 5, 6, 2, 6, 200, 200, 80);
        fillRect(px, w, 11, 5, 2, 7, 200, 200, 80);
    });
    tex.buffRegen = makeBuffIcon(50, 200, 80, [&](u8* px, u32 w) {
        fillRect(px, w, 6, 4, 4, 8, 80, 230, 100);
        fillRect(px, w, 4, 6, 8, 4, 80, 230, 100);
    });
    tex.buffMana = makeBuffIcon(80, 80, 220, [&](u8* px, u32 w) {
        for (u32 y = 4; y < 12; y++)
            for (u32 x = 4; x < 12; x++) {
                f32 d = std::abs((f32)x-8) + std::abs((f32)y-8);
                if (d < 5) fillRect(px, w, x, y, 1, 1, 120, 150, 255);
            }
    });
    tex.buffFire = makeBuffIcon(220, 80, 30, [&](u8* px, u32 w) {
        fillRect(px, w, 6, 8, 4, 5, 255, 160, 30);
        fillRect(px, w, 7, 5, 2, 4, 255, 200, 60);
        putPx(px, w, 7, 4, 255, 240, 100);
    });
    tex.buffShadow = makeBuffIcon(120, 40, 160, [&](u8* px, u32 w) {
        for (u32 y = 3; y < 13; y++)
            for (u32 x = 3; x < 13; x++) {
                f32 d = ((f32)x-8)*((f32)x-8) + ((f32)y-8)*((f32)y-8);
                if (d < 20) fillRect(px, w, x, y, 1, 1, 160, 60, 200);
            }
    });
    tex.buffArmor = makeBuffIcon(160, 160, 180, [&](u8* px, u32 w) {
        fillRect(px, w, 5, 4, 6, 8, 190, 190, 210);
        fillRect(px, w, 4, 5, 8, 6, 170, 170, 195);
    });

    return tex;
}

// =========================================================================
// TAB 1 — Controls
// =========================================================================
inline gut::Ref<gut::Element> buildControlsTab(const DemoConfig& cfg) {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(24.0f);

    // --- Column 1: Buttons + Toggles ---
    auto col1 = makeRef<StackPanel>(Orientation::Vertical);
    col1->setspacing(16.0f);

    { // Buttons card
        auto card = makeCard(280.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Buttons"));

        auto mb = [&](const char* text) {
            auto btn = makeRef<Button>();
            btn->setlabel(text);
            btn->setwidth(220.0f);
            btn->setheight(34.0f);
            btn->setmargin(Thickness{0, 0, 0, 6});
            return btn;
        };
        inner->addChild(mb("Primary Action"));
        inner->addChild(mb("Secondary"));
        inner->addChild(mb("Settings"));
        inner->addChild(mb("About"));
        card->addChild(inner);
        col1->addChild(card);
    }
    { // Toggles card
        auto card = makeCard(280.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Toggles"));

        auto cb = makeRef<CheckBox>();
        cb->setlabel("Enable VSync");
        cb->setforeground(c(200, 205, 220));
        cb->setmargin(Thickness{0, 0, 0, 4});
        inner->addChild(cb);

        auto r1 = makeRef<RadioButton>();
        r1->setlabel("Quality: High");
        r1->setforeground(c(200, 205, 220));
        r1->setmargin(Thickness{0, 4, 0, 0});
        inner->addChild(r1);

        auto r2 = makeRef<RadioButton>();
        r2->setlabel("Quality: Low");
        r2->setforeground(c(200, 205, 220));
        r2->setmargin(Thickness{0, 4, 0, 0});
        inner->addChild(r2);

        card->addChild(inner);
        col1->addChild(card);
    }
    page->addChild(col1);

    // --- Column 2: Sliders + Progress + TextBox ---
    auto col2 = makeRef<StackPanel>(Orientation::Vertical);
    col2->setspacing(16.0f);

    { // Sliders card
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Sliders"));

        inner->addChild(makeLabel("Brightness"));
        auto s1 = makeRef<Slider>();
        s1->setwidth(270.0f);
        s1->setminimum(0); s1->setmaximum(100); s1->setvalue(75);
        s1->setmargin(Thickness{0, 6, 0, 0});
        inner->addChild(s1);

        auto volLabel = makeLabel("Volume");
        volLabel->setmargin(Thickness{0, 10, 0, 0});
        inner->addChild(volLabel);
        auto s2 = makeRef<Slider>();
        s2->setwidth(270.0f);
        s2->setminimum(0); s2->setmaximum(100); s2->setvalue(50);
        s2->setmargin(Thickness{0, 6, 0, 0});
        inner->addChild(s2);

        card->addChild(inner);
        col2->addChild(card);
    }
    { // ProgressBar card
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Progress"));

        inner->addChild(makeLabel("Loading Assets..."));
        auto prog = makeRef<ProgressBar>();
        prog->setwidth(270.0f);
        prog->setvalue(68.0f);
        prog->setmargin(Thickness{0, 8, 0, 0});
        inner->addChild(prog);

        card->addChild(inner);
        col2->addChild(card);
    }
    { // TextBox card
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Text Input"));

        inner->addChild(makeLabel("Player Name"));
        auto tb = makeRef<TextBox>();
        tb->setwidth(270.0f);
        tb->setplaceholder("Enter your name...");
        tb->setmargin(Thickness{0, 6, 0, 0});
        inner->addChild(tb);

        card->addChild(inner);
        col2->addChild(card);
    }
    page->addChild(col2);

    // --- Column 3: Info card ---
    auto col3 = makeRef<StackPanel>(Orientation::Vertical);
    col3->setspacing(16.0f);
    {
        auto card = makeCard(280.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("System Info"));

        auto addRow = [&](const char* key, const char* val) {
            auto row = makeRef<StackPanel>(Orientation::Horizontal);
            row->setmargin(Thickness{0, 0, 0, 5});
            auto k = makeLabel(key, 12.0f);
            k->setforeground(c(120, 130, 160));
            row->addChild(k);
            auto v = makeRef<Text>();
            v->settext(val);
            v->setfontSize(12.0f);
            v->setforeground(c(200, 210, 240));
            v->setmargin(Thickness{6, 0, 0, 0});
            row->addChild(v);
            inner->addChild(row);
        };
        addRow("Backend:", cfg.backendName);
        addRow("Shader Model:", cfg.shaderModel);
        addRow("Resolution:", (std::to_string(cfg.width) + "x" + std::to_string(cfg.height)).c_str());
        addRow("Features:", "SDF AA, Rounded clip");
        addRow("Blur:", "Multi-pass Kawase");
        if (cfg.extraInfo[0]) addRow("Extra:", cfg.extraInfo);

        card->addChild(inner);
        col3->addChild(card);
    }
    page->addChild(col3);

    return page;
}

// =========================================================================
// TAB 2 — Text & Fonts (+ alignment/wrapping/ellipsis from Metal)
// =========================================================================
inline gut::Ref<gut::Element> buildTextFontsTab() {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(24.0f);

    // Font weight/style card
    {
        auto card = makeCard(560.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{20, 20, 20, 20});
        inner->addChild(makeHeading("Font Weights & Styles"));

        auto addSample = [&](const char* label, FontWeight w, FontStyle s) {
            auto row = makeRef<StackPanel>(Orientation::Horizontal);
            row->setmargin(Thickness{0, 0, 0, 6});
            auto tag = makeRef<Text>();
            tag->settext(label);
            tag->setfontSize(12.0f);
            tag->setforeground(c(110, 120, 155));
            tag->setwidth(100.0f);
            row->addChild(tag);
            auto txt = makeRef<Text>();
            txt->settext("The quick brown fox jumps over the lazy dog");
            txt->setfontSize(16.0f);
            txt->setfontWeight(w);
            txt->setfontStyle(s);
            txt->setforeground(c(220, 225, 240));
            row->addChild(txt);
            inner->addChild(row);
        };
        addSample("Light",       FontWeight::Light,  FontStyle::Normal);
        addSample("Regular",     FontWeight::Normal, FontStyle::Normal);
        addSample("Bold",        FontWeight::Bold,   FontStyle::Normal);
        addSample("Italic",      FontWeight::Normal, FontStyle::Italic);
        addSample("Bold Italic", FontWeight::Bold,   FontStyle::Italic);

        auto sep = makeRef<Panel>();
        sep->setheight(1.0f);
        sep->setbackground(c(50, 55, 75));
        sep->setmargin(Thickness{0, 12, 0, 12});
        inner->addChild(sep);

        auto b = makeRef<Text>();
        b->settext("bold(true) convenience  \xe2\x86\x92 picks Bold weight");
        b->setfontSize(14.0f);
        b->setbold(true);
        b->setforeground(c(180, 200, 140));
        inner->addChild(b);

        auto it = makeRef<Text>();
        it->settext("italic(true) convenience  \xe2\x86\x92 picks Italic style");
        it->setfontSize(14.0f);
        it->setitalic(true);
        it->setforeground(c(140, 180, 200));
        it->setmargin(Thickness{0, 6, 0, 0});
        inner->addChild(it);

        card->addChild(inner);
        page->addChild(card);
    }

    // Right column: font sizes + alignment + ellipsis
    auto rightCol = makeRef<StackPanel>(Orientation::Vertical);
    rightCol->setspacing(16.0f);

    // Font sizes card
    {
        auto card = makeCard(360.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{20, 20, 20, 20});
        inner->addChild(makeHeading("Font Sizes"));

        f32 sizes[] = {10, 13, 16, 20, 28, 40};
        for (auto sz : sizes) {
            auto row = makeRef<StackPanel>(Orientation::Horizontal);
            row->setmargin(Thickness{0, 0, 0, 4});
            auto tag = makeRef<Text>();
            tag->settext((std::to_string((int)sz) + "px").c_str());
            tag->setfontSize(11.0f);
            tag->setforeground(c(110, 120, 155));
            tag->setwidth(45.0f);
            row->addChild(tag);
            auto txt = makeRef<Text>();
            txt->settext("Gut UI");
            txt->setfontSize(sz);
            txt->setforeground(c(220, 225, 240));
            row->addChild(txt);
            inner->addChild(row);
        }
        card->addChild(inner);
        rightCol->addChild(card);
    }

    // Text alignment card (from Metal)
    {
        auto card = makeCard(360.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Text Alignment & Wrapping"));

        std::string sampleText = "The quick brown fox jumps over the lazy dog near the old stone bridge.";
        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(8.0f);
        row->setmargin(Thickness{0, 8, 0, 0});

        struct AlignDemo { Text::TextAlignment align; const char* label; };
        AlignDemo demos[] = {
            {Text::TextAlignment::Left,   "Left"},
            {Text::TextAlignment::Center, "Center"},
            {Text::TextAlignment::Right,  "Right"},
        };
        for (auto& d : demos) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto bg = makeRef<Panel>();
            bg->setwidth(105.0f); bg->setheight(70.0f);
            bg->setcornerRadius(6.0f);
            bg->setbackground(c(35, 35, 50));
            bg->setborderColor(c(60, 60, 80));
            bg->setborderWidth(1.0f);
            bg->setclipToBounds(true);
            auto t = makeRef<Text>(sampleText, 8.0f);
            t->setforeground(c(200, 200, 220));
            t->settextWrapping(Text::TextWrapping::Wrap);
            t->settextAlignment(d.align);
            t->setisHitTestVisible(false);
            t->setpadding(Thickness{4, 3, 4, 3});
            t->setwidth(105.0f);
            bg->addChild(t);
            col->addChild(bg);
            auto lbl = makeLabel(d.label, 9.0f);
            lbl->setmargin(Thickness{0, 2, 0, 0});
            col->addChild(lbl);
            row->addChild(col);
        }
        inner->addChild(row);
        card->addChild(inner);
        rightCol->addChild(card);
    }

    // Text ellipsis card (from Metal)
    {
        auto card = makeCard(360.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Text Ellipsis / Truncation"));

        std::string longText = "The quick brown fox jumps over the lazy dog near the bridge";
        struct EllipsisDemo { const char* label; Text::TextTrimming trim; };
        EllipsisDemo demos[] = {
            {"No Trim (overflow)", Text::TextTrimming::None},
            {"CharacterEllipsis",  Text::TextTrimming::CharacterEllipsis},
            {"WordEllipsis",       Text::TextTrimming::WordEllipsis},
        };
        for (auto& d : demos) {
            auto row = makeRef<StackPanel>(Orientation::Horizontal);
            row->setspacing(8.0f);
            row->setmargin(Thickness{0, 4, 0, 0});

            auto bg = makeRef<Panel>();
            bg->setwidth(200.0f); bg->setheight(24.0f);
            bg->setcornerRadius(4.0f);
            bg->setbackground(c(35, 35, 50));
            bg->setborderColor(c(60, 60, 80));
            bg->setborderWidth(1.0f);
            bg->setclipToBounds(true);
            auto t = makeRef<Text>(longText, 10.0f);
            t->setforeground(c(200, 200, 220));
            t->settextTrimming(d.trim);
            t->setisHitTestVisible(false);
            t->setpadding(Thickness{6, 4, 6, 4});
            t->setwidth(200.0f);
            bg->addChild(t);
            row->addChild(bg);

            auto lbl = makeLabel(d.label, 9.0f);
            lbl->setforeground(c(140, 140, 170));
            row->addChild(lbl);
            inner->addChild(row);
        }
        card->addChild(inner);
        rightCol->addChild(card);
    }

    page->addChild(rightCol);
    return page;
}

// =========================================================================
// TAB 3 — Images & Effects (9-slice, radial grad, inset shadow, drop shadows, borders)
// =========================================================================
inline gut::Ref<gut::Element> buildImagesEffectsTab(const GameTextures& textures) {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto scroll = makeRef<ScrollViewer>();
    auto page = makeRef<StackPanel>(Orientation::Vertical);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(20.0f);

    // --- Row 1: 9-slice + Radial Gradients + Inset Shadows ---
    auto row1 = makeRef<StackPanel>(Orientation::Horizontal);
    row1->setspacing(20.0f);

    Thickness borders{12, 12, 12, 12};

    // 9-Slice card
    {
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("9-Slice Image Rendering"));
        inner->addChild(makeLabel("Corners stay crisp, edges stretch"));

        auto r = makeRef<StackPanel>(Orientation::Horizontal);
        r->setmargin(Thickness{0, 10, 0, 0});
        r->setspacing(10.0f);

        for (auto [w, h, label] : std::initializer_list<std::tuple<f32,f32,const char*>>{
                {48.f, 48.f, "48x48"}, {140.f, 48.f, "140x48"}, {48.f, 90.f, "48x90"}}) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto img = makeRef<Image>(textures.nineSlice);
            img->setwidth(w); img->setheight(h);
            img->setstretch(Image::Stretch::NineSlice);
            img->setsliceBorders(borders);
            col->addChild(img);
            col->addChild(makeLabel(label, 10.0f));
            r->addChild(col);
        }
        inner->addChild(r);

        auto sep = makeRef<Panel>();
        sep->setheight(1.0f); sep->setbackground(c(50, 55, 75));
        sep->setmargin(Thickness{0, 12, 0, 8});
        inner->addChild(sep);

        inner->addChild(makeLabel("Fill vs 9-Slice at 180x50"));
        auto r2 = makeRef<StackPanel>(Orientation::Horizontal);
        r2->setmargin(Thickness{0, 6, 0, 0}); r2->setspacing(14.0f);
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto imgFill = makeRef<Image>(textures.nineSlice);
            imgFill->setwidth(180.0f); imgFill->setheight(50.0f);
            imgFill->setstretch(Image::Stretch::Fill);
            col->addChild(imgFill);
            col->addChild(makeLabel("Fill (distorted)", 10.0f));
            r2->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto imgNS = makeRef<Image>(textures.nineSlice);
            imgNS->setwidth(180.0f); imgNS->setheight(50.0f);
            imgNS->setstretch(Image::Stretch::NineSlice);
            imgNS->setsliceBorders(borders);
            col->addChild(imgNS);
            col->addChild(makeLabel("9-Slice (crisp)", 10.0f));
            r2->addChild(col);
        }
        inner->addChild(r2);
        card->addChild(inner);
        row1->addChild(card);
    }

    // Radial Gradients card
    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Radial Gradients"));

        auto r = makeRef<StackPanel>(Orientation::Horizontal);
        r->setspacing(12.0f); r->setmargin(Thickness{0, 10, 0, 0});

        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto box = makeRef<Panel>();
            box->setwidth(80.0f); box->setheight(80.0f); box->setcornerRadius(8.0f);
            auto brush = makeRef<RadialGradientBrush>(
                Point2f{0.5f, 0.5f}, 0.5f,
                std::vector<GradientStop>{{0.0f, c(255,255,255)}, {1.0f, c(40,80,200)}});
            box->setbackgroundBrush(brush);
            col->addChild(box);
            col->addChild(makeLabel("Centre-out", 10.0f));
            r->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto box = makeRef<Panel>();
            box->setwidth(80.0f); box->setheight(80.0f); box->setcornerRadius(40.0f);
            auto brush = makeRef<RadialGradientBrush>(
                Point2f{0.5f, 0.5f}, 0.5f,
                std::vector<GradientStop>{{0.0f, c(255,220,80)}, {0.5f, c(255,100,40)}, {1.0f, c(80,20,100)}});
            box->setbackgroundBrush(brush);
            col->addChild(box);
            col->addChild(makeLabel("Sunset", 10.0f));
            r->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto box = makeRef<Panel>();
            box->setwidth(80.0f); box->setheight(80.0f); box->setcornerRadius(8.0f);
            auto brush = makeRef<RadialGradientBrush>();
            brush->setCenter({0.3f, 0.3f});
            brush->setGradientOrigin({0.3f, 0.3f});
            brush->setRadiusX(0.6f); brush->setRadiusY(0.6f);
            brush->setStops({{0.0f, c(255,255,255)}, {0.4f, c(60,180,100)}, {1.0f, c(20,60,30)}});
            box->setbackgroundBrush(brush);
            col->addChild(box);
            col->addChild(makeLabel("Off-centre", 10.0f));
            r->addChild(col);
        }
        inner->addChild(r);
        card->addChild(inner);
        row1->addChild(card);
    }

    // Inset Shadows card
    {
        auto card = makeCard(280.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Inset Shadows"));

        auto r = makeRef<StackPanel>(Orientation::Horizontal);
        r->setspacing(12.0f); r->setmargin(Thickness{0, 10, 0, 0});

        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto box = makeRef<Panel>();
            box->setwidth(70.0f); box->setheight(70.0f); box->setcornerRadius(8.0f);
            box->setbackground(c(50, 55, 75));
            box->setinsetShadowColor(c(0, 0, 0, 120));
            box->setinsetShadowBlurRadius(12.0f);
            col->addChild(box);
            col->addChild(makeLabel("Soft", 10.0f));
            r->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto box = makeRef<Panel>();
            box->setwidth(70.0f); box->setheight(70.0f); box->setcornerRadius(8.0f);
            box->setbackground(c(50, 55, 75));
            box->setinsetShadowColor(c(0, 0, 0, 160));
            box->setinsetShadowBlurRadius(10.0f);
            box->setinsetShadowOffsetX(4.0f);
            box->setinsetShadowOffsetY(4.0f);
            col->addChild(box);
            col->addChild(makeLabel("Offset", 10.0f));
            r->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto box = makeRef<Panel>();
            box->setwidth(70.0f); box->setheight(70.0f); box->setcornerRadius(35.0f);
            box->setbackground(c(60, 100, 180));
            box->setinsetShadowColor(c(0, 0, 0, 180));
            box->setinsetShadowBlurRadius(16.0f);
            col->addChild(box);
            col->addChild(makeLabel("Round", 10.0f));
            r->addChild(col);
        }
        inner->addChild(r);
        card->addChild(inner);
        row1->addChild(card);
    }

    page->addChild(row1);

    // --- Row 2: Drop Shadows (from Metal) + Borders & Stroke (from Metal) ---
    auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
    row2->setspacing(20.0f);

    // Drop Shadows card
    {
        auto card = makeCard(500.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Drop Shadows"));

        f32 boxW = 80.0f, boxH = 55.0f;

        // Row A: Subtle, Medium, Heavy
        auto ra = makeRef<StackPanel>(Orientation::Horizontal);
        ra->setspacing(24.0f); ra->setmargin(Thickness{0, 8, 0, 0});

        struct ShadowDemo { const char* label; Color sc; f32 ox, oy, blur; Color bg; Color bc; f32 bw; };
        ShadowDemo demos[] = {
            {"Subtle",    c(0,0,0,80),    1,2,4,    c(50,48,58,240), c(0,0,0,0), 0},
            {"Medium",    c(0,0,0,160),   3,4,10,   c(50,48,58,240), c(0,0,0,0), 0},
            {"Heavy",     c(0,0,0,240),   6,8,20,   c(50,48,58,240), c(0,0,0,0), 0},
            {"Blue Glow", c(80,120,255,180), 0,0,16, c(20,18,35,240), c(0,0,0,0), 0},
            {"Fire Glow", c(255,100,20,160), 0,2,18, c(35,18,12,240), c(0,0,0,0), 0},
            {"Contact",   c(0,0,0,200),   0,12,6,   c(50,48,58,240), c(0,0,0,0), 0},
            {"Hard",      c(0,0,0,220),   5,5,0,    c(60,55,70,255), c(30,25,40), 2},
            {"Comic",     c(20,20,20,255),4,4,0,    c(255,230,80,255), c(20,20,20), 2},
            {"Deep",      c(0,0,0,180),   8,8,0,    c(220,70,90,255), c(180,40,60), 1},
        };

        for (int i = 0; i < 9; i++) {
            if (i == 3 || i == 6) {
                inner->addChild(ra);
                ra = makeRef<StackPanel>(Orientation::Horizontal);
                ra->setspacing(24.0f); ra->setmargin(Thickness{0, 20, 0, 0});
            }
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto& d = demos[i];
            auto box = makeRef<Panel>();
            box->setwidth(boxW); box->setheight(boxH);
            box->setcornerRadius(6.0f);
            box->setbackground(d.bg);
            box->setshadowColor(d.sc);
            box->setshadowOffsetX(d.ox); box->setshadowOffsetY(d.oy);
            box->setshadowBlurRadius(d.blur);
            if (d.bw > 0) { box->setborderColor(d.bc); box->setborderWidth(d.bw); }
            col->addChild(box);
            auto lbl = makeLabel(d.label, 9.0f);
            lbl->setmargin(Thickness{0, 4, 0, 0});
            col->addChild(lbl);
            ra->addChild(col);
        }
        inner->addChild(ra);
        card->addChild(inner);
        row2->addChild(card);
    }

    // Borders & Stroke card
    {
        auto card = makeCard(420.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Borders & Stroke"));

        auto r = makeRef<StackPanel>(Orientation::Horizontal);
        r->setspacing(12.0f); r->setmargin(Thickness{0, 8, 0, 0});

        struct BorderDemo { const char* label; f32 w; f32 h; f32 cr; Color bg; Color bc; f32 bw; Color tc; };
        BorderDemo demos[] = {
            {"Thin",  70, 50, 8,  c(0,0,0,0),      c(180,180,220), 1, c(180,180,200)},
            {"Thick", 70, 50, 10, c(40,35,60),      c(140,100,220), 3, c(180,180,200)},
            {"Gold",  70, 50, 6,  c(30,28,22),      c(210,170,60),  2, c(210,170,60)},
            {"Pill",  70, 32, 16, c(0,0,0,0),       c(80,200,160),  2, c(80,200,160)},
            {"Sharp", 70, 50, 0,  c(50,20,20),      c(220,60,60),   4, c(220,60,60)},
        };
        for (auto& d : demos) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto p = makeRef<Panel>();
            p->setwidth(d.w); p->setheight(d.h);
            p->setcornerRadius(d.cr);
            if (d.bg.a > 0) p->setbackground(d.bg);
            p->setborderColor(d.bc);
            p->setborderWidth(d.bw);
            col->addChild(p);
            auto lbl = makeLabel(d.label, 9.0f);
            lbl->setforeground(d.tc);
            lbl->setmargin(Thickness{0, 4, 0, 0});
            col->addChild(lbl);
            r->addChild(col);
        }
        inner->addChild(r);
        card->addChild(inner);
        row2->addChild(card);
    }

    page->addChild(row2);
    scroll->addChild(page);
    return scroll;
}

// =========================================================================
// TAB 4 — Layouts (WrapPanel, DockPanel, ViewBox, AnchorPanel)
// =========================================================================
inline gut::Ref<gut::Element> buildLayoutsTab() {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(20.0f);

    auto col1 = makeRef<StackPanel>(Orientation::Vertical);
    col1->setspacing(20.0f);

    { // WrapPanel card
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("WrapPanel"));
        inner->addChild(makeLabel("Children flow & wrap to new lines"));

        auto wrap = makeRef<WrapPanel>();
        wrap->setorientation(Orientation::Horizontal);
        wrap->setitemSpacing(6.0f);
        wrap->setlineSpacing(6.0f);
        wrap->setwidth(300.0f);
        wrap->setmargin(Thickness{0, 10, 0, 0});

        const char* tags[] = {"C++","Rust","Go","Python","TypeScript","Zig","Kotlin","Swift","Haskell","Lua"};
        Color tagColors[] = {
            c(80,140,220), c(200,100,60), c(60,180,200), c(60,160,100),
            c(50,120,200), c(200,160,50), c(150,80,200), c(220,100,60),
            c(100,60,180), c(40,80,180)
        };
        for (int i = 0; i < 10; ++i) {
            auto chip = makeRef<Panel>();
            chip->setbackground(tagColors[i]);
            chip->setcornerRadius(12.0f);
            auto lbl = makeRef<Text>();
            lbl->settext(tags[i]);
            lbl->setfontSize(12.0f);
            lbl->setforeground(c(255,255,255));
            lbl->setmargin(Thickness{10, 4, 10, 4});
            chip->addChild(lbl);
            wrap->addChild(chip);
        }
        inner->addChild(wrap);
        card->addChild(inner);
        col1->addChild(card);
    }
    { // DockPanel card
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("DockPanel"));
        inner->addChild(makeLabel("Dock children to edges; last fills"));

        auto dock = makeRef<DockPanel>();
        dock->setwidth(300.0f); dock->setheight(150.0f);
        dock->setlastChildFill(true);
        dock->setmargin(Thickness{0, 10, 0, 0});

        auto topBar = makeRef<Panel>(); topBar->setheight(26.0f);
        topBar->setbackground(c(60,130,200));
        DockPanel::setDock(*topBar, Dock::Top);
        auto topT = makeRef<Text>(); topT->settext("Toolbar"); topT->setfontSize(11.0f);
        topT->setforeground(c(255,255,255)); topT->setmargin(Thickness{8, 5, 0, 0});
        topBar->addChild(topT); dock->addChild(topBar);

        auto botBar = makeRef<Panel>(); botBar->setheight(20.0f);
        botBar->setbackground(c(50,55,70));
        DockPanel::setDock(*botBar, Dock::Bottom);
        auto botT = makeRef<Text>(); botT->settext("Status"); botT->setfontSize(10.0f);
        botT->setforeground(c(150,155,175)); botT->setmargin(Thickness{8, 3, 0, 0});
        botBar->addChild(botT); dock->addChild(botBar);

        auto leftBar = makeRef<Panel>(); leftBar->setwidth(60.0f);
        leftBar->setbackground(c(40,44,60));
        DockPanel::setDock(*leftBar, Dock::Left);
        auto leftT = makeRef<Text>(); leftT->settext("Nav"); leftT->setfontSize(11.0f);
        leftT->setforeground(c(170,175,195)); leftT->setmargin(Thickness{8, 8, 0, 0});
        leftBar->addChild(leftT); dock->addChild(leftBar);

        auto center = makeRef<Panel>(); center->setbackground(c(28,30,42));
        auto centerT = makeRef<Text>(); centerT->settext("Content"); centerT->setfontSize(13.0f);
        centerT->setforeground(c(200,205,220)); centerT->setmargin(Thickness{14, 14, 0, 0});
        center->addChild(centerT); dock->addChild(center);

        inner->addChild(dock);
        card->addChild(inner);
        col1->addChild(card);
    }
    page->addChild(col1);

    auto col2 = makeRef<StackPanel>(Orientation::Vertical);
    col2->setspacing(20.0f);

    { // ViewBox card
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("ViewBox"));
        inner->addChild(makeLabel("Scales child to fit container"));

        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(10.0f); row->setmargin(Thickness{0, 10, 0, 0});

        auto makeVB = [&](f32 w, f32 h, const char* label) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto vb = makeRef<ViewBox>();
            vb->setwidth(w); vb->setheight(h);
            vb->setstretch(ViewBoxStretch::Uniform);
            vb->setborderColor(c(55, 60, 80)); vb->setborderWidth(1.0f);
            auto content = makeRef<StackPanel>(Orientation::Vertical);
            content->setwidth(100.0f); content->setheight(60.0f);
            content->setbackground(c(50, 80, 120)); content->setcornerRadius(4.0f);
            auto t1 = makeRef<Text>(); t1->settext("Hello"); t1->setfontSize(20.0f);
            t1->setforeground(c(255,255,255)); t1->setmargin(Thickness{8, 4, 0, 0});
            content->addChild(t1);
            auto t2 = makeRef<Text>(); t2->settext("World!"); t2->setfontSize(12.0f);
            t2->setforeground(c(200,220,255)); t2->setmargin(Thickness{8, 0, 0, 0});
            content->addChild(t2);
            vb->addChild(content);
            col->addChild(vb);
            col->addChild(makeLabel(label, 10.0f));
            row->addChild(col);
        };
        makeVB(80, 100, "80x100");
        makeVB(140, 80, "140x80");
        makeVB(90, 55, "90x55");
        inner->addChild(row);
        card->addChild(inner);
        col2->addChild(card);
    }
    { // AnchorPanel card
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("AnchorPanel"));
        inner->addChild(makeLabel("Proportional anchors + offsets"));

        auto anchor = makeRef<AnchorPanel>();
        anchor->setwidth(300.0f); anchor->setheight(170.0f);
        anchor->setbackground(c(24, 26, 36));
        anchor->setcornerRadius(4.0f);
        anchor->setborderColor(c(45, 50, 70)); anchor->setborderWidth(1.0f);
        anchor->setmargin(Thickness{0, 10, 0, 0});

        auto makeChip = [&](const char* label, Color bg) {
            auto p = makeRef<Panel>(); p->setbackground(bg); p->setcornerRadius(4.0f);
            auto t = makeRef<Text>(); t->settext(label); t->setfontSize(11.0f);
            t->setforeground(c(255,255,255)); t->setmargin(Thickness{8, 3, 8, 3});
            p->addChild(t); return p;
        };

        auto tl = makeChip("TL", c(80,140,220));
        AnchorPanel::setAnchorLeft(*tl, 0); AnchorPanel::setAnchorTop(*tl, 0);
        AnchorPanel::setOffsetLeft(*tl, 6); AnchorPanel::setOffsetTop(*tl, 6);
        anchor->addChild(tl);

        auto tr = makeChip("TR", c(200,100,60));
        AnchorPanel::setAnchorRight(*tr, 1); AnchorPanel::setAnchorTop(*tr, 0);
        AnchorPanel::setOffsetRight(*tr, 6); AnchorPanel::setOffsetTop(*tr, 6);
        anchor->addChild(tr);

        auto bl = makeChip("BL", c(60,180,100));
        AnchorPanel::setAnchorLeft(*bl, 0); AnchorPanel::setAnchorBottom(*bl, 1);
        AnchorPanel::setOffsetLeft(*bl, 6); AnchorPanel::setOffsetBottom(*bl, 6);
        anchor->addChild(bl);

        auto br = makeChip("BR", c(220,160,40));
        AnchorPanel::setAnchorRight(*br, 1); AnchorPanel::setAnchorBottom(*br, 1);
        AnchorPanel::setOffsetRight(*br, 6); AnchorPanel::setOffsetBottom(*br, 6);
        anchor->addChild(br);

        auto bar = makeRef<Panel>();
        bar->setheight(28.0f); bar->setbackground(c(150, 80, 200)); bar->setcornerRadius(4.0f);
        AnchorPanel::setAnchorLeft(*bar, 0.15f); AnchorPanel::setAnchorRight(*bar, 0.85f);
        AnchorPanel::setAnchorTop(*bar, 0.5f);
        AnchorPanel::setOffsetLeft(*bar, 0); AnchorPanel::setOffsetRight(*bar, 0);
        AnchorPanel::setOffsetTop(*bar, -14);
        auto barT = makeRef<Text>(); barT->settext("15%..85% stretch"); barT->setfontSize(11.0f);
        barT->setforeground(c(255,255,255)); barT->setmargin(Thickness{8, 5, 0, 0});
        bar->addChild(barT); anchor->addChild(bar);

        inner->addChild(anchor);
        card->addChild(inner);
        col2->addChild(card);
    }
    page->addChild(col2);
    return page;
}

// =========================================================================
// TAB 5 — Layouts 2 (UniformGrid, FlexPanel, RelativePanel, RadialPanel)
// =========================================================================
inline gut::Ref<gut::Element> buildLayouts2Tab() {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(20.0f);

    auto col1 = makeRef<StackPanel>(Orientation::Vertical);
    col1->setspacing(20.0f);

    { // UniformGrid
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("UniformGrid"));
        inner->addChild(makeLabel("All cells equal size (3 columns)"));

        auto grid = makeRef<UniformGrid>();
        grid->setcolumns(3); grid->setwidth(300.0f); grid->setheight(200.0f);
        grid->setmargin(Thickness{0, 10, 0, 0});

        Color cellColors[] = {
            c(80,140,220), c(200,100,60), c(60,180,100),
            c(220,160,40), c(150,80,200), c(50,160,180),
            c(200,60,100), c(100,180,60), c(180,120,60)
        };
        for (int i = 0; i < 9; ++i) {
            auto cell = makeRef<Panel>();
            cell->setbackground(cellColors[i]);
            cell->setmargin(Thickness{2,2,2,2}); cell->setcornerRadius(4.0f);
            auto lbl = makeRef<Text>(); lbl->settext(std::to_string(i+1));
            lbl->setfontSize(16.0f); lbl->setforeground(c(255,255,255));
            lbl->setmargin(Thickness{8,8,0,0});
            cell->addChild(lbl); grid->addChild(cell);
        }
        inner->addChild(grid);
        card->addChild(inner);
        col1->addChild(card);
    }
    { // FlexPanel
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("FlexPanel"));
        inner->addChild(makeLabel("CSS Flexbox: grow, spaceEvenly"));

        auto flex = makeRef<FlexPanel>(Orientation::Horizontal);
        flex->setwidth(300.0f); flex->setheight(60.0f); flex->setspacing(6.0f);
        flex->setjustifyContent(FlexJustify::SpaceEvenly);
        flex->setalignItems(FlexAlign::Center);
        flex->setmargin(Thickness{0, 10, 0, 0});
        flex->setbackground(c(24, 26, 36)); flex->setcornerRadius(4.0f);

        Color flexColors[] = {c(80,140,220), c(200,100,60), c(60,180,100)};
        const char* flexLabels[] = {"A", "B", "C"};
        for (int i = 0; i < 3; ++i) {
            auto box = makeRef<Panel>();
            box->setwidth(60.0f); box->setheight(36.0f);
            box->setbackground(flexColors[i]); box->setcornerRadius(6.0f);
            FlexPanel::setFlexGrow(*box, 1.0f);
            auto lbl = makeRef<Text>(); lbl->settext(flexLabels[i]);
            lbl->setfontSize(14.0f); lbl->setforeground(c(255,255,255));
            lbl->setmargin(Thickness{8,8,0,0});
            box->addChild(lbl); flex->addChild(box);
        }
        inner->addChild(flex);

        auto flex2 = makeRef<FlexPanel>(Orientation::Vertical);
        flex2->setwidth(300.0f); flex2->setheight(100.0f); flex2->setspacing(4.0f);
        flex2->setjustifyContent(FlexJustify::SpaceBetween);
        flex2->setalignItems(FlexAlign::Stretch);
        flex2->setmargin(Thickness{0, 8, 0, 0});
        flex2->setbackground(c(24, 26, 36)); flex2->setcornerRadius(4.0f);
        for (int i = 0; i < 4; ++i) {
            auto row = makeRef<Panel>(); row->setheight(20.0f);
            row->setbackground(c(50+i*30, 80, 180-i*20)); row->setcornerRadius(3.0f);
            auto t = makeRef<Text>(); t->settext(String("Row ") + std::to_string(i+1).c_str());
            t->setfontSize(11.0f); t->setforeground(c(255,255,255));
            t->setmargin(Thickness{8,2,0,0});
            row->addChild(t); flex2->addChild(row);
        }
        inner->addChild(flex2);
        card->addChild(inner);
        col1->addChild(card);
    }
    page->addChild(col1);

    auto col2 = makeRef<StackPanel>(Orientation::Vertical);
    col2->setspacing(20.0f);

    { // RelativePanel
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("RelativePanel"));
        inner->addChild(makeLabel("Position relative to each other"));

        auto rel = makeRef<RelativePanel>();
        rel->setwidth(300.0f); rel->setheight(150.0f);
        rel->setbackground(c(24,26,36)); rel->setcornerRadius(4.0f);
        rel->setborderColor(c(45,50,70)); rel->setborderWidth(1.0f);
        rel->setmargin(Thickness{0, 10, 0, 0});

        auto header = makeRef<Panel>(); header->setid("header");
        header->setwidth(120.0f); header->setheight(30.0f);
        header->setbackground(c(80,140,220)); header->setcornerRadius(4.0f);
        RelativePanel::setAlignLeftWithPanel(*header, true);
        RelativePanel::setAlignTopWithPanel(*header, true);
        auto ht = makeRef<Text>(); ht->settext("Header"); ht->setfontSize(12.0f);
        ht->setforeground(c(255,255,255)); ht->setmargin(Thickness{8,6,0,0});
        header->addChild(ht); rel->addChild(header);

        auto sidebar = makeRef<Panel>(); sidebar->setid("sidebar");
        sidebar->setwidth(70.0f); sidebar->setheight(80.0f);
        sidebar->setbackground(c(60,180,100)); sidebar->setcornerRadius(4.0f);
        RelativePanel::setBelow(*sidebar, "header");
        RelativePanel::setAlignLeftWithPanel(*sidebar, true);
        auto st = makeRef<Text>(); st->settext("Side"); st->setfontSize(11.0f);
        st->setforeground(c(255,255,255)); st->setmargin(Thickness{8,6,0,0});
        sidebar->addChild(st); rel->addChild(sidebar);

        auto content = makeRef<Panel>(); content->setid("content");
        content->setwidth(140.0f); content->setheight(80.0f);
        content->setbackground(c(200,100,60)); content->setcornerRadius(4.0f);
        RelativePanel::setRightOf(*content, "sidebar");
        RelativePanel::setBelow(*content, "header");
        auto ct = makeRef<Text>(); ct->settext("Content"); ct->setfontSize(11.0f);
        ct->setforeground(c(255,255,255)); ct->setmargin(Thickness{8,6,0,0});
        content->addChild(ct); rel->addChild(content);

        auto footer = makeRef<Panel>(); footer->setid("footer");
        footer->setwidth(100.0f); footer->setheight(24.0f);
        footer->setbackground(c(150,80,200)); footer->setcornerRadius(4.0f);
        RelativePanel::setAlignRightWithPanel(*footer, true);
        RelativePanel::setAlignBottomWithPanel(*footer, true);
        auto ft = makeRef<Text>(); ft->settext("Footer"); ft->setfontSize(10.0f);
        ft->setforeground(c(255,255,255)); ft->setmargin(Thickness{8,4,0,0});
        footer->addChild(ft); rel->addChild(footer);

        inner->addChild(rel);
        card->addChild(inner);
        col2->addChild(card);
    }
    { // RadialPanel
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("RadialPanel"));
        inner->addChild(makeLabel("Arrange children in a circle"));

        auto radial = makeRef<RadialPanel>();
        radial->setwidth(300.0f); radial->setheight(260.0f);
        radial->setbackground(c(24,26,36)); radial->setcornerRadius(4.0f);
        radial->setborderColor(c(45,50,70)); radial->setborderWidth(1.0f);
        radial->setmargin(Thickness{0, 10, 0, 0});

        const char* hours[] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
        for (int i = 0; i < 12; ++i) {
            auto dot = makeRef<Panel>();
            dot->setwidth(32.0f); dot->setheight(32.0f);
            dot->setbackground(c(50+i*15, 100, 220-i*10)); dot->setcornerRadius(16.0f);
            auto t = makeRef<Text>(); t->settext(hours[i]); t->setfontSize(11.0f);
            t->setforeground(c(255,255,255)); t->setmargin(Thickness{8,7,0,0});
            dot->addChild(t); radial->addChild(dot);
        }
        inner->addChild(radial);
        card->addChild(inner);
        col2->addChild(card);
    }
    page->addChild(col2);
    return page;
}

// =========================================================================
// TAB 6 — Drawing (Polylines, Paths, Element Scale)
// =========================================================================
inline gut::Ref<gut::Element> buildDrawingTab() {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(20.0f);

    auto col1 = makeRef<StackPanel>(Orientation::Vertical);
    col1->setspacing(20.0f);

    { // Polyline basics
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Polylines"));
        inner->addChild(makeLabel("Joins & caps"));

        auto canvas = makeRef<DrawCanvas>();
        canvas->setwidth(308.0f); canvas->setheight(200.0f);
        canvas->setmargin(Thickness{0, 10, 0, 0});
        canvas->setbackground(c(22, 24, 34)); canvas->setcornerRadius(4.0f);
        canvas->setclipToBounds(true);
        canvas->drawCallback = [](RenderContext& ctx, Rectf r) {
            Point2f zigzag[] = {{20,170},{60,40},{100,160},{140,50},{180,150},{220,30},{260,140},{290,60}};
            ctx.drawPolyline(zigzag, 8, cc(80,200,255), 3.0f, false, LineJoin::Round, LineCap::Round);
            Point2f tri[] = {{80,180},{50,130},{110,130}};
            ctx.drawPolyline(tri, 3, cc(255,180,60), 2.0f, true, LineJoin::Miter, LineCap::Flat);
        };
        inner->addChild(canvas);
        card->addChild(inner);
        col1->addChild(card);
    }
    { // Filled polygons
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Filled Polygons"));
        inner->addChild(makeLabel("Ear-clipping triangulation"));

        auto canvas = makeRef<DrawCanvas>();
        canvas->setwidth(308.0f); canvas->setheight(200.0f);
        canvas->setmargin(Thickness{0, 10, 0, 0});
        canvas->setbackground(c(22, 24, 34)); canvas->setcornerRadius(4.0f);
        canvas->setclipToBounds(true);
        canvas->drawCallback = [](RenderContext& ctx, Rectf r) {
            Point2f star[10];
            for (int i = 0; i < 10; ++i) {
                float angle = (float)i * 3.14159f * 2.0f / 10.0f - 3.14159f / 2.0f;
                float rad = (i % 2 == 0) ? 70.0f : 30.0f;
                star[i] = {100 + rad * cosf(angle), 100 + rad * sinf(angle)};
            }
            ctx.fillPolygon(star, 10, cc(200,80,255,180));
            ctx.drawPolyline(star, 10, cc(220,120,255), 1.5f, true);
            Point2f pent[5];
            for (int i = 0; i < 5; ++i) {
                float angle = (float)i * 3.14159f * 2.0f / 5.0f - 3.14159f / 2.0f;
                pent[i] = {240 + 50*cosf(angle), 100 + 50*sinf(angle)};
            }
            ctx.fillPolygon(pent, 5, cc(60,200,120,180));
            ctx.drawPolyline(pent, 5, cc(100,240,160), 1.5f, true);
        };
        inner->addChild(canvas);
        card->addChild(inner);
        col1->addChild(card);
    }
    page->addChild(col1);

    auto col2 = makeRef<StackPanel>(Orientation::Vertical);
    col2->setspacing(20.0f);

    { // Path fill
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Path Fill"));
        inner->addChild(makeLabel("Bezier curves flattened & triangulated"));

        auto canvas = makeRef<DrawCanvas>();
        canvas->setwidth(308.0f); canvas->setheight(200.0f);
        canvas->setmargin(Thickness{0, 10, 0, 0});
        canvas->setbackground(c(22, 24, 34)); canvas->setcornerRadius(4.0f);
        canvas->setclipToBounds(true);
        canvas->drawCallback = [](RenderContext& ctx, Rectf r) {
            auto heart = makeRef<Path>();
            heart->moveTo(154,80);
            heart->cubicTo(154,60,130,30,100,30);
            heart->cubicTo(55,30,20,70,20,110);
            heart->cubicTo(20,170,100,200,154,230);
            heart->cubicTo(208,200,288,170,288,110);
            heart->cubicTo(288,70,253,30,208,30);
            heart->cubicTo(178,30,154,60,154,80);
            heart->close();
            ctx.fillPath(*heart, cc(220,60,80,200));
            auto rr = makeRef<Path>();
            rr->addRoundedRect({20,150,80,40}, 10);
            ctx.fillPath(*rr, cc(60,140,220,180));
        };
        inner->addChild(canvas);
        card->addChild(inner);
        col2->addChild(card);
    }
    { // Path stroke
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Path Stroke"));
        inner->addChild(makeLabel("Thick stroked curves"));

        auto canvas = makeRef<DrawCanvas>();
        canvas->setwidth(308.0f); canvas->setheight(200.0f);
        canvas->setmargin(Thickness{0, 10, 0, 0});
        canvas->setbackground(c(22, 24, 34)); canvas->setcornerRadius(4.0f);
        canvas->setclipToBounds(true);
        canvas->drawCallback = [](RenderContext& ctx, Rectf r) {
            auto wave = makeRef<Path>();
            wave->moveTo(20,100);
            wave->cubicTo(80,20,140,180,200,100);
            wave->cubicTo(240,50,260,50,290,100);
            ctx.strokePath(*wave, cc(255,200,60), 4.0f);
            auto ell = makeRef<Path>(); ell->addEllipse({154,140}, 80, 30);
            ctx.strokePath(*ell, cc(100,220,200), 2.5f);
            auto circ = makeRef<Path>(); circ->addCircle({80,60}, 30);
            ctx.strokePath(*circ, cc(200,100,255), 3.0f);
        };
        inner->addChild(canvas);
        card->addChild(inner);
        col2->addChild(card);
    }
    { // Fill + Stroke
        auto card = makeCard(340.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Fill + Stroke"));

        auto canvas = makeRef<DrawCanvas>();
        canvas->setwidth(308.0f); canvas->setheight(160.0f);
        canvas->setmargin(Thickness{0, 10, 0, 0});
        canvas->setbackground(c(22, 24, 34)); canvas->setcornerRadius(4.0f);
        canvas->setclipToBounds(true);
        canvas->drawCallback = [](RenderContext& ctx, Rectf r) {
            auto rr = makeRef<Path>(); rr->addRoundedRect({20,20,120,120}, 16);
            ctx.fillPath(*rr, cc(60,80,140,180));
            ctx.strokePath(*rr, cc(120,160,240), 2.5f);
            auto ci = makeRef<Path>(); ci->addCircle({230,80}, 50);
            ctx.fillPath(*ci, cc(140,60,120,180));
            ctx.strokePath(*ci, cc(220,120,200), 2.5f);
        };
        inner->addChild(canvas);
        card->addChild(inner);
        col2->addChild(card);
    }
    page->addChild(col2);

    // Column 3: Element scale
    auto col3 = makeRef<StackPanel>(Orientation::Vertical);
    col3->setspacing(20.0f);
    {
        auto card = makeCard(240.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Element Scale"));

        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(16.0f); row->setmargin(Thickness{0, 14, 0, 0});
        struct ScaleDemo { f32 sx; f32 sy; Color bg; const char* label; };
        ScaleDemo demos[] = {{1,1,c(80,150,240),"1.0x"},{1.5f,1.5f,c(200,100,60),"1.5x"},{0.6f,0.6f,c(60,180,100),"0.6x"}};
        for (auto& d : demos) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50.0f); box->setheight(50.0f); box->setcornerRadius(6.0f);
            box->setbackground(d.bg); box->setscaleX(d.sx); box->setscaleY(d.sy);
            col->addChild(box);
            col->addChild(makeLabel(d.label, 10.0f));
            row->addChild(col);
        }
        inner->addChild(row);

        auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
        row2->setspacing(16.0f); row2->setmargin(Thickness{0, 24, 0, 0});
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50); box->setcornerRadius(25);
            box->setbackground(c(200,80,200)); box->setscaleX(1.8f); box->setscaleY(1.0f);
            col->addChild(box); col->addChild(makeLabel("1.8x\xc3\x97" "1.0x", 10.0f));
            row2->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50); box->setcornerRadius(25);
            box->setbackground(c(255,200,60)); box->setscaleX(1.0f); box->setscaleY(1.8f);
            col->addChild(box); col->addChild(makeLabel("1.0x\xc3\x97" "1.8x", 10.0f));
            row2->addChild(col);
        }
        inner->addChild(row2);
        card->addChild(inner);
        col3->addChild(card);
    }
    {
        auto card = makeCard(240.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Scaled Button"));
        inner->addChild(makeLabel("Interactive at 1.3x"));
        auto btn = makeRef<Button>();
        btn->setlabel("Click Me"); btn->setwidth(100.0f); btn->setheight(36.0f);
        btn->setscaleX(1.3f); btn->setscaleY(1.3f);
        btn->setmargin(Thickness{20, 20, 0, 0});
        inner->addChild(btn);
        card->addChild(inner);
        col3->addChild(card);
    }
    page->addChild(col3);
    return page;
}

// =========================================================================
// TAB 7 — Transforms (Rotation, Skew, Combined)
// =========================================================================
inline gut::Ref<gut::Element> buildTransformsTab() {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };
    const float pi = 3.14159265f;

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(20.0f);

    // Col 1: Rotation
    auto col1 = makeRef<StackPanel>(Orientation::Vertical);
    col1->setspacing(20.0f);

    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Rotation"));

        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(28.0f); row->setmargin(Thickness{10, 20, 0, 0});

        struct RotDemo { f32 angle; Color bg; const char* label; };
        RotDemo demos[] = {{0,c(80,150,240),"0\xc2\xb0"},{15*pi/180,c(200,100,60),"15\xc2\xb0"},{45*pi/180,c(60,180,100),"45\xc2\xb0"},{90*pi/180,c(220,160,40),"90\xc2\xb0"}};
        for (auto& d : demos) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50); box->setcornerRadius(6);
            box->setbackground(d.bg); box->setrotation(d.angle);
            col->addChild(box);
            auto lbl = makeLabel(d.label, 10.0f);
            lbl->setmargin(Thickness{0, 8, 0, 0});
            col->addChild(lbl);
            row->addChild(col);
        }
        inner->addChild(row);

        auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
        row2->setspacing(28.0f); row2->setmargin(Thickness{10, 28, 0, 0});
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(60); box->setheight(35); box->setcornerRadius(6);
            box->setbackground(c(150,80,200)); box->setrotation(pi);
            auto t = makeRef<Text>(); t->settext("Hi"); t->setfontSize(14.0f);
            t->setforeground(c(255,255,255)); t->setmargin(Thickness{8,6,0,0});
            box->addChild(t); col->addChild(box);
            auto lbl = makeLabel("180\xc2\xb0", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row2->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(60); box->setheight(35); box->setcornerRadius(6);
            box->setbackground(c(50,160,180)); box->setrotation(-30.0f * pi / 180.0f);
            auto t = makeRef<Text>(); t->settext("Tilt"); t->setfontSize(14.0f);
            t->setforeground(c(255,255,255)); t->setmargin(Thickness{6,6,0,0});
            box->addChild(t); col->addChild(box);
            auto lbl = makeLabel("-30\xc2\xb0", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row2->addChild(col);
        }
        inner->addChild(row2);
        card->addChild(inner);
        col1->addChild(card);
    }
    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Rotated Button"));
        auto btn = makeRef<Button>();
        btn->setlabel("Click Me"); btn->setwidth(120); btn->setheight(36);
        btn->setrotation(10.0f * pi / 180.0f);
        btn->setmargin(Thickness{20, 24, 0, 0});
        inner->addChild(btn);
        card->addChild(inner);
        col1->addChild(card);
    }
    page->addChild(col1);

    // Col 2: Skew
    auto col2 = makeRef<StackPanel>(Orientation::Vertical);
    col2->setspacing(20.0f);
    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Skew"));

        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(28.0f); row->setmargin(Thickness{10, 20, 0, 0});

        struct SkewDemo { f32 sx; f32 sy; Color bg; const char* label; };
        SkewDemo demos[] = {
            {15*pi/180,0,c(80,150,240),"skewX 15\xc2\xb0"},
            {30*pi/180,0,c(200,100,60),"skewX 30\xc2\xb0"},
            {0,20*pi/180,c(60,180,100),"skewY 20\xc2\xb0"},
        };
        for (auto& d : demos) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50);
            box->setbackground(d.bg); box->setskewX(d.sx); box->setskewY(d.sy);
            col->addChild(box);
            auto lbl = makeLabel(d.label, 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row->addChild(col);
        }
        inner->addChild(row);

        auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
        row2->setspacing(28.0f); row2->setmargin(Thickness{10, 28, 0, 0});
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(60); box->setheight(60);
            box->setbackground(c(220,160,40));
            box->setskewX(15*pi/180); box->setskewY(10*pi/180);
            col->addChild(box);
            auto lbl = makeLabel("X+Y skew", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row2->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(80); box->setheight(40); box->setcornerRadius(6);
            box->setbackground(c(150,80,200));
            box->setskewX(-12*pi/180);
            auto t = makeRef<Text>(); t->settext("Slanted"); t->setfontSize(14.0f);
            t->setforeground(c(255,255,255)); t->setmargin(Thickness{10,8,0,0});
            box->addChild(t); col->addChild(box);
            auto lbl = makeLabel("skewX -12\xc2\xb0", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row2->addChild(col);
        }
        inner->addChild(row2);
        card->addChild(inner);
        col2->addChild(card);
    }
    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Skewed Button"));
        auto btn = makeRef<Button>();
        btn->setlabel("Click Me"); btn->setwidth(120); btn->setheight(36);
        btn->setskewX(15.0f * pi / 180.0f);
        btn->setmargin(Thickness{20, 24, 0, 0});
        inner->addChild(btn);
        card->addChild(inner);
        col2->addChild(card);
    }
    page->addChild(col2);

    // Col 3: Combined
    auto col3 = makeRef<StackPanel>(Orientation::Vertical);
    col3->setspacing(20.0f);
    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Combined Transforms"));

        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(28.0f); row->setmargin(Thickness{10, 20, 0, 0});
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50); box->setcornerRadius(6);
            box->setbackground(c(80,150,240));
            box->setscaleX(1.3f); box->setscaleY(1.3f);
            box->setrotation(20*pi/180);
            col->addChild(box);
            auto lbl = makeLabel("1.3x+20\xc2\xb0", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50);
            box->setbackground(c(200,100,60));
            box->setrotation(-15*pi/180); box->setskewX(20*pi/180);
            col->addChild(box);
            auto lbl = makeLabel("-15\xc2\xb0+skX", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(50); box->setheight(50); box->setcornerRadius(6);
            box->setbackground(c(60,180,100));
            box->setscaleX(1.4f); box->setskewY(15*pi/180);
            col->addChild(box);
            auto lbl = makeLabel("1.4x+skY", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row->addChild(col);
        }
        inner->addChild(row);

        auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
        row2->setspacing(28.0f); row2->setmargin(Thickness{10, 28, 0, 0});
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(70); box->setheight(70); box->setcornerRadius(8);
            box->setbackground(c(220,80,120));
            box->setscaleX(1.2f); box->setscaleY(0.8f);
            box->setrotation(25*pi/180); box->setskewX(10*pi/180);
            auto t = makeRef<Text>(); t->settext("All 3"); t->setfontSize(13.0f);
            t->setforeground(c(255,255,255)); t->setmargin(Thickness{12,24,0,0});
            box->addChild(t); col->addChild(box);
            auto lbl = makeLabel("S+R+Sk", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row2->addChild(col);
        }
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(80); box->setheight(50); box->setcornerRadius(25);
            box->setbackground(c(255,200,60));
            box->setrotation(30*pi/180); box->setscaleX(1.2f);
            col->addChild(box);
            auto lbl = makeLabel("Pill 30\xc2\xb0", 10.0f); lbl->setmargin(Thickness{0,8,0,0});
            col->addChild(lbl); row2->addChild(col);
        }
        inner->addChild(row2);
        card->addChild(inner);
        col3->addChild(card);
    }
    {
        auto card = makeCard(320.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Full Transform Button"));
        auto btn = makeRef<Button>();
        btn->setlabel("Click Me"); btn->setwidth(130); btn->setheight(36);
        btn->setscaleX(1.2f); btn->setscaleY(1.2f);
        btn->setrotation(12*pi/180); btn->setskewX(8*pi/180);
        btn->setmargin(Thickness{20, 24, 0, 0});
        inner->addChild(btn);
        card->addChild(inner);
        col3->addChild(card);
    }
    page->addChild(col3);
    return page;
}

// =========================================================================
// TAB 8 — Clipping & Blur
// =========================================================================
inline gut::Ref<gut::Element> buildClippingBlurTab() {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(24.0f);

    // Rounded Clipping card
    {
        auto card = makeCard(480.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Rounded Clipping"));
        inner->addChild(makeLabel("Children clipped to rounded parent bounds"));

        auto row = makeRef<StackPanel>(Orientation::Horizontal);
        row->setspacing(16.0f); row->setmargin(Thickness{0, 12, 0, 0});

        // Style 1: Gradient clipped
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto clip = makeRef<Panel>();
            clip->setwidth(100); clip->setheight(80); clip->setcornerRadius(16);
            clip->setclipToBounds(true); clip->setbackground(c(40,40,55));
            auto grad = makeRef<Panel>();
            grad->setwidth(140); grad->setheight(120);
            grad->setbackgroundGradientTop(c(255,100,50));
            grad->setbackgroundGradientBottom(c(180,30,200));
            Canvas::setLeft(*grad, -20); Canvas::setTop(*grad, -20);
            clip->addChild(grad);
            col->addChild(clip);
            col->addChild(makeLabel("Gradient", 9.0f));
            row->addChild(col);
        }
        // Style 2: Quadrants
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto clip = makeRef<Panel>();
            clip->setwidth(100); clip->setheight(80); clip->setcornerRadius(16);
            clip->setclipToBounds(true); clip->setbackground(c(30,30,45));
            auto q1 = makeRef<Panel>(); q1->setwidth(50); q1->setheight(40);
            q1->setbackground(c(220,50,50)); Canvas::setLeft(*q1,0); Canvas::setTop(*q1,0);
            clip->addChild(q1);
            auto q2 = makeRef<Panel>(); q2->setwidth(50); q2->setheight(40);
            q2->setbackground(c(50,180,50)); Canvas::setLeft(*q2,50); Canvas::setTop(*q2,0);
            clip->addChild(q2);
            auto q3 = makeRef<Panel>(); q3->setwidth(50); q3->setheight(40);
            q3->setbackground(c(50,100,220)); Canvas::setLeft(*q3,0); Canvas::setTop(*q3,40);
            clip->addChild(q3);
            auto q4 = makeRef<Panel>(); q4->setwidth(50); q4->setheight(40);
            q4->setbackground(c(220,180,50)); Canvas::setLeft(*q4,50); Canvas::setTop(*q4,40);
            clip->addChild(q4);
            col->addChild(clip);
            col->addChild(makeLabel("Quadrants", 9.0f));
            row->addChild(col);
        }
        // Style 3: Pill
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto pill = makeRef<Panel>();
            pill->setwidth(100); pill->setheight(40); pill->setcornerRadius(20);
            pill->setclipToBounds(true); pill->setbackground(c(60,30,120));
            auto fill = makeRef<Panel>();
            fill->setwidth(70); fill->setheight(40);
            fill->setbackgroundGradientTop(c(100,200,255));
            fill->setbackgroundGradientBottom(c(50,120,200));
            Canvas::setLeft(*fill, 0); Canvas::setTop(*fill, 0);
            pill->addChild(fill);
            col->addChild(pill);
            col->addChild(makeLabel("Pill", 9.0f));
            row->addChild(col);
        }
        // Style 4: Text clip
        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            auto clip = makeRef<Panel>();
            clip->setwidth(100); clip->setheight(80); clip->setcornerRadius(16);
            clip->setclipToBounds(true); clip->setbackground(c(35,55,45));
            auto t1 = makeRef<Text>("ABCDEFGHIJKLMNOPQR", 11.0f);
            t1->setforeground(c(200,255,180)); t1->setisHitTestVisible(false);
            Canvas::setLeft(*t1, -4); Canvas::setTop(*t1, -2);
            clip->addChild(t1);
            auto t2 = makeRef<Text>("The quick brown fox", 9.0f);
            t2->setforeground(c(255,255,255)); t2->setisHitTestVisible(false);
            Canvas::setLeft(*t2, 2); Canvas::setTop(*t2, 20);
            clip->addChild(t2);
            auto t3 = makeRef<Text>("jumps over the lazy", 9.0f);
            t3->setforeground(c(255,255,255)); t3->setisHitTestVisible(false);
            Canvas::setLeft(*t3, 2); Canvas::setTop(*t3, 36);
            clip->addChild(t3);
            auto t4 = makeRef<Text>("STUVWXYZ0123456789", 11.0f);
            t4->setforeground(c(200,255,180)); t4->setisHitTestVisible(false);
            Canvas::setLeft(*t4, -4); Canvas::setTop(*t4, 62);
            clip->addChild(t4);
            col->addChild(clip);
            col->addChild(makeLabel("Text Clip", 9.0f));
            row->addChild(col);
        }
        inner->addChild(row);
        card->addChild(inner);
        page->addChild(card);
    }

    // Backdrop Blur card
    {
        auto card = makeCard(420.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Backdrop Blur / Frosted Glass"));
        inner->addChild(makeLabel("Gaussian blur of content behind the panel"));

        // Background content to blur through
        auto container = makeRef<Canvas>();
        container->setwidth(380.0f); container->setheight(180.0f);
        container->setmargin(Thickness{0, 12, 0, 0});

        // Colorful background
        auto bgStripe1 = makeRef<Panel>();
        bgStripe1->setwidth(380); bgStripe1->setheight(60);
        bgStripe1->setbackgroundGradientTop(c(200,60,60));
        bgStripe1->setbackgroundGradientBottom(c(200,120,40));
        container->addChild(bgStripe1);
        Canvas::setLeft(*bgStripe1, 0); Canvas::setTop(*bgStripe1, 0);

        auto bgStripe2 = makeRef<Panel>();
        bgStripe2->setwidth(380); bgStripe2->setheight(60);
        bgStripe2->setbackgroundGradientTop(c(40,120,200));
        bgStripe2->setbackgroundGradientBottom(c(40,200,120));
        container->addChild(bgStripe2);
        Canvas::setLeft(*bgStripe2, 0); Canvas::setTop(*bgStripe2, 60);

        auto bgStripe3 = makeRef<Panel>();
        bgStripe3->setwidth(380); bgStripe3->setheight(60);
        bgStripe3->setbackgroundGradientTop(c(120,40,200));
        bgStripe3->setbackgroundGradientBottom(c(200,40,120));
        container->addChild(bgStripe3);
        Canvas::setLeft(*bgStripe3, 0); Canvas::setTop(*bgStripe3, 120);

        auto bgText = makeRef<Text>("Background content visible through blur", 14.0f);
        bgText->setforeground(c(255,255,255));
        bgText->setisHitTestVisible(false);
        container->addChild(bgText);
        Canvas::setLeft(*bgText, 40); Canvas::setTop(*bgText, 80);

        // Frosted glass panels
        struct BlurDemo { f32 radius; const char* label; Color tint; };
        BlurDemo demos[] = {
            {4.0f,  "blur=4",  c(255,255,255,30)},
            {12.0f, "blur=12", c(255,255,255,35)},
            {24.0f, "blur=24", c(200,220,255,45)},
        };
        f32 px = 10.0f;
        for (int i = 0; i < 3; i++) {
            auto glass = makeRef<Panel>();
            glass->setwidth(110); glass->setheight(60);
            glass->setcornerRadius(10);
            glass->setbackdropBlur(demos[i].radius);
            glass->setbackdropTint(demos[i].tint);
            glass->setborderColor(c(255,255,255,60));
            glass->setborderWidth(1.0f);
            glass->setisHitTestVisible(false);
            container->addChild(glass);
            Canvas::setLeft(*glass, px); Canvas::setTop(*glass, 55);

            auto lbl = makeRef<Text>(demos[i].label, 11.0f);
            lbl->setforeground(c(255,255,255,220));
            lbl->setisHitTestVisible(false);
            container->addChild(lbl);
            Canvas::setLeft(*lbl, px + 10); Canvas::setTop(*lbl, 80);

            px += 125.0f;
        }

        inner->addChild(container);
        card->addChild(inner);
        page->addChild(card);
    }

    return page;
}

// =========================================================================
// TAB 9 — Animations
// =========================================================================
inline gut::Ref<gut::Element> buildAnimationsTab(std::vector<gut::Ref<gut::FloatAnimation>>& outAnims) {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    auto page = makeRef<StackPanel>(Orientation::Horizontal);
    page->setmargin(Thickness{24, 24, 24, 24});
    page->setspacing(24.0f);

    // Cast bar card
    {
        auto card = makeCard(350.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Cast Bar Animation"));
        inner->addChild(makeLabel("Width animates 0 \xe2\x86\x92 full, easeInOutCubic"));

        auto castBg = makeRef<Panel>();
        castBg->setwidth(300); castBg->setheight(20);
        castBg->setcornerRadius(4); castBg->setbackground(c(40,30,10));
        castBg->setmargin(Thickness{0, 12, 0, 0});

        auto castFill = makeRef<Panel>();
        castFill->setwidth(0); castFill->setheight(20);
        castFill->setcornerRadius(4);
        castFill->setbackgroundGradientTop(c(255,245,120,230));
        castFill->setbackgroundGradientBottom(c(180,120,10,210));
        castBg->addChild(castFill);

        inner->addChild(castBg);

        auto anim = makeRef<FloatAnimation>();
        anim->setTargetProperty(&castFill->widthProperty());
        anim->setFrom(0.0f); anim->setTo(300.0f);
        anim->setduration(2500.0f);
        anim->setEasingFunction(easing::easeInOutCubic);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        outAnims.push_back(anim);

        card->addChild(inner);
        page->addChild(card);
    }

    // Pulse & Glow card
    {
        auto card = makeCard(280.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Pulse & Glow"));

        // Glow panel
        inner->addChild(makeLabel("Proc glow (easeInOutSine)"));
        auto glow = makeRef<Panel>();
        glow->setwidth(80); glow->setheight(80);
        glow->setbackground(c(255,255,100,80));
        glow->setcornerRadius(8); glow->setopacity(0.0f);
        glow->setmargin(Thickness{0, 8, 0, 0});
        inner->addChild(glow);
        {
            auto anim = makeRef<FloatAnimation>();
            anim->setTargetProperty(&glow->opacityProperty());
            anim->setFrom(0.0f); anim->setTo(1.0f);
            anim->setduration(600.0f);
            anim->setEasingFunction(easing::easeInOutSine);
            anim->setautoReverse(true);
            anim->setRepeatBehavior(RepeatBehavior::forever());
            anim->begin();
            outAnims.push_back(anim);
        }

        // Low HP pulse
        auto sep = makeRef<Panel>();
        sep->setheight(1); sep->setbackground(c(50,55,75));
        sep->setmargin(Thickness{0, 12, 0, 8});
        inner->addChild(sep);

        inner->addChild(makeLabel("Low HP pulse (easeInOutQuad)"));
        auto hpPanel = makeRef<Panel>();
        hpPanel->setwidth(120); hpPanel->setheight(30);
        hpPanel->setcornerRadius(4);
        hpPanel->setbackground(c(180,30,30));
        hpPanel->setmargin(Thickness{0, 8, 0, 0});
        auto hpText = makeRef<Text>("35% HP", 11.0f);
        hpText->setforeground(c(255,255,255));
        hpText->setmargin(Thickness{8, 6, 0, 0});
        hpPanel->addChild(hpText);
        inner->addChild(hpPanel);
        {
            auto anim = makeRef<FloatAnimation>();
            anim->setTargetProperty(&hpPanel->opacityProperty());
            anim->setFrom(1.0f); anim->setTo(0.4f);
            anim->setduration(800.0f);
            anim->setEasingFunction(easing::easeInOutQuad);
            anim->setautoReverse(true);
            anim->setRepeatBehavior(RepeatBehavior::forever());
            anim->begin();
            outAnims.push_back(anim);
        }

        card->addChild(inner);
        page->addChild(card);
    }

    // Fade & Breathe card
    {
        auto card = makeCard(280.0f);
        auto inner = makeRef<StackPanel>(Orientation::Vertical);
        inner->setmargin(Thickness{16, 16, 16, 16});
        inner->addChild(makeHeading("Fade & Breathe"));

        // Combat text staggered
        inner->addChild(makeLabel("Combat text (easeOutQuart)"));
        struct DmgText { const char* text; Color color; f32 size; };
        DmgText dmgTexts[] = {
            {"-4,832",  c(255,255,100), 16.0f},
            {"CRIT",    c(255,100,100), 12.0f},
            {"+2,150",  c(100,255,100), 13.0f},
            {"DODGE",   c(255,255,255), 10.0f},
        };
        auto dmgRow = makeRef<StackPanel>(Orientation::Horizontal);
        dmgRow->setspacing(12.0f); dmgRow->setmargin(Thickness{0, 8, 0, 0});
        for (int i = 0; i < 4; i++) {
            auto t = makeRef<Text>(dmgTexts[i].text, dmgTexts[i].size);
            t->setforeground(dmgTexts[i].color);
            dmgRow->addChild(t);

            auto anim = makeRef<FloatAnimation>();
            anim->setTargetProperty(&t->opacityProperty());
            anim->setFrom(1.0f); anim->setTo(0.0f);
            anim->setduration(2000.0f);
            anim->setEasingFunction(easing::easeOutQuart);
            anim->setbeginTime(static_cast<f32>(i) * 400.0f);
            anim->setautoReverse(true);
            anim->setRepeatBehavior(RepeatBehavior::forever());
            anim->begin();
            outAnims.push_back(anim);
        }
        inner->addChild(dmgRow);

        auto sep = makeRef<Panel>();
        sep->setheight(1); sep->setbackground(c(50,55,75));
        sep->setmargin(Thickness{0, 12, 0, 8});
        inner->addChild(sep);

        // Tooltip breathing
        inner->addChild(makeLabel("Tooltip breathing (easeInOutSine)"));
        auto tooltip = makeRef<Panel>();
        tooltip->setwidth(200); tooltip->setheight(60);
        tooltip->setcornerRadius(6);
        tooltip->setbackground(c(15,12,20,240));
        tooltip->setborderColor(c(120,100,50)); tooltip->setborderWidth(1);
        tooltip->setmargin(Thickness{0, 8, 0, 0});

        auto ttTitle = makeRef<Text>("Inferno Blast", 11.0f);
        ttTitle->setforeground(c(255,128,0));
        ttTitle->setmargin(Thickness{8, 6, 0, 0});
        tooltip->addChild(ttTitle);

        auto ttDesc = makeRef<Text>("1,420 to 1,780 Fire damage", 9.0f);
        ttDesc->setforeground(c(255,210,80));
        ttDesc->setmargin(Thickness{8, 24, 0, 0});
        tooltip->addChild(ttDesc);

        auto ttCd = makeRef<Text>("8 sec cooldown", 9.0f);
        ttCd->setforeground(c(255,255,255));
        ttCd->setmargin(Thickness{8, 40, 0, 0});
        tooltip->addChild(ttCd);

        inner->addChild(tooltip);

        {
            auto fadeIn = makeRef<FloatAnimation>();
            fadeIn->setTargetProperty(&tooltip->opacityProperty());
            fadeIn->setFrom(0.0f); fadeIn->setTo(1.0f);
            fadeIn->setduration(1500.0f);
            fadeIn->setEasingFunction(easing::easeOutExpo);
            fadeIn->setbeginTime(500.0f);
            fadeIn->begin();
            outAnims.push_back(fadeIn);

            auto breathe = makeRef<FloatAnimation>();
            breathe->setTargetProperty(&tooltip->opacityProperty());
            breathe->setFrom(0.85f); breathe->setTo(1.0f);
            breathe->setduration(2000.0f);
            breathe->setEasingFunction(easing::easeInOutSine);
            breathe->setbeginTime(2200.0f);
            breathe->setautoReverse(true);
            breathe->setRepeatBehavior(RepeatBehavior::forever());
            breathe->begin();
            outAnims.push_back(breathe);
        }

        card->addChild(inner);
        page->addChild(card);
    }

    return page;
}

// =========================================================================
// TAB 10 — Game UI (MMORPG HUD in a ViewBox)
// =========================================================================
inline gut::Ref<gut::Element> buildGameUITab(
    const GameTextures& tex,
    std::vector<gut::Ref<gut::FloatAnimation>>& outAnims)
{
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };
    const f32 W = 1024, H = 768;

    auto frame = [&](f32 w, f32 h, Color bg = Color::fromRgba8(20,20,30,220), f32 cr = 4.0f) {
        auto p = makeRef<Panel>();
        p->setwidth(w); p->setheight(h);
        p->setbackground(bg); p->setcornerRadius(cr);
        p->setborderColor(Color::fromRgba8(60,55,45,180));
        p->setborderWidth(1.0f);
        return p;
    };

    auto bar = [&](f32 w, f32 h, f32 fill, Color fillColor, Color bgColor, f32 cr = 3.0f,
                   Color gradTop = Color::transparent(), Color gradBot = Color::transparent()) {
        auto container = makeRef<Canvas>();
        container->setwidth(w); container->setheight(h);
        auto bg = makeRef<Panel>(); bg->setwidth(w); bg->setheight(h);
        bg->setbackground(bgColor); bg->setcornerRadius(cr);
        container->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);
        auto fg = makeRef<Panel>(); fg->setwidth(w * fill); fg->setheight(h);
        fg->setcornerRadius(cr);
        if (gradTop.a > 0 && gradBot.a > 0) {
            fg->setbackgroundGradientTop(gradTop);
            fg->setbackgroundGradientBottom(gradBot);
        } else { fg->setbackground(fillColor); }
        container->addChild(fg); Canvas::setLeft(*fg,0); Canvas::setTop(*fg,0);
        return container;
    };

    auto img = [&](Ref<Texture> t, f32 w, f32 h) {
        auto i = makeRef<Image>(t);
        i->setwidth(w); i->setheight(h);
        i->setstretch(Image::Stretch::Fill);
        return i;
    };

    auto root = makeRef<Canvas>();
    root->setwidth(W); root->setheight(H);
    root->setbackground(c(15,15,20));

    // --- Player Frame ---
    {
        auto pf = makeRef<Canvas>(); pf->setwidth(260); pf->setheight(70);
        root->addChild(pf); Canvas::setLeft(*pf,10); Canvas::setTop(*pf,10);

        auto bg = frame(260,70,c(25,22,18,230));
        bg->setbackgroundGradientTop(c(40,35,28,230));
        bg->setbackgroundGradientBottom(c(15,12,8,240));
        bg->setshadowColor(c(0,0,0,200)); bg->setshadowOffsetX(4); bg->setshadowOffsetY(6); bg->setshadowBlurRadius(12);
        pf->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);

        auto portrait = frame(50,50,c(80,60,45));
        portrait->setisHitTestVisible(false);
        pf->addChild(portrait); Canvas::setLeft(*portrait,8); Canvas::setTop(*portrait,10);
        auto face = img(tex.portrait,46,46); face->setisHitTestVisible(false);
        pf->addChild(face); Canvas::setLeft(*face,10); Canvas::setTop(*face,12);

        auto nameText = makeRef<Text>("Aelindra", 11.0f);
        nameText->setforeground(c(255,210,100)); nameText->setisHitTestVisible(false);
        pf->addChild(nameText); Canvas::setLeft(*nameText,70); Canvas::setTop(*nameText,7);

        auto lvlText = makeRef<Text>("60", 10.0f);
        lvlText->setforeground(c(255,220,80)); lvlText->setisHitTestVisible(false);
        pf->addChild(lvlText); Canvas::setLeft(*lvlText,236); Canvas::setTop(*lvlText,8);

        auto hp = bar(185,14,0.78f,c(180,30,30),c(60,15,15),3,c(255,100,80),c(120,10,5));
        hp->setisHitTestVisible(false);
        pf->addChild(hp); Canvas::setLeft(*hp,65); Canvas::setTop(*hp,25);
        auto hpText = makeRef<Text>("15,432 / 19,780", 9.0f);
        hpText->setforeground(c(255,255,255)); hpText->setisHitTestVisible(false);
        pf->addChild(hpText); Canvas::setLeft(*hpText,100); Canvas::setTop(*hpText,26);

        auto mp = bar(185,14,0.45f,c(30,60,180),c(15,20,60),3,c(100,160,255),c(10,30,120));
        mp->setisHitTestVisible(false);
        pf->addChild(mp); Canvas::setLeft(*mp,65); Canvas::setTop(*mp,42);
        auto mpText = makeRef<Text>("4,210 / 9,350", 9.0f);
        mpText->setforeground(c(255,255,255)); mpText->setisHitTestVisible(false);
        pf->addChild(mpText); Canvas::setLeft(*mpText,105); Canvas::setTop(*mpText,43);
    }

    // --- Target Frame ---
    {
        auto tf = makeRef<Canvas>(); tf->setwidth(260); tf->setheight(60);
        root->addChild(tf); Canvas::setLeft(*tf,10); Canvas::setTop(*tf,85);
        auto bg = frame(260,60,c(30,18,18,230));
        bg->setshadowColor(c(0,0,0,200)); bg->setshadowOffsetX(4); bg->setshadowOffsetY(6); bg->setshadowBlurRadius(12);
        tf->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);
        auto portrait = frame(42,42,c(90,30,30)); portrait->setisHitTestVisible(false);
        tf->addChild(portrait); Canvas::setLeft(*portrait,8); Canvas::setTop(*portrait,9);
        auto face = img(tex.boss,38,38); face->setisHitTestVisible(false);
        tf->addChild(face); Canvas::setLeft(*face,10); Canvas::setTop(*face,11);
        auto nameText = makeRef<Text>("Dreadlord Malachar", 10.0f);
        nameText->setforeground(c(255,100,100)); nameText->setisHitTestVisible(false);
        tf->addChild(nameText); Canvas::setLeft(*nameText,58); Canvas::setTop(*nameText,6);
        auto hp = bar(192,12,0.62f,c(200,40,40),c(60,15,15),3,c(255,100,80),c(120,10,5));
        hp->setisHitTestVisible(false);
        tf->addChild(hp); Canvas::setLeft(*hp,58); Canvas::setTop(*hp,22);
        auto hpText = makeRef<Text>("62%", 8.0f);
        hpText->setforeground(c(255,255,255)); hpText->setisHitTestVisible(false);
        tf->addChild(hpText); Canvas::setLeft(*hpText,140); Canvas::setTop(*hpText,23);
    }

    // --- Party Frames ---
    Ref<Canvas> brokkFrame;
    {
        f32 partyY = 160;
        struct PM { const char* name; f32 hp; f32 mp; Color nc; };
        PM members[] = {
            {"Thornguard",0.95f,0.30f,c(255,255,255)},
            {"Sylvaris",  0.60f,0.72f,c(150,220,255)},
            {"Brokk",     0.35f,0.55f,c(255,200,100)},
            {"Luminia",   0.88f,0.15f,c(200,255,200)},
        };
        int idx = 0;
        for (auto& m : members) {
            auto pf = makeRef<Canvas>(); pf->setwidth(140); pf->setheight(36);
            root->addChild(pf); Canvas::setLeft(*pf,10); Canvas::setTop(*pf,partyY);
            if (idx == 2) brokkFrame = pf;
            auto bg = frame(140,36,c(20,20,30,200));
            pf->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);
            auto nt = makeRef<Text>(m.name, 9.0f);
            nt->setforeground(m.nc); nt->setisHitTestVisible(false);
            pf->addChild(nt); Canvas::setLeft(*nt,5); Canvas::setTop(*nt,2);
            auto hpBar = bar(130,8,m.hp,c(40,160,40),c(20,50,20),3,c(100,240,100),c(20,100,20));
            hpBar->setisHitTestVisible(false);
            pf->addChild(hpBar); Canvas::setLeft(*hpBar,5); Canvas::setTop(*hpBar,15);
            auto mpBar = bar(130,6,m.mp,c(40,80,200),c(15,25,60),3,c(100,160,255),c(10,30,120));
            mpBar->setisHitTestVisible(false);
            pf->addChild(mpBar); Canvas::setLeft(*mpBar,5); Canvas::setTop(*mpBar,26);
            partyY += 40; idx++;
        }
    }

    // Low HP pulse
    if (brokkFrame) {
        auto anim = makeRef<FloatAnimation>();
        anim->setTargetProperty(&brokkFrame->opacityProperty());
        anim->setFrom(1.0f); anim->setTo(0.4f);
        anim->setduration(800.0f);
        anim->setEasingFunction(easing::easeInOutQuad);
        anim->setautoReverse(true);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        outAnims.push_back(anim);
    }

    // --- Minimap ---
    {
        auto mf = makeRef<Canvas>(); mf->setwidth(180); mf->setheight(200);
        root->addChild(mf); Canvas::setLeft(*mf,W-190); Canvas::setTop(*mf,10);
        auto outerBorder = frame(180,200,c(60,50,30));
        outerBorder->setshadowColor(c(0,0,0,220)); outerBorder->setshadowOffsetX(4);
        outerBorder->setshadowOffsetY(6); outerBorder->setshadowBlurRadius(14);
        mf->addChild(outerBorder); Canvas::setLeft(*outerBorder,0); Canvas::setTop(*outerBorder,0);
        auto mapImg = img(tex.minimap,170,170);
        mf->addChild(mapImg); Canvas::setLeft(*mapImg,5); Canvas::setTop(*mapImg,5);
        auto zoneText = makeRef<Text>("Shadowfang Keep", 9.0f);
        zoneText->setforeground(c(200,180,130));
        mf->addChild(zoneText); Canvas::setLeft(*zoneText,40); Canvas::setTop(*zoneText,178);
    }

    // --- Buff Bar ---
    {
        Ref<Texture> buffs[] = {tex.buffSword,tex.buffShield,tex.buffHaste,tex.buffRegen,
                                tex.buffMana,tex.buffFire,tex.buffShadow,tex.buffArmor};
        f32 bx = W - 190;
        for (int i = 0; i < 8; i++) {
            auto bi = img(buffs[i],20,20);
            root->addChild(bi); Canvas::setLeft(*bi,bx); Canvas::setTop(*bi,215);
            bx += 23;
        }
    }

    // --- Chat Window ---
    {
        auto cf = makeRef<Canvas>(); cf->setwidth(340); cf->setheight(180);
        root->addChild(cf); Canvas::setLeft(*cf,10); Canvas::setTop(*cf,H-195);
        auto bg = frame(340,180,c(10,10,15,200));
        bg->setshadowColor(c(0,0,0,180)); bg->setshadowOffsetX(3); bg->setshadowOffsetY(5); bg->setshadowBlurRadius(10);
        cf->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);

        struct ChatMsg { const char* text; Color color; };
        ChatMsg msgs[] = {
            {"[Thornguard]: Pulling boss in 5!",   c(255,180,100)},
            {"[Sylvaris]: Ready. Buffs up.",        c(150,220,255)},
            {"[Brokk]: LEEEEROOOY...",              c(255,200,100)},
            {"[Luminia]: Please don't.",             c(200,255,200)},
            {"Dreadlord casts Shadow Nova!",        c(255,80,80)},
            {"[Thornguard]: Interrupt!! NOW!",      c(255,180,100)},
            {"You interrupted Shadow Nova.",        c(255,255,100)},
            {"[Guild] gz on the kill guys",         c(100,255,100)},
            {"[Luminia]: Brokk is dead again...",   c(200,255,200)},
            {"[Brokk]: Worth it.",                  c(255,200,100)},
        };
        f32 msgY = 8;
        for (auto& m : msgs) {
            auto mt = makeRef<Text>(m.text, 9.0f);
            mt->setforeground(m.color);
            cf->addChild(mt); Canvas::setLeft(*mt,8); Canvas::setTop(*mt,msgY);
            msgY += 14;
        }
        auto inputText = makeRef<Text>("Say something...", 9.0f);
        inputText->setforeground(c(100,100,100));
        cf->addChild(inputText); Canvas::setLeft(*inputText,8); Canvas::setTop(*inputText,162);
    }

    // --- XP Bar ---
    {
        auto xpBg = frame(W-20,10,c(15,10,30));
        root->addChild(xpBg); Canvas::setLeft(*xpBg,10); Canvas::setTop(*xpBg,H-210);
        auto xpFill = frame((W-20)*0.67f,10,c(100,50,180));
        xpFill->setbackgroundGradientTop(c(180,120,255));
        xpFill->setbackgroundGradientBottom(c(60,20,130));
        root->addChild(xpFill); Canvas::setLeft(*xpFill,10); Canvas::setTop(*xpFill,H-210);
    }

    // --- Action Bar ---
    {
        Ref<Texture> icons[] = {
            tex.iconFireball, tex.iconFrost, tex.iconLightning, tex.iconHeal,
            tex.iconShield, tex.iconSword, tex.iconPotion, tex.iconSkull,
            tex.iconFireball, tex.iconFrost, tex.iconLightning, tex.iconHeal
        };
        f32 barWidth = 12 * 42;
        f32 barX = (W - barWidth) / 2;
        f32 barY = H - 55;

        auto abBg = frame(barWidth+16,50,c(20,18,15,230));
        abBg->setshadowColor(c(0,0,0,200)); abBg->setshadowOffsetY(6); abBg->setshadowBlurRadius(14);
        root->addChild(abBg); Canvas::setLeft(*abBg,barX-8); Canvas::setTop(*abBg,barY-4);

        const char* slotKeys[] = {"1","2","3","4","5","6","7","8","9","0","-","="};
        for (int i = 0; i < 12; i++) {
            f32 sx = barX + i * 42;
            auto slot = frame(38,38,c(30,28,22));
            slot->sethoverBackground(c(55,50,38));
            root->addChild(slot); Canvas::setLeft(*slot,sx); Canvas::setTop(*slot,barY);

            if (i < 10) {
                auto icon = img(icons[i],32,32); icon->setisHitTestVisible(false);
                root->addChild(icon); Canvas::setLeft(*icon,sx+3); Canvas::setTop(*icon,barY+3);
            }
            if (i == 2 || i == 5) {
                auto cd = frame(32,32,c(0,0,0,150)); cd->setisHitTestVisible(false);
                root->addChild(cd); Canvas::setLeft(*cd,sx+3); Canvas::setTop(*cd,barY+3);
                auto cdText = makeRef<Text>(i==2?"3.2":"12", 10.0f);
                cdText->setforeground(c(255,255,100)); cdText->setisHitTestVisible(false);
                root->addChild(cdText); Canvas::setLeft(*cdText,sx+12); Canvas::setTop(*cdText,barY+14);
            }
            if (i == 0 || i == 3) {
                auto glow = makeRef<Panel>();
                glow->setwidth(38); glow->setheight(38);
                glow->setbackground(c(255,255,100,80)); glow->setcornerRadius(4);
                glow->setopacity(0.0f); glow->setisHitTestVisible(false);
                root->addChild(glow); Canvas::setLeft(*glow,sx); Canvas::setTop(*glow,barY);
                auto anim = makeRef<FloatAnimation>();
                anim->setTargetProperty(&glow->opacityProperty());
                anim->setFrom(0.0f); anim->setTo(1.0f);
                anim->setduration(600.0f);
                anim->setEasingFunction(easing::easeInOutSine);
                anim->setautoReverse(true);
                anim->setRepeatBehavior(RepeatBehavior::forever());
                anim->begin();
                outAnims.push_back(anim);
            }
            auto keyText = makeRef<Text>(slotKeys[i], 8.0f);
            keyText->setforeground(c(200,200,200,180)); keyText->setisHitTestVisible(false);
            root->addChild(keyText); Canvas::setLeft(*keyText,sx+2); Canvas::setTop(*keyText,barY+1);
        }
    }

    // --- Casting Bar ---
    {
        f32 castW = 280;
        auto castFrame = makeRef<Canvas>(); castFrame->setwidth(castW); castFrame->setheight(28);
        root->addChild(castFrame); Canvas::setLeft(*castFrame,(W-castW)/2); Canvas::setTop(*castFrame,H/2+80);
        auto bg = frame(castW,28,c(15,15,20,220));
        bg->setshadowColor(c(0,0,0,180)); bg->setshadowOffsetY(5); bg->setshadowBlurRadius(10);
        castFrame->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);
        auto castBg2 = frame(castW-8,16,c(40,30,10));
        castFrame->addChild(castBg2); Canvas::setLeft(*castBg2,4); Canvas::setTop(*castBg2,4);

        auto castBarFill = makeRef<Panel>();
        castBarFill->setwidth(0); castBarFill->setheight(16); castBarFill->setcornerRadius(3);
        castBarFill->setbackgroundGradientTop(c(255,245,120,230));
        castBarFill->setbackgroundGradientBottom(c(180,120,10,210));
        castFrame->addChild(castBarFill); Canvas::setLeft(*castBarFill,4); Canvas::setTop(*castBarFill,4);

        auto castText = makeRef<Text>("Fireball", 10.0f);
        castText->setforeground(c(255,255,255));
        castFrame->addChild(castText); Canvas::setLeft(*castText,castW/2-20); Canvas::setTop(*castText,6);

        auto anim = makeRef<FloatAnimation>();
        anim->setTargetProperty(&castBarFill->widthProperty());
        anim->setFrom(0.0f); anim->setTo(272.0f);
        anim->setduration(2500.0f);
        anim->setEasingFunction(easing::easeInOutCubic);
        anim->setRepeatBehavior(RepeatBehavior::forever());
        anim->begin();
        outAnims.push_back(anim);
    }

    // --- Combat Text ---
    {
        struct DmgText { const char* text; f32 x; f32 y; Color color; f32 size; };
        DmgText dmg[] = {
            {"-4,832",480,200,c(255,255,100),16},{"-1,204",520,250,c(255,255,255),12},
            {"+2,150",350,220,c(100,255,100),13},{"DODGE",400,310,c(255,255,255),10},
            {"-3,621",550,230,c(255,200,50),14},
        };
        for (int i = 0; i < 5; i++) {
            auto t = makeRef<Text>(dmg[i].text, dmg[i].size);
            t->setforeground(dmg[i].color);
            root->addChild(t); Canvas::setLeft(*t,dmg[i].x); Canvas::setTop(*t,dmg[i].y);
            auto anim = makeRef<FloatAnimation>();
            anim->setTargetProperty(&t->opacityProperty());
            anim->setFrom(1.0f); anim->setTo(0.0f);
            anim->setduration(2000.0f);
            anim->setEasingFunction(easing::easeOutQuart);
            anim->setbeginTime(static_cast<f32>(i) * 400.0f);
            anim->setautoReverse(true);
            anim->setRepeatBehavior(RepeatBehavior::forever());
            anim->begin();
            outAnims.push_back(anim);
        }
    }

    // --- Boss Health Bar ---
    {
        f32 bossW = 350;
        auto bf = makeRef<Canvas>(); bf->setwidth(bossW); bf->setheight(32);
        root->addChild(bf); Canvas::setLeft(*bf,(W-bossW)/2); Canvas::setTop(*bf,10);
        auto bg = frame(bossW,32,c(25,15,15,230));
        bg->setshadowColor(c(40,0,0,200)); bg->setshadowOffsetY(5); bg->setshadowBlurRadius(12);
        bf->addChild(bg); Canvas::setLeft(*bg,0); Canvas::setTop(*bg,0);
        auto hpBg = frame(bossW-10,16,c(50,15,15));
        bf->addChild(hpBg); Canvas::setLeft(*hpBg,5); Canvas::setTop(*hpBg,12);
        auto hpFill = frame((bossW-10)*0.62f,16,c(180,25,25));
        bf->addChild(hpFill); Canvas::setLeft(*hpFill,5); Canvas::setTop(*hpFill,12);
        auto bossName = makeRef<Text>("Dreadlord Malachar \xe2\x80\x94 Phase 2", 10.0f);
        bossName->setforeground(c(255,100,100));
        bf->addChild(bossName); Canvas::setLeft(*bossName,70); Canvas::setTop(*bossName,0);
        auto bossHp = makeRef<Text>("62%", 10.0f);
        bossHp->setforeground(c(255,255,255));
        bf->addChild(bossHp); Canvas::setLeft(*bossHp,bossW/2-10); Canvas::setTop(*bossHp,13);
    }

    // Wrap in ViewBox so it scales to tab area
    auto vb = makeRef<ViewBox>();
    vb->setstretch(ViewBoxStretch::Uniform);
    vb->addChild(root);
    return vb;
}

// =========================================================================
// buildDemoUI() — master builder
// =========================================================================
inline DemoResult buildDemoUI(gut::Context& ctx, const DemoConfig& cfg) {
    using namespace gut;
    auto c = [](gut::u8 r, gut::u8 g, gut::u8 b, gut::u8 a = 255) { return cc(r, g, b, a); };

    DemoResult result;

    // Create textures
    auto textures = createGameTextures(ctx.renderBackend());

    // Root: DockPanel (title top, status bottom, tabs fill)
    auto root = makeRef<DockPanel>();
    root->setwidth(static_cast<f32>(cfg.width));
    root->setheight(static_cast<f32>(cfg.height));
    root->setbackground(c(20, 22, 30));
    root->setlastChildFill(true);

    // Title bar
    auto titleBar = makeRef<Panel>();
    titleBar->setheight(52.0f);
    titleBar->setbackground(c(25, 27, 38));
    DockPanel::setDock(*titleBar, Dock::Top);
    auto titleText = makeRef<Text>();
    titleText->settext((std::string("Gut \xe2\x80\x94 ") + cfg.backendShort + " Demo").c_str());
    titleText->setfontSize(22.0f);
    titleText->setfontWeight(FontWeight::Bold);
    titleText->setforeground(c(230, 235, 255));
    titleText->setmargin(Thickness{20, 14, 0, 0});
    titleBar->addChild(titleText);
    auto versionText = makeRef<Text>();
    versionText->settext((std::string("v1.0.0  |  ") + cfg.backendName).c_str());
    versionText->setfontSize(12.0f);
    versionText->setforeground(c(100, 110, 140));
    versionText->setmargin(Thickness{20, 34, 0, 0});
    titleBar->addChild(versionText);
    root->addChild(titleBar);

    // Status bar
    auto statusBar = makeRef<Panel>();
    statusBar->setheight(28.0f);
    statusBar->setbackground(c(25, 27, 35));
    DockPanel::setDock(*statusBar, Dock::Bottom);
    auto statusText = makeRef<Text>();
    statusText->settext((std::string("Gut UI Toolkit \xe2\x80\x94 ") + cfg.backendName + " backend").c_str());
    statusText->setfontSize(11.0f);
    statusText->setforeground(c(100, 105, 125));
    statusText->setmargin(Thickness{12, 6, 0, 0});
    statusBar->addChild(statusText);
    root->addChild(statusBar);

    // TabControl
    auto tabs = makeRef<TabControl>();
    tabs->settabBarBackground(c(28, 30, 42));
    tabs->settabBarBorderColor(c(45, 48, 65));
    tabs->settabForeground(c(140, 145, 165));
    tabs->settabActiveForeground(c(230, 235, 255));
    tabs->settabActiveIndicator(c(80, 150, 240));
    tabs->settabHoverBackground(c(40, 42, 58, 150));
    tabs->setcontentBackground(c(20, 22, 30));
    tabs->settabBarHeight(36.0f);
    tabs->settabFontSize(13.0f);
    tabs->settabPadding(20.0f);

    tabs->addTab("Controls",       buildControlsTab(cfg));
    tabs->addTab("Text & Fonts",   buildTextFontsTab());
    tabs->addTab("Images & Effects", buildImagesEffectsTab(textures));
    tabs->addTab("Layouts",        buildLayoutsTab());
    tabs->addTab("Layouts 2",      buildLayouts2Tab());
    tabs->addTab("Drawing",        buildDrawingTab());
    tabs->addTab("Transforms",     buildTransformsTab());
    tabs->addTab("Clipping & Blur", buildClippingBlurTab());
    tabs->addTab("Animations",     buildAnimationsTab(result.animations));
    tabs->addTab("Game UI",        buildGameUITab(textures, result.animations));

    root->addChild(tabs);

    result.root = root;
    return result;
}

} // namespace demo
