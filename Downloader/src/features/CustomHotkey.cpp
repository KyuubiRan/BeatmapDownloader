#include "pch.h"
#include "CustomHotkey.h"

#include "Feature.h"
#include "ui/BeatmapIdSearchUi.h"
#include "ui/MainUi.h"

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
    // TODO: Custom Hotkey UI
}

FeatureInfo &CustomHotkey::getInfo() {
    static FeatureInfo info{
        "Hotkey", ""};
    return info;
}
}
