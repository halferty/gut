/**
 * @file d3d12_demo.cpp
 * @brief Win32 + D3D12 demo for Gut UI Toolkit
 *
 * Build:
 *   cl /std:c++20 /EHsc /I.. d3d12_demo.cpp /link d3d12.lib dxgi.lib d3dcompiler.lib user32.lib
 *
 * Or use CMake (see examples/CMakeLists.txt).
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Include backend (defines GUT_IMPLEMENTATION)
#include "d3d12_backend.h"
#include "d3d12_backend.cpp"

#include <memory>
#include <chrono>

// ============================================================================
// Globals
// ============================================================================

static constexpr UINT FRAME_COUNT = 2;

static ComPtr<IDXGIFactory4>        g_factory;
static ComPtr<ID3D12Device>         g_device;
static ComPtr<ID3D12CommandQueue>   g_commandQueue;
static ComPtr<IDXGISwapChain3>      g_swapChain;

static std::unique_ptr<gut::Context> g_gutCtx;
static bool g_running = true;
static UINT g_width = 1280;
static UINT g_height = 800;
static float g_dpi = 1.0f;

// ============================================================================
// D3D12 Init
// ============================================================================

static bool InitD3D12(HWND hwnd) {
    UINT dxgiFlags = 0;
#ifdef _DEBUG
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
            dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    if (FAILED(CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&g_factory))))
        return false;

    // Try to create a hardware device
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; g_factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                        IID_PPV_ARGS(&g_device))))
            break;
    }
    if (!g_device) return false;

    // Command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_commandQueue))))
        return false;

    // Swap chain
    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.BufferCount = FRAME_COUNT;
    scDesc.Width = g_width;
    scDesc.Height = g_height;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(g_factory->CreateSwapChainForHwnd(
            g_commandQueue.Get(), hwnd, &scDesc,
            nullptr, nullptr, &swapChain1)))
        return false;

    // Disable Alt+Enter fullscreen
    g_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    swapChain1.As(&g_swapChain);
    return g_swapChain != nullptr;
}

// ============================================================================
// Build Gut UI
// ============================================================================

static void BuildUI() {
    using namespace gut;

    auto backend = std::make_unique<D3D12RenderBackend>(
        g_device.Get(), g_commandQueue.Get(), g_swapChain.Get());

    g_gutCtx = std::make_unique<Context>(std::move(backend));

    // Load system font
    {
        const char* fontPaths[] = {
            "C:\\Windows\\Fonts\\segoeui.ttf",
            "C:\\Windows\\Fonts\\arial.ttf",
            "C:\\Windows\\Fonts\\tahoma.ttf",
        };
        for (auto path : fontPaths) {
            FILE* f = fopen(path, "rb");
            if (!f) continue;
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<u8> data(sz);
            fread(data.data(), 1, sz, f);
            fclose(f);
            g_gutCtx->loadFont(data.data(), data.size());
            break;
        }
    }

    auto c = [](u8 r, u8 g, u8 b, u8 a = 255) { return Color::fromRgba8(r, g, b, a); };

    // Root canvas
    auto root = makeRef<Canvas>();
    root->setwidth(static_cast<f32>(g_width));
    root->setheight(static_cast<f32>(g_height));
    root->setbackground(c(20, 22, 30));

    // --- Title ---
    auto title = makeRef<Text>();
    title->settext("Gut — D3D12 Demo");
    title->setfontSize(28.0f);
    title->setforeground(c(230, 235, 255));
    Canvas::setLeft(*title, 30.0f);
    Canvas::setTop(*title, 20.0f);
    root->addChild(title);

    // --- Info panel ---
    auto infoPanel = makeRef<Panel>();
    infoPanel->setwidth(350.0f);
    infoPanel->setheight(180.0f);
    infoPanel->setbackground(c(35, 38, 50));
    infoPanel->setcornerRadius(8.0f);
    infoPanel->setborderColor(c(60, 65, 90));
    infoPanel->setborderWidth(1.0f);
    Canvas::setLeft(*infoPanel, 30.0f);
    Canvas::setTop(*infoPanel, 70.0f);
    root->addChild(infoPanel);

    auto infoStack = makeRef<StackPanel>();
    infoStack->setorientation(Orientation::Vertical);
    infoStack->setmargin(Thickness{16, 16, 16, 16});
    infoPanel->addChild(infoStack);

    auto addInfoRow = [&](const char* label, const char* value) {
        auto row = makeRef<StackPanel>();
        row->setorientation(Orientation::Horizontal);
        row->setmargin(Thickness{0, 0, 0, 6});

        auto lbl = makeRef<Text>();
        lbl->settext(label);
        lbl->setfontSize(14.0f);
        lbl->setforeground(c(140, 150, 180));
        row->addChild(lbl);

        auto val = makeRef<Text>();
        val->settext(value);
        val->setfontSize(14.0f);
        val->setforeground(c(220, 225, 240));
        val->setmargin(Thickness{8, 0, 0, 0});
        row->addChild(val);

        infoStack->addChild(row);
    };

    addInfoRow("Backend:", "Direct3D 12");
    addInfoRow("Shader Model:", "5.1");
    addInfoRow("Resolution:", (std::to_string(g_width) + "x" + std::to_string(g_height)).c_str());
    addInfoRow("Root Signature:", "CBV inline + SRV table");
    addInfoRow("Frame Buffering:", "Double-buffered");

    // --- Buttons column ---
    auto buttonPanel = makeRef<Panel>();
    buttonPanel->setwidth(350.0f);
    buttonPanel->setheight(280.0f);
    buttonPanel->setbackground(c(35, 38, 50));
    buttonPanel->setcornerRadius(8.0f);
    buttonPanel->setborderColor(c(60, 65, 90));
    buttonPanel->setborderWidth(1.0f);
    Canvas::setLeft(*buttonPanel, 30.0f);
    Canvas::setTop(*buttonPanel, 270.0f);
    root->addChild(buttonPanel);

    auto btnStack = makeRef<StackPanel>();
    btnStack->setorientation(Orientation::Vertical);
    btnStack->setmargin(Thickness{16, 16, 16, 16});
    buttonPanel->addChild(btnStack);

    auto sectionTitle = makeRef<Text>();
    sectionTitle->settext("Controls");
    sectionTitle->setfontSize(16.0f);
    sectionTitle->setforeground(c(200, 205, 220));
    sectionTitle->setmargin(Thickness{0, 0, 0, 10});
    btnStack->addChild(sectionTitle);

    auto makeButton = [&](const char* text) {
        auto btn = makeRef<Button>();
        btn->setlabel(text);
        btn->setwidth(200.0f);
        btn->setheight(36.0f);
        btn->setmargin(Thickness{0, 0, 0, 8});
        return btn;
    };

    btnStack->addChild(makeButton("New Game"));
    btnStack->addChild(makeButton("Continue"));
    btnStack->addChild(makeButton("Options"));
    btnStack->addChild(makeButton("Quit"));

    // --- Checkbox / Radio section ---
    auto checkBox = makeRef<CheckBox>();
    checkBox->setlabel("Enable Ray Tracing");
    checkBox->setforeground(c(200, 205, 220));
    checkBox->setmargin(Thickness{0, 4, 0, 0});
    btnStack->addChild(checkBox);

    auto radio1 = makeRef<RadioButton>();
    radio1->setlabel("Quality: Ultra");
    radio1->setforeground(c(200, 205, 220));
    radio1->setmargin(Thickness{0, 4, 0, 0});
    btnStack->addChild(radio1);

    auto radio2 = makeRef<RadioButton>();
    radio2->setlabel("Quality: Medium");
    radio2->setforeground(c(200, 205, 220));
    radio2->setmargin(Thickness{0, 4, 0, 0});
    btnStack->addChild(radio2);

    // --- Slider panel ---
    auto sliderPanel = makeRef<Panel>();
    sliderPanel->setwidth(350.0f);
    sliderPanel->setheight(120.0f);
    sliderPanel->setbackground(c(35, 38, 50));
    sliderPanel->setcornerRadius(8.0f);
    sliderPanel->setborderColor(c(60, 65, 90));
    sliderPanel->setborderWidth(1.0f);
    Canvas::setLeft(*sliderPanel, 400.0f);
    Canvas::setTop(*sliderPanel, 70.0f);
    root->addChild(sliderPanel);

    auto sliderStack = makeRef<StackPanel>();
    sliderStack->setorientation(Orientation::Vertical);
    sliderStack->setmargin(Thickness{16, 16, 16, 16});
    sliderPanel->addChild(sliderStack);

    auto sliderLabel = makeRef<Text>();
    sliderLabel->settext("Render Scale");
    sliderLabel->setfontSize(14.0f);
    sliderLabel->setforeground(c(180, 185, 200));
    sliderStack->addChild(sliderLabel);

    auto slider = makeRef<Slider>();
    slider->setwidth(300.0f);
    slider->setminimum(50.0f);
    slider->setmaximum(200.0f);
    slider->setvalue(100.0f);
    slider->setmargin(Thickness{0, 8, 0, 0});
    sliderStack->addChild(slider);

    auto sliderLabel2 = makeRef<Text>();
    sliderLabel2->settext("FOV");
    sliderLabel2->setfontSize(14.0f);
    sliderLabel2->setforeground(c(180, 185, 200));
    sliderLabel2->setmargin(Thickness{0, 8, 0, 0});
    sliderStack->addChild(sliderLabel2);

    auto slider2 = makeRef<Slider>();
    slider2->setwidth(300.0f);
    slider2->setminimum(60.0f);
    slider2->setmaximum(120.0f);
    slider2->setvalue(90.0f);
    slider2->setmargin(Thickness{0, 4, 0, 0});
    sliderStack->addChild(slider2);

    // --- Progress bar ---
    auto progressPanel = makeRef<Panel>();
    progressPanel->setwidth(350.0f);
    progressPanel->setheight(80.0f);
    progressPanel->setbackground(c(35, 38, 50));
    progressPanel->setcornerRadius(8.0f);
    progressPanel->setborderColor(c(60, 65, 90));
    progressPanel->setborderWidth(1.0f);
    Canvas::setLeft(*progressPanel, 400.0f);
    Canvas::setTop(*progressPanel, 210.0f);
    root->addChild(progressPanel);

    auto progStack = makeRef<StackPanel>();
    progStack->setorientation(Orientation::Vertical);
    progStack->setmargin(Thickness{16, 16, 16, 16});
    progressPanel->addChild(progStack);

    auto progLabel = makeRef<Text>();
    progLabel->settext("Compiling Shaders...");
    progLabel->setfontSize(14.0f);
    progLabel->setforeground(c(180, 185, 200));
    progStack->addChild(progLabel);

    auto progress = makeRef<ProgressBar>();
    progress->setwidth(300.0f);
    progress->setvalue(42.0f);
    progress->setmargin(Thickness{0, 8, 0, 0});
    progStack->addChild(progress);

    // --- TextBox ---
    auto textBoxPanel = makeRef<Panel>();
    textBoxPanel->setwidth(350.0f);
    textBoxPanel->setheight(80.0f);
    textBoxPanel->setbackground(c(35, 38, 50));
    textBoxPanel->setcornerRadius(8.0f);
    textBoxPanel->setborderColor(c(60, 65, 90));
    textBoxPanel->setborderWidth(1.0f);
    Canvas::setLeft(*textBoxPanel, 400.0f);
    Canvas::setTop(*textBoxPanel, 310.0f);
    root->addChild(textBoxPanel);

    auto tbStack = makeRef<StackPanel>();
    tbStack->setorientation(Orientation::Vertical);
    tbStack->setmargin(Thickness{16, 16, 16, 16});
    textBoxPanel->addChild(tbStack);

    auto tbLabel = makeRef<Text>();
    tbLabel->settext("Server Address");
    tbLabel->setfontSize(14.0f);
    tbLabel->setforeground(c(180, 185, 200));
    tbStack->addChild(tbLabel);

    auto textBox = makeRef<TextBox>();
    textBox->setwidth(300.0f);
    textBox->setplaceholder("127.0.0.1:7777");
    textBox->setmargin(Thickness{0, 6, 0, 0});
    tbStack->addChild(textBox);

    // --- Transforms panel ---
    auto transformPanel = makeRef<Panel>();
    transformPanel->setwidth(350.0f);
    transformPanel->setheight(350.0f);
    transformPanel->setbackground(c(35, 38, 50));
    transformPanel->setcornerRadius(8.0f);
    transformPanel->setborderColor(c(60, 65, 90));
    transformPanel->setborderWidth(1.0f);
    Canvas::setLeft(*transformPanel, 400.0f);
    Canvas::setTop(*transformPanel, 410.0f);
    root->addChild(transformPanel);

    {
        auto tfStack = makeRef<StackPanel>();
        tfStack->setorientation(Orientation::Vertical);
        tfStack->setmargin(Thickness{16, 16, 16, 16});
        transformPanel->addChild(tfStack);

        auto tfTitle = makeRef<Text>();
        tfTitle->settext("Transforms");
        tfTitle->setfontSize(16.0f);
        tfTitle->setforeground(c(200, 205, 220));
        tfTitle->setmargin(Thickness{0, 0, 0, 6});
        tfStack->addChild(tfTitle);

        auto tfSub = makeRef<Text>();
        tfSub->settext("Scale / Rotate / Skew (Flash-style)");
        tfSub->setfontSize(12.0f);
        tfSub->setforeground(c(140, 150, 180));
        tfSub->setmargin(Thickness{0, 0, 0, 14});
        tfStack->addChild(tfSub);

        const float pi = 3.14159265f;

        // Row 1: Rotation samples
        auto row1 = makeRef<StackPanel>(Orientation::Horizontal);
        row1->setspacing(20.0f);

        for (auto [angle, clr, label] : std::initializer_list<std::tuple<f32, Color, const char*>>{
                {0.0f, c(80,150,240), "0\xc2\xb0"},
                {15.0f * pi / 180.0f, c(200,100,60), "15\xc2\xb0"},
                {45.0f * pi / 180.0f, c(60,180,100), "45\xc2\xb0"},
                {90.0f * pi / 180.0f, c(220,160,40), "90\xc2\xb0"}}) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(45.0f); box->setheight(45.0f);
            box->setcornerRadius(6.0f);
            box->setbackground(clr);
            box->setrotation(angle);
            col->addChild(box);
            auto lbl = makeRef<Text>();
            lbl->settext(label); lbl->setfontSize(10.0f);
            lbl->setforeground(c(140, 150, 180));
            lbl->setmargin(Thickness{0, 6, 0, 0});
            col->addChild(lbl);
            row1->addChild(col);
        }
        tfStack->addChild(row1);

        // Row 2: Skew samples
        auto row2 = makeRef<StackPanel>(Orientation::Horizontal);
        row2->setspacing(20.0f);
        row2->setmargin(Thickness{0, 24, 0, 0});

        for (auto [skx, sky, clr, label] : std::initializer_list<std::tuple<f32, f32, Color, const char*>>{
                {15.0f * pi / 180.0f, 0.0f, c(150,80,200), "skX 15\xc2\xb0"},
                {30.0f * pi / 180.0f, 0.0f, c(50,160,180), "skX 30\xc2\xb0"},
                {0.0f, 20.0f * pi / 180.0f, c(200,60,100), "skY 20\xc2\xb0"},
                {10.0f * pi / 180.0f, 10.0f * pi / 180.0f, c(100,180,60), "X+Y"}}) {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(45.0f); box->setheight(45.0f);
            box->setbackground(clr);
            box->setskewX(skx); box->setskewY(sky);
            col->addChild(box);
            auto lbl = makeRef<Text>();
            lbl->settext(label); lbl->setfontSize(10.0f);
            lbl->setforeground(c(140, 150, 180));
            lbl->setmargin(Thickness{0, 6, 0, 0});
            col->addChild(lbl);
            row2->addChild(col);
        }
        tfStack->addChild(row2);

        // Row 3: Combined transforms
        auto row3 = makeRef<StackPanel>(Orientation::Horizontal);
        row3->setspacing(20.0f);
        row3->setmargin(Thickness{0, 24, 0, 0});

        {
            auto col = makeRef<StackPanel>(Orientation::Vertical);
            col->sethorizontalAlignment(HorizontalAlignment::Center);
            auto box = makeRef<Panel>();
            box->setwidth(55.0f); box->setheight(55.0f);
            box->setcornerRadius(8.0f);
            box->setbackground(c(220, 80, 120));
            box->setscaleX(1.2f); box->setscaleY(0.8f);
            box->setrotation(25.0f * pi / 180.0f);
            box->setskewX(10.0f * pi / 180.0f);
            col->addChild(box);
            auto lbl = makeRef<Text>();
            lbl->settext("S+R+Sk"); lbl->setfontSize(10.0f);
            lbl->setforeground(c(140, 150, 180));
            lbl->setmargin(Thickness{0, 6, 0, 0});
            col->addChild(lbl);
            row3->addChild(col);
        }

        {
            auto btn = makeRef<Button>();
            btn->setlabel("Rotated Btn");
            btn->setwidth(110.0f);
            btn->setheight(32.0f);
            btn->setrotation(10.0f * pi / 180.0f);
            row3->addChild(btn);
        }

        {
            auto btn = makeRef<Button>();
            btn->setlabel("Skewed Btn");
            btn->setwidth(110.0f);
            btn->setheight(32.0f);
            btn->setskewX(12.0f * pi / 180.0f);
            row3->addChild(btn);
        }

        tfStack->addChild(row3);
    }

    // --- Status bar ---
    auto statusBar = makeRef<Panel>();
    statusBar->setwidth(static_cast<f32>(g_width));
    statusBar->setheight(32.0f);
    statusBar->setbackground(c(25, 27, 35));
    Canvas::setLeft(*statusBar, 0.0f);
    Canvas::setTop(*statusBar, static_cast<f32>(g_height - 32));
    root->addChild(statusBar);

    auto statusText = makeRef<Text>();
    statusText->settext("Gut UI Toolkit — D3D12 backend — Windows");
    statusText->setfontSize(12.0f);
    statusText->setforeground(c(120, 125, 140));
    statusText->setmargin(Thickness{12, 8, 0, 0});
    statusBar->addChild(statusText);

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
    SetProcessDPIAware();

    // Register window class
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"GutD3D12Demo";
    RegisterClassExW(&wc);

    // Create window
    RECT rc = { 0, 0, (LONG)g_width, (LONG)g_height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0, L"GutD3D12Demo", L"Gut — D3D12 Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    // DPI
    HDC hdc = GetDC(hwnd);
    g_dpi = (float)GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    ReleaseDC(hwnd, hdc);

    // Init D3D12
    if (!InitD3D12(hwnd)) {
        MessageBoxW(hwnd, L"Failed to initialize D3D12", L"Error", MB_OK);
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

        g_swapChain->Present(1, 0);
    }

    g_gutCtx.reset();
    gut::shutdown();

    return 0;
}
