#include "pch.h"
#include "BeatmapIdSearchUi.h"

#include <regex>

#include "BlockingInput.hpp"
#include "config/I18nManager.h"
#include "features/Downloader.h"
#include "utils/gui_utils.h"

static bool show = false;

bool ui::search::beatmapid::IsShowed() {
    return show;
}

void ui::search::beatmapid::ToggleShow() {
    show = !show;
    show ? InputBlock::Push() : InputBlock::Pop();
}

void parseAndPostSearch(const std::string &s) {
    static const std::string regex1 = "^(beatmapset)?s?\\/?(\\d+)$";
    std::smatch m;
    if (const std::regex re1(regex1); std::regex_search(s, m, re1) && m.size() > 2) {
        try {
            features::Downloader::GetInstance().postSearch(features::downloader::BeatmapInfo{
                features::downloader::BeatmapType::Sid,
                std::stoi(m[2].str())
            });
        } catch (...) {
            LOGW("Invalid int value: %s", m[2].str().c_str());
        }
        return;
    }

    static const std::string regex2 = "^b(eatmaps)?\\/?(\\d+)$";
    if (const std::regex re2(regex2); std::regex_search(s, m, re2) && m.size() > 2) {
        try {
            features::Downloader::GetInstance().postSearch(features::downloader::BeatmapInfo{
                features::downloader::BeatmapType::Bid,
                std::stoi(m[2].str())
            });
        } catch (...) {
            LOGW("Invalid int value: %s", m[2].str().c_str());
        }
        return;
    }

    LOGI("Invalid input: %s", s.c_str());
}

void ui::search::beatmapid::Update() {
    if (!show)
        return;

    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::SetNextWindowSize(ImVec2(380, 65), ImGuiCond_FirstUseEver);
    ImGui::Begin(lang.GetTextCStr("SearchBeatmapId"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    static std::string input;
    ImGui::InputText("##search", &input);
    ImGui::SameLine();
    if (ImGui::Button(lang.GetTextCStr("Clear"))) {
        input = "";
    }
    ImGui::SameLine();
    if (ImGui::Button(lang.GetTextCStr("Search"))) {
        parseAndPostSearch(input);
    }
    GuiHelper::ShowTooltip(lang.GetTextCStr("SearchBeatmapIdDesc"));
    ImGui::End();
}
