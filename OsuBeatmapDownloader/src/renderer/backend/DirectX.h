#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <Event.hpp>

namespace backend {

void InitDX9Hooks();

namespace DX9Events {
    inline static event::Event<LPDIRECT3DDEVICE9> OnRender;
    inline static event::Event<LPDIRECT3DDEVICE9> OnReset;
};
}