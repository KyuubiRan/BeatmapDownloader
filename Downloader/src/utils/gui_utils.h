#pragma once

#include <imgui.h>
#include <imgui_notify.h>

#include "features/Settings.h"

namespace GuiHelper {
void ShowTooltip(const char *s, bool marked = true);

template <typename... Fmt>
void ShowToast(const char *s, Fmt... fmt) {
    auto &settings = features::Settings::GetInstance();
    if (!settings.f_EnableToast.getValue())
        return;

    ImGui::InsertNotification({ImGuiToastType_None, settings.f_ToastDuration.getValue(), s, fmt...});
}

template <typename... Fmt>
void ShowWarnToast(const char *s, Fmt... fmt) {
    auto &settings = features::Settings::GetInstance();
    if (!settings.f_EnableToast.getValue())
        return;

    ImGuiToast t{ImGuiToastType_Warning, settings.f_ToastDuration.getValue(), s, fmt...};
    t.set_title(i18n::I18nManager::GetInstance().getTextCStr("Warning"));

    ImGui::InsertNotification(t);
}

template <typename... Fmt>
void ShowSuccessToast(const char *s, Fmt... fmt) {
    auto &settings = features::Settings::GetInstance();
    if (!settings.f_EnableToast.getValue())
        return;

    ImGuiToast t{ImGuiToastType_Success, settings.f_ToastDuration.getValue(), s, fmt...};
    t.set_title(i18n::I18nManager::GetInstance().getTextCStr("Success"));

    ImGui::InsertNotification(t);
}

template <typename... Fmt>
void ShowErrorToast(const char *s, Fmt... fmt) {
    auto &settings = features::Settings::GetInstance();
    if (!settings.f_EnableToast.getValue())
        return;

    ImGuiToast t{ImGuiToastType_Error, settings.f_ToastDuration.getValue(), s, fmt...};
    t.set_title(i18n::I18nManager::GetInstance().getTextCStr("Error"));

    ImGui::InsertNotification(t);
}

template <typename... Fmt>
void ShowInfoToast(const char *s, Fmt... fmt) {
    auto &settings = features::Settings::GetInstance();
    if (!settings.f_EnableToast.getValue())
        return;

    ImGuiToast t{ImGuiToastType_Info, settings.f_ToastDuration.getValue(), s, fmt...};
    t.set_title(i18n::I18nManager::GetInstance().getTextCStr("Info"));

    ImGui::InsertNotification(t);
}

inline void RenderToast() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
    ImGui::RenderNotifications();
    ImGui::PopStyleVar(1); // Don't forget to Pop()
    ImGui::PopStyleColor(1);
}

}

namespace ImGui {

bool Splitter(bool split_vertically, float thickness, float *size1, float *size2, float min_size1, float min_size2,
              float splitter_long_axis_size = -1.0f);

bool BeginGroupPanel(const char *label, bool node = false, const ImVec2 &size = ImVec2(-1.0f, 0.0f));
void EndGroupPanel();

bool PasswordInputText(const char *label, std::string *s, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
                       void *user_data = nullptr);

void TextUrl(const char *url);

bool HotkeyWidget(const char *label, misc::Hotkey &hotkey, const ImVec2 &size = ImVec2(0, 0));

void SetTheme(std::string_view theme);

}
