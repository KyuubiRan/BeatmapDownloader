#include "pch.h"
#include "DirectX.h"

#include <backends/imgui_impl_dx9.h>
#include <renderer/renderer.h>

#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "winmm.lib")

typedef HRESULT(WINAPI *FnEndScene)(LPDIRECT3DDEVICE9);
typedef HRESULT(WINAPI *FnReset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS *);

LPDIRECT3DDEVICE9 d3dDevice;

HRESULT WINAPI OnEndScene(LPDIRECT3DDEVICE9 pDevice) {
    backend::DX9Events::OnRender(pDevice);
    return HookManager::CallOriginal(OnEndScene, pDevice);
}

HRESULT WINAPI OnReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters) {
    backend::DX9Events::OnReset(pDevice);
    return HookManager::CallOriginal(OnReset, pDevice, pPresentationParameters);
}

HRESULT APIENTRY OnCreateDevice(IDirect3D9 *This, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
    DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface) {
    HRESULT result = HookManager::CallOriginal(OnCreateDevice, This, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    if (SUCCEEDED(result)) {
        d3dDevice = *ppReturnedDeviceInterface;

        static bool hooked = false;

        if (!hooked) {
            ImGui_ImplDX9_Init(d3dDevice);

            DWORD_PTR *pVTable = *(DWORD_PTR **)d3dDevice;
            FnEndScene oEndScene = (FnEndScene)(pVTable[42]);
            FnReset oReset = (FnReset)(pVTable[16]);

            HookManager::InstallHook(oEndScene, OnEndScene);
            HookManager::InstallHook(oReset, OnReset);

            LOGI("D3D9 Rendering hooks installed!");

            hooked = true;
        }
    }

    return result;
}

void backend::InitDX9Hooks() {
    LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);

    if (d3d) {
        DWORD_PTR *vTable = *(DWORD_PTR **)d3d;
        auto oCreateDevice = (decltype(&OnCreateDevice))(vTable[16]);
        HookManager::InstallHook(oCreateDevice, OnCreateDevice);
        LOGI("D3D9 hooks installed!");
    }
}


