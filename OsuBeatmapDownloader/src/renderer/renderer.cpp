#include "pch.h"
#include "renderer.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_opengl3.h>

#include "backend/DirectX.h"
#include "backend/OpenGL.h"
#include "ui/MainUi.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer {

HWND osuHwnd;

void Update() {
    ui::main::Update();
}

ImFont *_currentFont = nullptr;

static ID3D11RenderTargetView *mainRenderTargetView;

void OnInitializeDX11(HWND window, ID3D11Device *pDevice, ID3D11DeviceContext *pContext, IDXGISwapChain *pChain) {
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);

    osuHwnd = window;

    ID3D11Texture2D *pBackBuffer;
    pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID *>(&pBackBuffer));
    pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
    LOGD("Success initialized dx11!");
}

void OnRenderDX11(ID3D11DeviceContext *pContext) {
    ImGuiIO &io = ImGui::GetIO();

    if (!io.Fonts->IsBuilt()) {
        io.Fonts->Build();
        ImGui_ImplDX11_InvalidateDeviceObjects();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (_currentFont != nullptr)
        io.FontDefault = _currentFont;

    ImGui::NewFrame();
    Update();
    ImGui::Render();

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    return CallWindowProc(OriginalWndProcHandler, hWnd, msg, wParam, lParam);
}

void OnRenderGL(HDC hdc) {
    ImGuiIO &io = ImGui::GetIO();
    if (ImGui::GetCurrentContext() == nullptr)
        return;

    if (!io.BackendRendererUserData) {
        osuHwnd = WindowFromDC(hdc);
        ImGui_ImplOpenGL3_Init();
        ImGui_ImplWin32_Init(osuHwnd);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (_currentFont != nullptr)
        io.FontDefault = _currentFont;

    ImGui::NewFrame();
    Update();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MainUiToggleCheck(LPVOID) {
    while (1) {
        bool isDown = (GetKeyState(VK_HOME) & 8000) != 0;
        static bool toggled = false;
        if (isDown) {
            if (!toggled) {
                toggled = true;
                ui::main::ToggleShow();
            }
        } else {
            toggled = false;
        }
        Sleep(20);
    }
}

BOOL OnPeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    BOOL ret = HookManager::CallOriginal(OnPeekMessage, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplWin32_WndProcHandler(hWnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);

    bool mouseClicked = false;
    switch (lpMsg->message) {
        case WM_KEYDOWN:
            if (lpMsg->wParam == VK_HOME) {
                ui::main::ToggleShow();
            }
            break;
        case WM_KEYUP:
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            mouseClicked = true;
            break;
    }

    return ret;
}

void Init(GraphicsApiType version) {
    switch (version) {
        case GraphicsApiType::D3D11:
            backend::InitDX11Hooks();
            break;
        case GraphicsApiType::OpenGL3:
            backend::InitGLHooks();
            break;
        default:
            LOGE("Unsupported api version!");
            return;
    }

    HMODULE hModule = GetModuleHandle(TEXT("user32.dll"));
    if (hModule == nullptr) {
        LOGE("Cannot get user32.dll handle!");
        return;
    }

    
    auto p = (decltype(&OnPeekMessage))GetProcAddress(hModule, "PeekMessageW");
    LOGD("PeekMessageW address: 0x%p", p);
    HookManager::InstallHook(p, OnPeekMessage);
    
    //CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainUiToggleCheck, nullptr, 0, nullptr);
}

void SetCurrentFont(uint8_t *byte, size_t size, float pixelSize, ImFontConfig *cfg, const ImWchar *glyphRanges) {
    if (byte == nullptr || size == 0) {
        LOGE("Font file doesn't exists!");
        return;
    }

    LOGD("Begin change font");
    ImGuiIO &io = ImGui::GetIO();
    _currentFont = io.Fonts->AddFontFromMemoryTTF(byte, size, pixelSize, cfg, glyphRanges);
    LOGD("Finished change font");
}

}
