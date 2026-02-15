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

// Shared UI code
#include "demo_ui.h"

// ============================================================================
// Globals
// ============================================================================

static ComPtr<ID3D11Device>        g_device;
static ComPtr<ID3D11DeviceContext>  g_context;
static ComPtr<IDXGISwapChain>      g_swapChain;

static std::unique_ptr<gut::Context> g_gutCtx;
static std::vector<gut::Ref<gut::FloatAnimation>> g_animations;
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
    auto backend = std::make_unique<gut::D3D11RenderBackend>(
        g_device.Get(), g_context.Get(), g_swapChain.Get());

    g_gutCtx = std::make_unique<gut::Context>(std::move(backend));

    // Clipboard callbacks (Windows)
    g_gutCtx->setOnGetClipboardText([]() -> gut::String {
        if (!OpenClipboard(nullptr)) return {};
        gut::String result;
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* text = static_cast<wchar_t*>(GlobalLock(hData));
            if (text) {
                int len = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
                if (len > 0) {
                    std::vector<char> buffer(len);
                    WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer.data(), len, nullptr, nullptr);
                    result = buffer.data();
                }
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
        return result;
    });
    g_gutCtx->setOnSetClipboardText([](const gut::String& text) {
        if (!OpenClipboard(nullptr)) return;
        EmptyClipboard();
        int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (wlen > 0) {
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wlen * sizeof(wchar_t));
            if (hMem) {
                wchar_t* dest = static_cast<wchar_t*>(GlobalLock(hMem));
                if (dest) {
                    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, dest, wlen);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
            }
        }
        CloseClipboard();
    });

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
            std::vector<gut::u8> data(sz);
            fread(data.data(), 1, sz, f);
            fclose(f);
            g_gutCtx->loadFont(data.data(), data.size());
        }
    }

    // Build shared UI
    demo::DemoConfig cfg;
    cfg.width = g_width;
    cfg.height = g_height;
    cfg.backendName = "Direct3D 11";
    cfg.backendShort = "D3D11";
    cfg.shaderModel = "5.0";

    auto result = demo::buildDemoUI(*g_gutCtx, cfg);
    g_gutCtx->setRoot(result.root);
    g_animations = std::move(result.animations);
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
        case WM_SYSKEYUP:
            g_gutCtx->processKey(static_cast<gut::Key>(wParam), false);
            break;
        case WM_CHAR:
            if (wParam >= 32)
                g_gutCtx->processTextInput(static_cast<char32_t>(wParam));
            break;
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

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"GutD3D11Demo";
    RegisterClassExW(&wc);

    RECT rc = { 0, 0, (LONG)g_width, (LONG)g_height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0, L"GutD3D11Demo", L"Gut \xe2\x80\x94 D3D11 Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    HDC hdc = GetDC(hwnd);
    g_dpi = (float)GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    ReleaseDC(hwnd, hdc);

    if (!InitD3D11(hwnd)) {
        MessageBoxW(hwnd, L"Failed to initialize D3D11", L"Error", MB_OK);
        return 1;
    }

    gut::initialize();
    BuildUI();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

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

    g_animations.clear();
    g_gutCtx.reset();
    gut::shutdown();
    return 0;
}
