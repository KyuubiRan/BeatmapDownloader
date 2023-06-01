#include "pch.h"
#include "renderer.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_opengl3.h>

#include "backend/DirectX.h"
#include "backend/OpenGL.h"
#include "ui/MainUi.h"
#include <set>
#include <imgui_internal.h>
#include "../ui/BlockingInput.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer {
static HWND s_OsuHwnd;

void Update() {
    ui::main::Update();
}

ImFont *_currentFont = nullptr;

// taken from https://github.com/veritas501/Osu-Ingame-Downloader/blob/master/OsuIngameDownloader/game_hook.cpp#L166
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION) {
        return CallNextHookEx(g_msgHook, nCode, wParam, lParam);
    }
    MSG *msg = (MSG*)lParam;

    if (wParam == PM_REMOVE) {
        if (msg->message == WM_KEYDOWN) {
            if (msg->wParam == VK_HOME) {
                ui::main::ToggleShow();
            }
        }

        if (ui::InputBlock::IsBlocked())
            ImGui_ImplWin32_WndProcHandler(msg->hwnd, msg->message, msg->wParam, msg->lParam);
    }

    if (ui::InputBlock::IsBlocked()) {
        if (msg->message == WM_CHAR) {
            msg->message = WM_NULL;
            return 1;
        }
        if ((WM_MOUSEFIRST <= msg->message && msg->message <= WM_MOUSELAST) || (msg->message == WM_NCHITTEST) || (msg->message ==
            WM_SETCURSOR)) {
            msg->message = WM_NULL;
            return 1;
        }
    }
    return CallNextHookEx(g_msgHook, nCode, wParam, lParam);
}

DWORD WINAPI MsgHookThread(LPVOID lpParam) {
    DWORD tid = *(DWORD*)lpParam;
    g_msgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, GetModuleHandle(NULL), tid);
    if (!g_msgHook) {
        LOGE("Cannot set message hook!");
        return 1;
    }

    LOGD("Successfully to set message hook.");
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void OnRenderGL(HDC hdc) {
    ImGuiIO &io = ImGui::GetIO();
    ImGuiContext *g = ImGui::GetCurrentContext();
    if (g == nullptr)
        return;

    if (!io.BackendRendererUserData) {
        ImGui_ImplOpenGL3_Init();
        s_OsuHwnd = WindowFromDC(hdc);
        ImGui_ImplWin32_InitForOpenGL(s_OsuHwnd);
        DWORD tid = GetCurrentThreadId();
        CreateThread(nullptr, 0, MsgHookThread, &tid, 0, nullptr);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (_currentFont != nullptr)
        io.FontDefault = _currentFont;


    ImGui::NewFrame();
    __try {
        Update();
    } __except (1) {
        // LOGW("Exception on update");
    }
    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Init(GraphicsApiType version) {
    switch (version) {
    case GraphicsApiType::D3D11:
        LOGE("D3D is not impl yet! Please turn off the compatibility mode!");
        backend::InitDX11Hooks();
        break;
    case GraphicsApiType::OpenGL3:
        backend::InitGLHooks();
        break;
    default:
        LOGE("Unsupported api version!");
        return;
    }
}

static ID3D11RenderTargetView *mainRenderTargetView;

void OnInitializeDX11(HWND window, ID3D11Device *pDevice, ID3D11DeviceContext *pContext, IDXGISwapChain *pChain) {
    /*
    ImGuiIO &io = ImGui::GetIO();
    if (!io.BackendRendererUserData)
        return;

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);

    ID3D11Texture2D *pBackBuffer;
    pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID *>(&pBackBuffer));
    pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
    LOGD("Success initialized dx11!");
     */
}

void OnRenderDX11(ID3D11DeviceContext *pContext) {
    /*
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
    */
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
