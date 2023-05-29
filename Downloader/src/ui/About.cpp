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
    ImGui::Text("%s", lang.GetTextCStr("Author"));
    ImGui::Text("%s", lang.GetTextCStr("ProjectLink"));
}
