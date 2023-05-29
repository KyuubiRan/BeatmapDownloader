#include "pch.h"
#include "Settings.h"

#include "utils/gui_utils.h"

ui::misc::Settings::Settings() : Feature() {
}

void ui::misc::Settings::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    static const char *langs[] =  {"English | en_us", "简体中文 | zh_cn" };
    static int currentLang = (int)lang.lang.getValue();
    if (ImGui::Combo(lang.GetTextCStr("Language"), &currentLang, langs , IM_ARRAYSIZE(langs))) {
        lang.lang = (i18n::Language)currentLang;
    }
}

ui::main::FeatureInfo & ui::misc::Settings::getInfo() {
    static auto info = main::FeatureInfo{
        .category = "Settings",
        .groupName = ""
    };
    return info;
}
