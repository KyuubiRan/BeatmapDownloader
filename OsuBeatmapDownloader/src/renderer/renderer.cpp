#include "pch.h"
#include "renderer.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_opengl3.h>

#include "backend/DirectX.h"
#include "backend/OpenGL.h"
#include "glob/GlobalEvents.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer {

ImFont *_currentFont = nullptr;

void OnRenderDX9(LPDIRECT3DDEVICE9 d3ddvice) {
    ImGuiIO &io = ImGui::GetIO();
    if (!io.Fonts->IsBuilt()) {
        io.Fonts->Build();
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (_currentFont != nullptr)
        io.FontDefault = _currentFont;

    ImGui::NewFrame();
    global::OnRender();
    ImGui::EndFrame();

    ImGui::Render();

    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void OnResetDX9(LPDIRECT3DDEVICE9 d3ddvice) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();
}

void OnRenderGL(HDC hdc) {
    static bool glContextCreated = false;
    if (!glContextCreated) {
        ImGui_ImplOpenGL3_Init();
        glContextCreated = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (_currentFont != nullptr)
        io.FontDefault = _currentFont;

    ImGui::NewFrame();
    global::OnRender();
    ImGui::EndFrame();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


LRESULT CALLBACK HwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ImGuiIO &io = ImGui::GetIO();
    POINT mPos;
    GetCursorPos(&mPos);
    ScreenToClient(hWnd, &mPos);
    io.MousePos.x = static_cast<float>(mPos.x);
    io.MousePos.y = static_cast<float>(mPos.y);

    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    global::OnWndProc(hWnd, uMsg, wParam, lParam);

    return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

void Init(GraphicsApiType version) {
    HWND window = nullptr;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        wchar_t className[256];
        GetClassName(hwnd, className, 256);
        if (wcsncmp(className, L"WindowsForms", 12) == 0) {
            wchar_t title[256];
            GetWindowText(hwnd, title, 256);
            if (wcscmp(title, L"osu!") == 0) {
                *(HWND *)lParam = hwnd;
                return FALSE;
            }
        }
        return TRUE;
    }, (LPARAM)&window);

    if (window == nullptr) {
        LOGE("Cannot found osu! window!");
        return;
    }

    renderer::OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)renderer::HwndProc);

    LOGI("HWND hook Inited");

    switch (version) {
        case GraphicsApiType::D3D9:
            backend::DX9Events::OnRender += OnRenderDX9;
            backend::DX9Events::OnReset += OnResetDX9;
            backend::InitDX9Hooks();
            break;
        case GraphicsApiType::OpenGL3:
            backend::GLEvents::OnRender += OnRenderGL;
            backend::InitGLHooks();
            break;
        default:
            LOGE("Unsupported api version!");
            break;
    }
}

void SetCurrentFont(uint8_t *byte, size_t size, float pixelSize, ImFontConfig *cfg, const ImWchar *glyphRanges) {
    if (byte == nullptr || size == 0) {
        LOGE("Font file doesn't exists!");
        return;
    }

    LOGI("Begin change font");
    ImGuiIO &io = ImGui::GetIO();
    _currentFont = io.Fonts->AddFontFromMemoryTTF(byte, size, pixelSize, cfg, glyphRanges);
    LOGI("Finished change font");
}

}

