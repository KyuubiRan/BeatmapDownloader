#pragma once

#include <imgui.h>

namespace renderer {
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

LRESULT CALLBACK HwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}
