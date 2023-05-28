#pragma once

#include <imgui.h>
#include <d3d11.h>

namespace renderer {
inline HHOOK g_msgHook = nullptr;

enum class GraphicsApiType {
    D3D9,
    D3D10,
    D3D11,
    D3D12,
    OpenGL2,
    OpenGL3,
    Vulkan
};

inline static WNDPROC OriginalWndProcHandler = nullptr;

void Init(GraphicsApiType version = GraphicsApiType::OpenGL3);

void SetCurrentFont(uint8_t *byte, size_t size, float pixelSize, ImFontConfig *cfg, const ImWchar *glyphRanges);

void OnRenderDX11(ID3D11DeviceContext *pContext);
void OnInitializeDX11(HWND window, ID3D11Device *pDevice, ID3D11DeviceContext *pContext, IDXGISwapChain *pChain);

void OnRenderGL(HDC hdc);
}
