#include "pch.h"
#include "MainUi.h"
#include "Feature.h"
#include "config/I18nManager.h"

#include <ranges>
#include <set>

#include "utils/gui_utils.h"

namespace ui::main {
static bool show = true;
static std::unordered_map<std::string_view, std::set<Feature*>> s_Features;
static const std::string_view *s_CurrentSelectedCategory = nullptr;

bool IsShowed() {
    return show;
}

void ToggleShow() {
    show = !show;
}

void Update() {
    ImGuiIO &io = ImGui::GetIO();
    i18n::I18nManager &lang = i18n::I18nManager::GetInstance();
    io.MouseDrawCursor = show;
    if (!show) return;

    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(lang.GetTextCStr("OsuBeatmapDownloader"), nullptr, ImGuiWindowFlags_None)) {
        ImGui::End();
        return;
    }

    ImGui::BeginGroup();

    // Draw category
    if (ImGui::BeginListBox("##feature category list", ImVec2(100, -FLT_MIN))) {
        for (const auto &category : s_Features | std::views::keys) {
            const bool isSelected = s_CurrentSelectedCategory == &category;

            if (ImGui::Selectable(lang.GetTextCStr(category), isSelected)) {
                s_CurrentSelectedCategory = &category;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("ChildR", ImVec2(0, 0), true, window_flags);

    // Draw group
    if (s_CurrentSelectedCategory) {
        for (auto &needDrawFeatures = s_Features[*s_CurrentSelectedCategory]; const auto feature :
             std::ranges::reverse_view(needDrawFeatures)) {
            const auto &info = feature->getInfo();
            auto group = lang.GetText(info.groupName);
            
            if (group.empty()) {
                feature->drawMain();
            } else {
                ImGui::BeginGroupPanel(group.data());

                ImGui::PushID(feature);
                feature->drawMain();
                ImGui::PopID();

                ImGui::EndGroupPanel();
            }
            
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::EndGroup();

    ImGui::End();
}

void AddFeature(Feature *feature) {
    auto &info = feature->getInfo();
    if (!s_Features.contains(info.category))
        s_Features[info.category] = {};
    s_Features[info.category].insert(feature);
}
}
