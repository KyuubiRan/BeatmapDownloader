#pragma once

#include <Windows.h>
#include <gl/GL.h>
#include <Event.hpp>

namespace backend {
void InitGLHooks();

namespace GLEvents {
inline static event::Event<HDC> OnRender {};
}
}