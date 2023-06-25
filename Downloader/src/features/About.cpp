#include "pch.h"
#include "About.h"

#include "config/I18nManager.h"
#include "utils/gui_utils.h"

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

void About::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::BeginGroupPanel(lang.getTextCStr("About"));
    ImGui::Text("%s", lang.getTextCStr("ProjectAuthor"));
    ImGui::Text("%s", lang.getTextCStr("ProjectLink"));
    ImGui::SameLine();
    ImGui::TextUrl("https://github.com/KyuubiRan/BeatmapDownloader");
    ImGui::EndGroupPanel();

    ImGui::BeginGroupPanel(lang.getTextCStr("Hotkey"));
    ImGui::Text("%s", lang.getTextCStr("HotkeyDesc"));
    ImGui::EndGroupPanel();
}
}
