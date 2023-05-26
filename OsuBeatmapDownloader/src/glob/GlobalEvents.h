#pragma once
#include <Event.hpp>

namespace global {

inline static event::Event<> OnRender {};
inline static event::Event<short> OnKeyDown {};
inline static event::Event<HWND, UINT, WPARAM, LPARAM> OnWndProc {};

}
