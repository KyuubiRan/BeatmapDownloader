#include "pch.h"
#include "OpenGL.h"
#include "renderer/renderer.h"

namespace backend {

typedef BOOL(WINAPI *FnSwapBuffers)(HDC);

BOOL WINAPI OnSwapBuffers(HDC hdc) {
    GLEvents::OnRender(hdc);
    return HookManager::CallOriginal(OnSwapBuffers, hdc);
}

void InitGLHooks() {
    HMODULE hModule = GetModuleHandle(L"gdi32.dll");
    if (hModule == nullptr) {
        LOGE("Cannot get gdi32.dll handle!");
        return;
    }
   
    auto pFn = (FnSwapBuffers)GetProcAddress(hModule, "SwapBuffers");
    LOGD("SwapBuffers address: 0x%p", pFn);
    HookManager::InstallHook(pFn, OnSwapBuffers);
    LOGD("OpenGL rendering hooks installed!");
}

}
