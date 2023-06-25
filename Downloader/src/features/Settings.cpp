#include "pch.h"
#include "Settings.h"

#include "utils/gui_utils.h"

namespace features {

static const char *s_Themes[] = {"Sakura", "ImGuiDark", "ImGuiLight", "ImGuiClassic"};

Settings::Settings() :
    Feature(),
    f_EnableToast("EnableToast", true),
    f_ToastDuration("ToastDuration", 3000),
    f_EnableConsole("EnableConsole", true),
    f_OsuPath("OsuPath", ""),
    f_Theme("Theme", 0) {
}

void Settings::applyTheme() {
    if (ImGui::GetCurrentContext() == nullptr) {
        LOGW("Apply theme failed: ImGui Context == nullptr");
        return;
    }
    ImGui::SetTheme(s_Themes[f_Theme.getValue()]);
}

void Settings::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();

    ImGui::BeginGroupPanel(lang.getTextCStr("Settings"));

    static const char *langs[] = {"English | en_us", "简体中文 | zh_cn"};

    static int langIdx = (int)lang.lang.getValue();
    if (ImGui::Combo(lang.getTextCStr("Language"), &langIdx, langs, IM_ARRAYSIZE(langs))) {
        lang.lang.setValue(static_cast<i18n::Language>(langIdx));
    }

    if (ImGui::Combo(lang.getTextCStr("Theme"), f_Theme.getPtr(), s_Themes, IM_ARRAYSIZE(s_Themes))) {
        ImGui::SetTheme(s_Themes[f_Theme.getValue()]);
    }

    if (ImGui::InputText(lang.getTextCStr("OsuPath"), f_OsuPath.getPtr())) {
        if (!f_OsuPath->empty()) {
            if (const auto op = std::filesystem::path(f_OsuPath.getValue()) / "osu!.exe"; exists(op)) {
                utils::SetOsuDirPath(op.parent_path());
            } else {
                GuiHelper::ShowWarnToast(lang.getTextCStr("InvalidOsuPath"));
            }
        }
    }
    GuiHelper::ShowTooltip(lang.getTextCStr("OsuPathDesc"));

    ImGui::EndGroupPanel();

    ImGui::BeginGroupPanel(lang.getTextCStr("Toast"));
    ImGui::Checkbox(lang.getTextCStr("EnableToast"), f_EnableToast.getPtr());
    if (f_EnableToast.getValue()) {
        ImGui::Indent();
        ImGui::DragInt(lang.getTextCStr("ToastDuration"), f_ToastDuration.getPtr(), 1.0f, 500, 10000, "%d ms", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Unindent();
    }
    ImGui::EndGroupPanel();

    ImGui::BeginGroupPanel(lang.getTextCStr("Debug"));
    ImGui::Checkbox(lang.getTextCStr("EnableConsole"), f_EnableConsole.getPtr());
    GuiHelper::ShowTooltip(lang.getTextCStr("RestartToApply"));
    ImGui::EndGroupPanel();
}

FeatureInfo &Settings::getInfo() {
    static auto info = FeatureInfo{
        .category = "Settings",
        .groupName = ""
    };
    return info;
}

}
