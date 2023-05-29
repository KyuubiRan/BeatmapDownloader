#include "pch.h"
#include <shellapi.h>
#include "HandleLinkHook.h"

#include "config/I18nManager.h"
#include "utils/gui_utils.h"

namespace hook {
void HandleLinkHook::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::Checkbox(lang.GetTextCStr("Enabled"), f_Enabled.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("HandleLinkDesc"));
    ImGui::InputText(lang.GetTextCStr("Domain"), f_Domain.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("HandleLinkDomainDesc"));
}

FeatureInfo& HandleLinkHook::getInfo() {
    static auto info = FeatureInfo{
        .category = "Downloader",
        .groupName = "HandleLink"
    };

    return info;
}

BOOL ShellExecuteExW_Hook(SHELLEXECUTEINFOW *pExecInfo) {
    auto &inst = HandleLinkHook::GetInstance();
    if (!inst.f_Enabled.getValue())
        return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);

    if (GetAsyncKeyState(VK_SHIFT) & 8000) {
        LOGI("Shift pressed, skip handle link");
        return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);
    }

    LOGD("ShellExecuteExW call");

    return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);
}

HandleLinkHook::HandleLinkHook() :
    Feature(),
    f_Enabled("HandleLinkEnabled", true),
    f_Domain("HandleLinkDomain", "osu.ppy.sh") {
    const HMODULE hMod = GetModuleHandleA("shell32.dll");
    if (!hMod) {
        LOGE("Cannot get shell32.dll handle! Error code: %d, HandleLink hook will not be initilized!", GetLastError());
        return;
    }
    const auto pFunc = (decltype(&ShellExecuteExW_Hook))GetProcAddress(hMod, "ShellExecuteExW");
    if (!pFunc) {
        LOGE("Cannot get ShellExecuteExW address! Error code: %d, HandleLink hook will not be initilized!", GetLastError());
        return;
    }
    HookManager::InstallHook(pFunc, ShellExecuteExW_Hook);
    LOGI("Initialized HandleLink hook");
}
}
