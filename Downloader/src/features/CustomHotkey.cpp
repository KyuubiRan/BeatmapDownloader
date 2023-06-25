#include "pch.h"
#include "CustomHotkey.h"

#include "Feature.h"
#include "ui/BeatmapIdSearchUi.h"
#include "ui/MainUi.h"
#include "utils/gui_utils.h"

namespace features {

CustomHotkey::CustomHotkey() :
    Feature(),
    f_MainMenuHotkey("MainMenuHotkey", {VK_HOME}),
    f_SearchHotkey("SearchHotkey", {VK_END}) {
    f_MainMenuHotkey.getValue() += [] {
        ui::main::ToggleShow();
    };
    f_SearchHotkey.getValue() += [] {
        ui::search::beatmapid::ToggleShow();
    };
    LOGI("Inited Hotkey");
}

void CustomHotkey::drawMain() {
    GET_LANG();
    ImGui::BeginGroupPanel(lang.getTextCStr("Main"));
    ImGui::HotkeyWidget(lang.getTextCStr("MainMenuHotkey"), f_MainMenuHotkey);
    ImGui::HotkeyWidget(lang.getTextCStr("IdSearchHotkey"), f_SearchHotkey);
    ImGui::EndGroupPanel();
}

FeatureInfo &CustomHotkey::getInfo() {
    static FeatureInfo info{
        "Hotkey", ""};
    return info;
}
}
