#include "pch.h"
#include "About.h"

#include "config/I18nManager.h"
#include "utils/gui_utils.h"

ui::misc::About::About() : Feature() {
}

ui::main::FeatureInfo& ui::misc::About::getInfo() {
    static auto info = main::FeatureInfo{
        .category = "About",
        .groupName = ""
    };

    return info;
}

void ui::misc::About::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::BeginGroupPanel(lang.getTextCStr("About"));
    ImGui::Text("%s", lang.getTextCStr("ProjectAuthor"));
    ImGui::Text("%s", lang.getTextCStr("ProjectLink"));
    ImGui::EndGroupPanel();

    ImGui::BeginGroupPanel(lang.getTextCStr("Hotkey"));
    ImGui::Text("%s", lang.getTextCStr("HotkeyDesc"));
    ImGui::EndGroupPanel();
}
