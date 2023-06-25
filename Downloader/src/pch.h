#ifndef PCH_H
#define PCH_H

#include "framework.h"
#include "Logger.h"
#include "HookManager.hpp"
#include <imgui.h>
#include "config/I18nManager.h"
#include <misc/cpp/imgui_stdlib.h>

#define GET_LANG() auto &lang = i18n::I18nManager::GetInstance()
#endif
