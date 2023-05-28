#include "pch.h"
#include <shellapi.h>
#include "HandleLinkHook.h"

namespace hook {
void HandleLinkHook::DrawMain() {
}

FeatureInfo &HandleLinkHook::GetInfo() {
    static auto info = FeatureInfo{
        .category = "Downloader",
        .groupName = "HandleLink"
    };

    return info;
}

BOOL ShellExecuteExW_Hook(SHELLEXECUTEINFOW *pExecInfo) {
    return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);
}

HandleLinkHook::HandleLinkHook() : Feature() {

}

}


