#include "pch.h"
#include "About.h"

#include "CustomHotkey.h"
#include "config/I18nManager.h"
#include "misc/Color.h"
#include "utils/gui_utils.h"
#include <shellapi.h>
#include "misc/VersionManager.h"

namespace features {

About::About() :
    Feature() {
}

FeatureInfo &About::getInfo() {
    static auto info = FeatureInfo{
        .category = "About",
        .groupName = ""
    };

    return info;
}

using misc::VersionManager;

void About::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::BeginGroupPanel(lang.getTextCStr("About"));
    ImGui::Text("%s", lang.getTextCStr("ProjectAuthor"));
    ImGui::Text("%s", lang.getTextCStr("ProjectLink"));
    ImGui::SameLine();
    ImGui::TextUrl("https://github.com/KyuubiRan/BeatmapDownloader");
    ImGui::TextColored(color::Green, lang.getTextCStr("CurrentVersion"), VersionManager::GetCurrentVersionName().data(),
                       VersionManager::GetCurrentVersionCode());
    ImGui::SameLine();
    if (ImGui::Button(lang.getTextCStr("CheckUpdate"))) {
        VersionManager::CheckUpdate(true);
    }
    if (!VersionManager::IsLatest()) {
        ImGui::TextColored(color::Orange, lang.getTextCStr("FoundNewVersion"), VersionManager::GetLatestVersionName().data(),
                           VersionManager::GetLatestVersionCode());
        ImGui::SameLine();
        if (ImGui::Button(lang.getTextCStr("GotoDownload"))) {
            ShellExecuteW(nullptr, L"open", L"https://github.com/KyuubiRan/BeatmapDownloader/releases/latest", nullptr, nullptr, SW_HIDE);
        }
    }
    ImGui::EndGroupPanel();

    ImGui::BeginGroupPanel(lang.getTextCStr("Hotkey"));
    auto &htk = features::CustomHotkey::GetInstance();
    ImGui::Text(lang.getTextCStr("HotkeyDesc"), htk.f_MainMenuHotkey->toString().c_str(), htk.f_SearchHotkey->toString().c_str());
    ImGui::EndGroupPanel();
}
}
