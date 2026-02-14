/**
 * @file d3d11_demo.cpp
 * @brief Win32 + D3D11 demo for Gut UI Toolkit
 *
 * Build:
 *   cl /std:c++20 /EHsc /I.. d3d11_demo.cpp /link d3d11.lib d3dcompiler.lib dxgi.lib user32.lib
 *
 * Or use CMake (see examples/CMakeLists.txt).
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Include backend (defines GUT_IMPLEMENTATION)
#include "d3d11_backend.h"
#include "d3d11_backend.cpp"

#include <memory>
#include <chrono>

// ============================================================================
// Globals
// ============================================================================

static ComPtr<ID3D11Device>        g_device;
static ComPtr<ID3D11DeviceContext>  g_context;
static ComPtr<IDXGISwapChain>      g_swapChain;

static std::unique_ptr<gut::Context> g_gutCtx;
static bool g_running = true;
static UINT g_width = 1100;
static UINT g_height = 720;
static float g_dpi = 1.0f;

// ============================================================================
// D3D11 Init
// ============================================================================

static bool InitD3D11(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = g_width;
    scd.BufferDesc.Height = g_height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &g_swapChain, &g_device, &featureLevel, &g_context);

    return SUCCEEDED(hr);
}

// ============================================================================
// Build Gut UI
// ============================================================================

static void BuildUI() {
    using namespace gut;

    auto backend = std::make_unique<D3D11RenderBackend>(
        g_device.Get(), g_context.Get(), g_swapChain.Get());

    g_gutCtx = std::make_unique<Context>(std::move(backend));

    // Load system fonts — multiple weights for font weight/style demo
    {
        struct FontFile { const char* path; } fontFiles[] = {
            {"C:\\Windows\\Fonts\\segoeuil.ttf"},   // Light (300)
            {"C:\\Windows\\Fonts\\segoeui.ttf"},    // Regular (400)
            {"C:\\Windows\\Fonts\\segoeuib.ttf"},   // Bold (700)
            {"C:\\Windows\\Fonts\\segoeuii.ttf"},   // Italic
            {"C:\\Windows\\Fonts\\segoeuiz.ttf"},   // Bold Italic
        };
        for (auto& ff : fontFiles) {
            FILE* f = fopen(ff.path, "rb");
            if (!f) continue;
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<u8> data(sz);
            fread(data.data(), 1, sz, f);
            fclose(f);
            g_gutCtx->loadFont(data.data(), data.size());
        }
    }

    auto c = [](u8 r, u8 g, u8 b, u8 a = 255) { return Color::fromRgba8(r, g, b, a); };

    // =====================================================================
    // Root layout: DockPanel (title top, status bottom, tabs fill)
    // =====================================================================
    auto root = makeRef<DockPanel>();
    root->setwidth(static_cast<f32>(g_width));
    root->setheight(static_cast<f32>(g_height));
    root->setbackground(c(20, 22, 30));
    root->setlastChildFill(true);

    // --- Title bar ---
    auto titleBar = makeRef<Panel>();
    titleBar->setheight(52.0f);
    titleBar->setbackground(c(25, 27, 38));
    DockPanel::setDock(*titleBar, Dock::Top);

    auto titleText = makeRef<Text>();
    titleText->settext("Gut \xe2\x80\x94 D3D11 Demo");
    titleText->setfontSize(22.0f);
    titleText->setfontWeight(FontWeight::Bold);
    titleText->setforeground(c(230, 235, 255));
    titleText->setmargin(Thickness{20, 14, 0, 0});
    titleBar->addChild(titleText);

    auto versionText = makeRef<Text>();
    versionText->settext("v1.0.0  |  D3D11  |  Segoe UI");
    versionText->setfontSize(12.0f);
    versionText->setforeground(c(100, 110, 140));
    versionText->setmargin(Thickness{20, 34, 0, 0});
    titleBar->addChild(versionText);

    root->addChild(titleBar);

    // --- Status bar ---
    auto statusBar = makeRef<Panel>();
    statusBar->setheight(28.0f);
    statusBar->setbackground(c(25, 27, 35));
    DockPanel::setDock(*statusBar, Dock::Bottom);

    auto statusText = makeRef<Text>();
    statusText->settext("Gut UI Toolkit \xe2\x80\x94 D3D11 backend \xe2\x80\x94 Windows");
    statusText->setfontSize(11.0f);
    statusText->setforeground(c(100, 105, 125));
    statusText->setmargin(Thickness{12, 6, 0, 0});
    statusBar->addChild(statusText);
    root->addChild(statusBar);

    // =====================================================================
    // TabControl (fills remaining space)
    // =====================================================================
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

    // Helper: make a section card
    auto makeCard = [&](f32 w = 0) {
        auto card = makeRef<Panel>();
        if (w > 0) card->setwidth(w);
        card->setbackground(c(32, 35, 48));
        card->setcornerRadius(8.0f);
        card->setborderColor(c(50, 55, 75));
        card->setborderWidth(1.0f);
        return card;
    };

    auto makeHeading = [&](const char* text) {
        auto h = makeRef<Text>();
        h->settext(text);
        h->setfontSize(15.0f);
        h->setfontWeight(FontWeight::Bold);
        h->setforeground(c(200, 205, 220));
        h->setmargin(Thickness{0, 0, 0, 10});
        return h;
    };

    auto makeLabel = [&](const char* text, f32 size = 13.0f) {
        auto lbl = makeRef<Text>();
        lbl->settext(text);
        lbl->setfontSize(size);
        lbl->setforeground(c(160, 165, 185));
        return lbl;
    };

    // =================================================================
    // TAB 1 — Controls
    // =================================================================
    {
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

            auto makeButton = [&](const char* text) {
                auto btn = makeRef<Button>();
                btn->setlabel(text);
                btn->setwidth(220.0f);
                btn->setheight(34.0f);
                btn->setmargin(Thickness{0, 0, 0, 6});
                return btn;
            };
            inner->addChild(makeButton("Primary Action"));
            inner->addChild(makeButton("Secondary"));
            inner->addChild(makeButton("Settings"));
            inner->addChild(makeButton("About"));
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
            addRow("Backend:", "Direct3D 11");
            addRow("Shader Model:", "5.0");
            addRow("Resolution:", (std::to_string(g_width) + "x" + std::to_string(g_height)).c_str());
            addRow("Features:", "SDF AA, Rounded clip");
            addRow("Blur:", "Multi-pass Kawase");

            card->addChild(inner);
            col3->addChild(card);
        }
        page->addChild(col3);

        tabs->addTab("Controls", page);
    }

    // =================================================================
    // TAB 2 — Text & Fonts
    // =================================================================
    {
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

            // Convenience API demos
            auto sep = makeRef<Panel>();
            sep->setheight(1.0f);
            sep->setbackground(c(50, 55, 75));
            sep->setmargin(Thickness{0, 12, 0, 12});
            inner->addChild(sep);

            auto b = makeRef<Text>();
            b->settext("bold(true) convenience  \xe2\x86\x92  picks Bold weight");
            b->setfontSize(14.0f);
            b->setbold(true);
            b->setforeground(c(180, 200, 140));
            inner->addChild(b);

            auto it = makeRef<Text>();
            it->settext("italic(true) convenience  \xe2\x86\x92  picks Italic style");
            it->setfontSize(14.0f);
            it->setitalic(true);
            it->setforeground(c(140, 180, 200));
            it->setmargin(Thickness{0, 6, 0, 0});
            inner->addChild(it);

            card->addChild(inner);
            page->addChild(card);
        }

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
            page->addChild(card);
        }

        tabs->addTab("Text & Fonts", page);
    }

    // =================================================================
    // TAB 3 — Images
    // =================================================================
    {
        auto page = makeRef<StackPanel>(Orientation::Horizontal);
        page->setmargin(Thickness{24, 24, 24, 24});
        page->setspacing(24.0f);

        // Generate procedural 9-slice texture
        Ref<Texture> nineSliceTex;
        {
            constexpr int S = 48, R = 10, B = 4;
            std::vector<u8> pixels(S * S * 4, 0);
            for (int y = 0; y < S; ++y)
            for (int x = 0; x < S; ++x) {
                float cx = (float)x - S * 0.5f + 0.5f;
                float cy = (float)y - S * 0.5f + 0.5f;
                float hx = S * 0.5f - (float)R;
                float hy = S * 0.5f - (float)R;
                float dx = std::max(std::abs(cx) - hx, 0.0f);
                float dy = std::max(std::abs(cy) - hy, 0.0f);
                float dist = std::sqrtf(dx*dx + dy*dy) - (float)R;
                u8* p = &pixels[(y * S + x) * 4];
                if      (dist < -B)   { p[0]=60; p[1]=70;  p[2]=100; p[3]=255; }
                else if (dist < 0.5f) { p[0]=100;p[1]=160; p[2]=255; p[3]=255; }
                else                  { p[0]=0;  p[1]=0;   p[2]=0;   p[3]=0;   }
            }
            nineSliceTex = g_gutCtx->renderBackend().createTexture(S, S, pixels.data());
        }
        Thickness borders{12, 12, 12, 12};

        // 9-Slice card
        {
            auto card = makeCard(480.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{20, 20, 20, 20});
            inner->addChild(makeHeading("9-Slice Image Rendering"));

            inner->addChild(makeLabel("9-slice: corners stay crisp, edges stretch"));

            auto row1 = makeRef<StackPanel>(Orientation::Horizontal);
            row1->setmargin(Thickness{0, 12, 0, 0});
            row1->setspacing(12.0f);

            for (auto [w, h, label] : std::initializer_list<std::tuple<f32,f32,const char*>>{
                    {48.f, 48.f, "48x48"}, {140.f, 48.f, "140x48"}, {48.f, 90.f, "48x90"}, {200.f, 70.f, "200x70"}}) {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto img = makeRef<Image>(nineSliceTex);
                img->setwidth(w); img->setheight(h);
                img->setstretch(Image::Stretch::NineSlice);
                img->setsliceBorders(borders);
                col->addChild(img);
                auto lbl = makeLabel(label, 10.0f);
                lbl->setmargin(Thickness{0, 2, 0, 0});
                col->addChild(lbl);
                row1->addChild(col);
            }
            inner->addChild(row1);

            // Fill vs 9-slice comparison
            auto sep = makeRef<Panel>();
            sep->setheight(1.0f);
            sep->setbackground(c(50, 55, 75));
            sep->setmargin(Thickness{0, 16, 0, 12});
            inner->addChild(sep);

            inner->addChild(makeLabel("Comparison: Fill vs 9-Slice at 180x50"));
            auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
            row2->setmargin(Thickness{0, 8, 0, 0});
            row2->setspacing(16.0f);

            auto fillCol = makeRef<StackPanel>(Orientation::Vertical);
            auto imgFill = makeRef<Image>(nineSliceTex);
            imgFill->setwidth(180.0f); imgFill->setheight(50.0f);
            imgFill->setstretch(Image::Stretch::Fill);
            fillCol->addChild(imgFill);
            fillCol->addChild(makeLabel("Fill (distorted)", 10.0f));
            row2->addChild(fillCol);

            auto nsCol = makeRef<StackPanel>(Orientation::Vertical);
            auto imgNS = makeRef<Image>(nineSliceTex);
            imgNS->setwidth(180.0f); imgNS->setheight(50.0f);
            imgNS->setstretch(Image::Stretch::NineSlice);
            imgNS->setsliceBorders(borders);
            nsCol->addChild(imgNS);
            nsCol->addChild(makeLabel("9-Slice (crisp)", 10.0f));
            row2->addChild(nsCol);

            inner->addChild(row2);

            card->addChild(inner);
            page->addChild(card);
        }

        // Radial Gradients card
        {
            auto card = makeCard(420.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{20, 20, 20, 20});
            inner->addChild(makeHeading("Radial Gradients"));
            inner->addChild(makeLabel("Per-vertex radial colour via RadialGradientBrush"));

            auto row = makeRef<StackPanel>(Orientation::Horizontal);
            row->setspacing(14.0f);
            row->setmargin(Thickness{0, 12, 0, 0});

            // 1) Simple radial — centre-out, white → blue
            {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto box = makeRef<Panel>();
                box->setwidth(100.0f); box->setheight(100.0f);
                box->setcornerRadius(8.0f);
                auto brush = makeRef<RadialGradientBrush>(
                    Point2f{0.5f, 0.5f}, 0.5f,
                    std::vector<GradientStop>{{0.0f, c(255,255,255)}, {1.0f, c(40,80,200)}});
                box->setbackgroundBrush(brush);
                col->addChild(box);
                col->addChild(makeLabel("Centre-out", 10.0f));
                row->addChild(col);
            }

            // 2) Sunset — 3 stops
            {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto box = makeRef<Panel>();
                box->setwidth(100.0f); box->setheight(100.0f);
                box->setcornerRadius(50.0f);
                auto brush = makeRef<RadialGradientBrush>(
                    Point2f{0.5f, 0.5f}, 0.5f,
                    std::vector<GradientStop>{
                        {0.0f, c(255,220,80)},
                        {0.5f, c(255,100,40)},
                        {1.0f, c(80,20,100)}});
                box->setbackgroundBrush(brush);
                col->addChild(box);
                col->addChild(makeLabel("Sunset (3-stop)", 10.0f));
                row->addChild(col);
            }

            // 3) Off-centre highlight
            {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto box = makeRef<Panel>();
                box->setwidth(100.0f); box->setheight(100.0f);
                box->setcornerRadius(8.0f);
                auto brush = makeRef<RadialGradientBrush>();
                brush->setCenter({0.3f, 0.3f});
                brush->setGradientOrigin({0.3f, 0.3f});
                brush->setRadiusX(0.6f);
                brush->setRadiusY(0.6f);
                brush->setStops({{0.0f, c(255,255,255)}, {0.4f, c(60,180,100)}, {1.0f, c(20,60,30)}});
                box->setbackgroundBrush(brush);
                col->addChild(box);
                col->addChild(makeLabel("Off-centre", 10.0f));
                row->addChild(col);
            }

            inner->addChild(row);

            // Second row — rectangular stretch
            auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
            row2->setspacing(14.0f);
            row2->setmargin(Thickness{0, 14, 0, 0});

            {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto box = makeRef<Panel>();
                box->setwidth(200.0f); box->setheight(60.0f);
                box->setcornerRadius(6.0f);
                auto brush = makeRef<RadialGradientBrush>(
                    Point2f{0.5f, 0.5f}, 0.5f,
                    std::vector<GradientStop>{{0.0f, c(220,60,200)}, {1.0f, c(30,30,60)}});
                box->setbackgroundBrush(brush);
                col->addChild(box);
                col->addChild(makeLabel("Wide rect", 10.0f));
                row2->addChild(col);
            }

            {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto box = makeRef<Panel>();
                box->setwidth(60.0f); box->setheight(120.0f);
                box->setcornerRadius(6.0f);
                auto brush = makeRef<RadialGradientBrush>(
                    Point2f{0.5f, 0.5f}, 0.5f,
                    std::vector<GradientStop>{{0.0f, c(255,200,60)}, {1.0f, c(60,30,10)}});
                box->setbackgroundBrush(brush);
                col->addChild(box);
                col->addChild(makeLabel("Tall rect", 10.0f));
                row2->addChild(col);
            }

            inner->addChild(row2);

            card->addChild(inner);
            page->addChild(card);
        }

        tabs->addTab("Images", page);
    }

    // =================================================================
    // TAB 4 — Layouts
    // =================================================================
    {
        auto page = makeRef<StackPanel>(Orientation::Horizontal);
        page->setmargin(Thickness{24, 24, 24, 24});
        page->setspacing(20.0f);

        // --- Column 1: WrapPanel + DockPanel ---
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

            const char* tags[] = {"C++", "Rust", "Go", "Python", "TypeScript",
                                  "Zig", "Kotlin", "Swift", "Haskell", "Lua"};
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
            dock->setwidth(300.0f);
            dock->setheight(150.0f);
            dock->setlastChildFill(true);
            dock->setmargin(Thickness{0, 10, 0, 0});

            auto topBar = makeRef<Panel>();
            topBar->setheight(26.0f);
            topBar->setbackground(c(60,130,200));
            DockPanel::setDock(*topBar, Dock::Top);
            auto topT = makeRef<Text>();
            topT->settext("Toolbar"); topT->setfontSize(11.0f);
            topT->setforeground(c(255,255,255));
            topT->setmargin(Thickness{8, 5, 0, 0});
            topBar->addChild(topT);
            dock->addChild(topBar);

            auto botBar = makeRef<Panel>();
            botBar->setheight(20.0f);
            botBar->setbackground(c(50,55,70));
            DockPanel::setDock(*botBar, Dock::Bottom);
            auto botT = makeRef<Text>();
            botT->settext("Status"); botT->setfontSize(10.0f);
            botT->setforeground(c(150,155,175));
            botT->setmargin(Thickness{8, 3, 0, 0});
            botBar->addChild(botT);
            dock->addChild(botBar);

            auto leftBar = makeRef<Panel>();
            leftBar->setwidth(60.0f);
            leftBar->setbackground(c(40,44,60));
            DockPanel::setDock(*leftBar, Dock::Left);
            auto leftT = makeRef<Text>();
            leftT->settext("Nav"); leftT->setfontSize(11.0f);
            leftT->setforeground(c(170,175,195));
            leftT->setmargin(Thickness{8, 8, 0, 0});
            leftBar->addChild(leftT);
            dock->addChild(leftBar);

            auto center = makeRef<Panel>();
            center->setbackground(c(28,30,42));
            auto centerT = makeRef<Text>();
            centerT->settext("Content"); centerT->setfontSize(13.0f);
            centerT->setforeground(c(200,205,220));
            centerT->setmargin(Thickness{14, 14, 0, 0});
            center->addChild(centerT);
            dock->addChild(center);

            inner->addChild(dock);
            card->addChild(inner);
            col1->addChild(card);
        }
        page->addChild(col1);

        // --- Column 2: ViewBox + AnchorPanel ---
        auto col2 = makeRef<StackPanel>(Orientation::Vertical);
        col2->setspacing(20.0f);

        { // ViewBox card
            auto card = makeCard(340.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{16, 16, 16, 16});
            inner->addChild(makeHeading("ViewBox"));
            inner->addChild(makeLabel("Scales child to fit container"));

            auto row = makeRef<StackPanel>(Orientation::Horizontal);
            row->setspacing(10.0f);
            row->setmargin(Thickness{0, 10, 0, 0});

            auto makeVB = [&](f32 w, f32 h, const char* label) {
                auto col = makeRef<StackPanel>(Orientation::Vertical);
                auto vb = makeRef<ViewBox>();
                vb->setwidth(w); vb->setheight(h);
                vb->setstretch(ViewBoxStretch::Uniform);
                vb->setborderColor(c(55, 60, 80));
                vb->setborderWidth(1.0f);

                auto content = makeRef<StackPanel>(Orientation::Vertical);
                content->setwidth(100.0f);
                content->setheight(60.0f);
                content->setbackground(c(50, 80, 120));
                content->setcornerRadius(4.0f);
                auto t1 = makeRef<Text>();
                t1->settext("Hello"); t1->setfontSize(20.0f);
                t1->setforeground(c(255,255,255));
                t1->setmargin(Thickness{8, 4, 0, 0});
                content->addChild(t1);
                auto t2 = makeRef<Text>();
                t2->settext("World!"); t2->setfontSize(12.0f);
                t2->setforeground(c(200,220,255));
                t2->setmargin(Thickness{8, 0, 0, 0});
                content->addChild(t2);
                vb->addChild(content);
                col->addChild(vb);

                auto lbl = makeLabel(label, 10.0f);
                lbl->setmargin(Thickness{0, 2, 0, 0});
                col->addChild(lbl);
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
            inner->addChild(makeLabel("Position with proportional anchors + offsets"));

            auto anchor = makeRef<AnchorPanel>();
            anchor->setwidth(300.0f);
            anchor->setheight(170.0f);
            anchor->setbackground(c(24, 26, 36));
            anchor->setcornerRadius(4.0f);
            anchor->setborderColor(c(45, 50, 70));
            anchor->setborderWidth(1.0f);
            anchor->setmargin(Thickness{0, 10, 0, 0});

            auto makeChip = [&](const char* label, Color bg) {
                auto p = makeRef<Panel>();
                p->setbackground(bg);
                p->setcornerRadius(4.0f);
                auto t = makeRef<Text>();
                t->settext(label); t->setfontSize(11.0f);
                t->setforeground(c(255,255,255));
                t->setmargin(Thickness{8, 3, 8, 3});
                p->addChild(t);
                return p;
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

            // Stretch bar
            auto bar = makeRef<Panel>();
            bar->setheight(28.0f);
            bar->setbackground(c(150, 80, 200));
            bar->setcornerRadius(4.0f);
            AnchorPanel::setAnchorLeft(*bar, 0.15f);
            AnchorPanel::setAnchorRight(*bar, 0.85f);
            AnchorPanel::setAnchorTop(*bar, 0.5f);
            AnchorPanel::setOffsetLeft(*bar, 0);
            AnchorPanel::setOffsetRight(*bar, 0);
            AnchorPanel::setOffsetTop(*bar, -14);
            auto barT = makeRef<Text>();
            barT->settext("15%..85% stretch"); barT->setfontSize(11.0f);
            barT->setforeground(c(255,255,255));
            barT->setmargin(Thickness{8, 5, 0, 0});
            bar->addChild(barT);
            anchor->addChild(bar);

            inner->addChild(anchor);
            card->addChild(inner);
            col2->addChild(card);
        }
        page->addChild(col2);

        tabs->addTab("Layouts", page);
    }

    // =================================================================
    // TAB 5 — Layouts 2 (UniformGrid, FlexPanel, RelativePanel, RadialPanel)
    // =================================================================
    {
        auto page = makeRef<StackPanel>(Orientation::Horizontal);
        page->setmargin(Thickness{24, 24, 24, 24});
        page->setspacing(20.0f);

        // --- Column 1: UniformGrid + FlexPanel ---
        auto col1 = makeRef<StackPanel>(Orientation::Vertical);
        col1->setspacing(20.0f);

        { // UniformGrid card
            auto card = makeCard(340.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{16, 16, 16, 16});
            inner->addChild(makeHeading("UniformGrid"));
            inner->addChild(makeLabel("All cells are equal size (3 columns)"));

            auto grid = makeRef<UniformGrid>();
            grid->setcolumns(3);
            grid->setwidth(300.0f);
            grid->setheight(200.0f);
            grid->setmargin(Thickness{0, 10, 0, 0});

            Color cellColors[] = {
                c(80,140,220), c(200,100,60), c(60,180,100),
                c(220,160,40), c(150,80,200), c(50,160,180),
                c(200,60,100), c(100,180,60), c(180,120,60)
            };
            for (int i = 0; i < 9; ++i) {
                auto cell = makeRef<Panel>();
                cell->setbackground(cellColors[i]);
                cell->setmargin(Thickness{2, 2, 2, 2});
                cell->setcornerRadius(4.0f);
                auto lbl = makeRef<Text>();
                lbl->settext(std::to_string(i + 1));
                lbl->setfontSize(16.0f);
                lbl->setforeground(c(255,255,255));
                lbl->setmargin(Thickness{8, 8, 0, 0});
                cell->addChild(lbl);
                grid->addChild(cell);
            }
            inner->addChild(grid);
            card->addChild(inner);
            col1->addChild(card);
        }
        { // FlexPanel card
            auto card = makeCard(340.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{16, 16, 16, 16});
            inner->addChild(makeHeading("FlexPanel"));
            inner->addChild(makeLabel("CSS Flexbox: grow=1, spaceEvenly"));

            auto flex = makeRef<FlexPanel>(Orientation::Horizontal);
            flex->setwidth(300.0f);
            flex->setheight(60.0f);
            flex->setspacing(6.0f);
            flex->setjustifyContent(FlexJustify::SpaceEvenly);
            flex->setalignItems(FlexAlign::Center);
            flex->setmargin(Thickness{0, 10, 0, 0});
            flex->setbackground(c(24, 26, 36));
            flex->setcornerRadius(4.0f);

            Color flexColors[] = {c(80,140,220), c(200,100,60), c(60,180,100)};
            const char* flexLabels[] = {"A", "B", "C"};
            for (int i = 0; i < 3; ++i) {
                auto box = makeRef<Panel>();
                box->setwidth(60.0f);
                box->setheight(36.0f);
                box->setbackground(flexColors[i]);
                box->setcornerRadius(6.0f);
                FlexPanel::setFlexGrow(*box, 1.0f);
                auto lbl = makeRef<Text>();
                lbl->settext(flexLabels[i]);
                lbl->setfontSize(14.0f);
                lbl->setforeground(c(255,255,255));
                lbl->setmargin(Thickness{8, 8, 0, 0});
                box->addChild(lbl);
                flex->addChild(box);
            }
            inner->addChild(flex);

            // Second row: vertical flex with shrink
            auto flex2 = makeRef<FlexPanel>(Orientation::Vertical);
            flex2->setwidth(300.0f);
            flex2->setheight(100.0f);
            flex2->setspacing(4.0f);
            flex2->setjustifyContent(FlexJustify::SpaceBetween);
            flex2->setalignItems(FlexAlign::Stretch);
            flex2->setmargin(Thickness{0, 8, 0, 0});
            flex2->setbackground(c(24, 26, 36));
            flex2->setcornerRadius(4.0f);

            for (int i = 0; i < 4; ++i) {
                auto row = makeRef<Panel>();
                row->setheight(20.0f);
                row->setbackground(c(50 + i * 30, 80, 180 - i * 20));
                row->setcornerRadius(3.0f);
                auto t = makeRef<Text>();
                t->settext(String("Row ") + std::to_string(i + 1).c_str());
                t->setfontSize(11.0f);
                t->setforeground(c(255,255,255));
                t->setmargin(Thickness{8, 2, 0, 0});
                row->addChild(t);
                flex2->addChild(row);
            }
            inner->addChild(flex2);

            card->addChild(inner);
            col1->addChild(card);
        }
        page->addChild(col1);

        // --- Column 2: RelativePanel + RadialPanel ---
        auto col2 = makeRef<StackPanel>(Orientation::Vertical);
        col2->setspacing(20.0f);

        { // RelativePanel card
            auto card = makeCard(340.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{16, 16, 16, 16});
            inner->addChild(makeHeading("RelativePanel"));
            inner->addChild(makeLabel("Position children relative to each other"));

            auto rel = makeRef<RelativePanel>();
            rel->setwidth(300.0f);
            rel->setheight(150.0f);
            rel->setbackground(c(24, 26, 36));
            rel->setcornerRadius(4.0f);
            rel->setborderColor(c(45, 50, 70));
            rel->setborderWidth(1.0f);
            rel->setmargin(Thickness{0, 10, 0, 0});

            // Header box: aligned top-left with panel
            auto header = makeRef<Panel>();
            header->setid("header");
            header->setwidth(120.0f); header->setheight(30.0f);
            header->setbackground(c(80,140,220));
            header->setcornerRadius(4.0f);
            RelativePanel::setAlignLeftWithPanel(*header, true);
            RelativePanel::setAlignTopWithPanel(*header, true);
            auto ht = makeRef<Text>();
            ht->settext("Header"); ht->setfontSize(12.0f);
            ht->setforeground(c(255,255,255));
            ht->setmargin(Thickness{8, 6, 0, 0});
            header->addChild(ht);
            rel->addChild(header);

            // Sidebar: below header, panel-left
            auto sidebar = makeRef<Panel>();
            sidebar->setid("sidebar");
            sidebar->setwidth(70.0f); sidebar->setheight(80.0f);
            sidebar->setbackground(c(60,180,100));
            sidebar->setcornerRadius(4.0f);
            RelativePanel::setBelow(*sidebar, "header");
            RelativePanel::setAlignLeftWithPanel(*sidebar, true);
            auto st = makeRef<Text>();
            st->settext("Side"); st->setfontSize(11.0f);
            st->setforeground(c(255,255,255));
            st->setmargin(Thickness{8, 6, 0, 0});
            sidebar->addChild(st);
            rel->addChild(sidebar);

            // Content: right of sidebar, below header
            auto content = makeRef<Panel>();
            content->setid("content");
            content->setwidth(140.0f); content->setheight(80.0f);
            content->setbackground(c(200,100,60));
            content->setcornerRadius(4.0f);
            RelativePanel::setRightOf(*content, "sidebar");
            RelativePanel::setBelow(*content, "header");
            auto ct = makeRef<Text>();
            ct->settext("Content"); ct->setfontSize(11.0f);
            ct->setforeground(c(255,255,255));
            ct->setmargin(Thickness{8, 6, 0, 0});
            content->addChild(ct);
            rel->addChild(content);

            // Footer: bottom-right of panel
            auto footer = makeRef<Panel>();
            footer->setid("footer");
            footer->setwidth(100.0f); footer->setheight(24.0f);
            footer->setbackground(c(150,80,200));
            footer->setcornerRadius(4.0f);
            RelativePanel::setAlignRightWithPanel(*footer, true);
            RelativePanel::setAlignBottomWithPanel(*footer, true);
            auto ft = makeRef<Text>();
            ft->settext("Footer"); ft->setfontSize(10.0f);
            ft->setforeground(c(255,255,255));
            ft->setmargin(Thickness{8, 4, 0, 0});
            footer->addChild(ft);
            rel->addChild(footer);

            inner->addChild(rel);
            card->addChild(inner);
            col2->addChild(card);
        }
        { // RadialPanel card
            auto card = makeCard(340.0f);
            auto inner = makeRef<StackPanel>(Orientation::Vertical);
            inner->setmargin(Thickness{16, 16, 16, 16});
            inner->addChild(makeHeading("RadialPanel"));
            inner->addChild(makeLabel("Arrange children in a circle"));

            auto radial = makeRef<RadialPanel>();
            radial->setwidth(300.0f);
            radial->setheight(260.0f);
            radial->setbackground(c(24, 26, 36));
            radial->setcornerRadius(4.0f);
            radial->setborderColor(c(45, 50, 70));
            radial->setborderWidth(1.0f);
            radial->setmargin(Thickness{0, 10, 0, 0});

            const char* hours[] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
            for (int i = 0; i < 12; ++i) {
                auto dot = makeRef<Panel>();
                dot->setwidth(32.0f);
                dot->setheight(32.0f);
                dot->setbackground(c(50 + i*15, 100, 220 - i*10));
                dot->setcornerRadius(16.0f);
                auto t = makeRef<Text>();
                t->settext(hours[i]);
                t->setfontSize(11.0f);
                t->setforeground(c(255,255,255));
                t->setmargin(Thickness{8, 7, 0, 0});
                dot->addChild(t);
                radial->addChild(dot);
            }
            inner->addChild(radial);
            card->addChild(inner);
            col2->addChild(card);
        }
        page->addChild(col2);

        tabs->addTab("Layouts 2", page);
    }

    root->addChild(tabs);
    g_gutCtx->setRoot(root);
}

// ============================================================================
// Window proc
// ============================================================================

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_gutCtx) {
        switch (msg) {
        case WM_MOUSEMOVE: {
            float mx = (float)LOWORD(lParam);
            float my = (float)HIWORD(lParam);
            g_gutCtx->processMouseMove(mx, my);
            break;
        }
        case WM_LBUTTONDOWN:
            g_gutCtx->processMouseButton(gut::MouseButton::Left, true);
            SetCapture(hwnd);
            break;
        case WM_LBUTTONUP:
            g_gutCtx->processMouseButton(gut::MouseButton::Left, false);
            ReleaseCapture();
            break;
        case WM_RBUTTONDOWN:
            g_gutCtx->processMouseButton(gut::MouseButton::Right, true);
            break;
        case WM_RBUTTONUP:
            g_gutCtx->processMouseButton(gut::MouseButton::Right, false);
            break;
        case WM_MOUSEWHEEL: {
            float delta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            g_gutCtx->processMouseWheel(0, delta);
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            gut::ModifierKeys mods = gut::ModifierKeys::None;
            if (GetKeyState(VK_SHIFT) & 0x8000)   mods = mods | gut::ModifierKeys::Shift;
            if (GetKeyState(VK_CONTROL) & 0x8000)  mods = mods | gut::ModifierKeys::Control;
            if (GetKeyState(VK_MENU) & 0x8000)     mods = mods | gut::ModifierKeys::Alt;
            g_gutCtx->processKey(static_cast<gut::Key>(wParam), true, mods);
            break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            g_gutCtx->processKey(static_cast<gut::Key>(wParam), false);
            break;
        }
        case WM_CHAR: {
            if (wParam >= 32) {
                g_gutCtx->processTextInput(static_cast<char32_t>(wParam));
            }
            break;
        }
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                g_width = LOWORD(lParam);
                g_height = HIWORD(lParam);
            }
            break;
        }
    }

    switch (msg) {
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ============================================================================
// Entry point
// ============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Get DPI
    SetProcessDPIAware();

    // Register window class
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"GutD3D11Demo";
    RegisterClassExW(&wc);

    // Create window
    RECT rc = { 0, 0, (LONG)g_width, (LONG)g_height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0, L"GutD3D11Demo", L"Gut — D3D11 Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    // DPI
    HDC hdc = GetDC(hwnd);
    g_dpi = (float)GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    ReleaseDC(hwnd, hdc);

    // Init D3D11
    if (!InitD3D11(hwnd)) {
        MessageBoxW(hwnd, L"Failed to initialize D3D11", L"Error", MB_OK);
        return 1;
    }

    // Init Gut
    gut::initialize();
    BuildUI();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Main loop
    auto lastTime = std::chrono::high_resolution_clock::now();

    MSG msg{};
    while (g_running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) g_running = false;
        }
        if (!g_running) break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::milli>(now - lastTime).count();
        lastTime = now;

        g_gutCtx->update(dt);
        g_gutCtx->render(g_width, g_height);

        g_swapChain->Present(1, 0); // VSync
    }

    g_gutCtx.reset();
    gut::shutdown();

    return 0;
}
