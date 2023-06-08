#include "pch.h"
#include <shellapi.h>
#include "HandleLinkHook.h"

#include <regex>

#include "Downloader.h"
#include "config/I18nManager.h"
#include "misc/Hotkey.hpp"
#include "osu/BeatmapManager.h"
#include "utils/gui_utils.h"

namespace features {
void HandleLinkHook::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();

    ImGui::Checkbox(lang.getTextCStr("Enabled"), f_Enabled.getPtr());
    GuiHelper::ShowTooltip(lang.getTextCStr("HandleLinkDesc"));

    ImGui::InputText(lang.getTextCStr("Domain"), f_Domain.getPtr());
    GuiHelper::ShowTooltip(lang.getTextCStr("HandleLinkDomainDesc"));
}

FeatureInfo &HandleLinkHook::getInfo() {
    static auto info = FeatureInfo{
        .category = "Downloader",
        .groupName = "HandleLink"
    };

    return info;
}

BOOL __stdcall HandleLinkHook::ShellExecuteExW_Hook(SHELLEXECUTEINFOW *pExecInfo) {
    auto &inst = GetInstance();
    if (!inst.f_Enabled.getValue())
        return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);

    if (std::wstring ws = pExecInfo->lpFile; !ws.starts_with(L"http")) {
        LOGD("Not a link, skip handle link");
        return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);
    }

    if (misc::Hotkey::IsDown(VK_CONTROL)) {
        LOGI("Ctrl pressed, skip handle link");
        return HookManager::CallOriginal(ShellExecuteExW_Hook, pExecInfo);
    }

    bool LShiftDown = misc::Hotkey::IsDown(VK_SHIFT);

    static std::string https = "https://";
    static std::string http = "http://";
    auto s = inst.f_Domain.getValue();
    if (s.find(http) == 0) {
        s.erase(0, http.size());
    } else if (s.find(https) == 0) {
        s.erase(0, https.size());
    }
    if (s.back() == '/') {
        s.pop_back();
    }

    auto &bmmgr = osu::BeatmapManager::GetInstance();
    auto &dl = Downloader::GetInstance();
    auto url = utils::ws2s(pExecInfo->lpFile);

    const std::string domainPattern1 = s + R"(/(beatmapsets|s)/(\d+))";
    std::smatch match;
    if (std::regex re1(domainPattern1); std::regex_search(url, match, re1) && match.size() > 2) {
        const int id = std::stoi(match[2].str());
        LOGI("Handle link beatmapsets: %d", id);
        dl.postSearch({downloader::BeatmapType::Sid, id, LShiftDown && !bmmgr.hasBeatmapSet(id)});
        return false;
    }

    std::string domainPattern2 = s + R"(/(beatmaps|b)/(\d+))";

    if (std::regex re2(domainPattern2); std::regex_search(url, match, re2) && match.size() > 2) {
        const int id = std::stoi(match[2].str());
        LOGI("Handle link beatmaps: %d", id);
        dl.postSearch({downloader::BeatmapType::Bid, id, LShiftDown && !bmmgr.hasBeatmap(id)});
        return false;
    }

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
