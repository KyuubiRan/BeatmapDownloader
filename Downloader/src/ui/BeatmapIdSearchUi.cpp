#include "pch.h"
#include "BeatmapIdSearchUi.h"

#include <regex>

#include "BlockingInput.hpp"
#include "config/I18nManager.h"
#include "features/Downloader.h"
#include "utils/gui_utils.h"

static bool show = false;

static bool focus = false;

bool ui::search::beatmapid::IsShowed() {
    return show;
}

void ui::search::beatmapid::ToggleShow() {
    show = !show;
    focus = false;
    show ? InputBlock::Push() : InputBlock::Pop();
}

// 0 = Sid, 1 = Bid
static int selected = 0;

void parseAndPostSearch(std::string *s) {
    static const std::string regex0 = "^(\\d+)$";
    std::smatch m;
    if (const std::regex re0(regex0); std::regex_search(*s, m, re0) && m.size() > 1) {
        try {
            features::Downloader::GetInstance().postSearch(features::downloader::BeatmapInfo{
                selected == 0 ? features::downloader::BeatmapType::Sid : features::downloader::BeatmapType::Bid,
                std::stoi(m[1].str())
            });
        } catch (...) {
            LOGW("Invalid int value: %s", m[1].str().c_str());
        }
        return;
    }

    std::string s2;
    std::regex_replace(std::back_inserter(s2), s->begin(), s->end(), std::regex(R"((https?:\/\/)?osu\.ppy\.sh\/)"), "");
    s = &s2;

    static const std::string regex1 = "^(beatmapset)?s\\/?(\\d+)$";
    if (const std::regex re1(regex1); std::regex_search(*s, m, re1) && m.size() > 2) {
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
    if (const std::regex re2(regex2); std::regex_search(*s, m, re2) && m.size() > 2) {
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

    LOGI("Invalid input: %s", s->c_str());
}

void ui::search::beatmapid::Update() {
    if (!show)
        return;

    auto &lang = i18n::I18nManager::GetInstance();

    constexpr static ImVec2 windowSize = ImVec2(400, 65);
    if (static bool inited = false; !inited) {
        const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        const ImVec2 windowPos(screenSize.x / 2 - windowSize.x / 2, screenSize.y / 2 - windowSize.y / 2);
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        inited = true;
    }

    ImGui::Begin(lang.GetTextCStr("SearchBeatmapId"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    static std::string input;
    static const char *items[] = {"Sid", "Bid"};

    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
    ImGui::Combo("##type", &selected, items, IM_ARRAYSIZE(items));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10);
    if (!focus) {
        ImGui::SetKeyboardFocusHere();
        focus = true;
    }
    ImGui::InputText("##search", &input);

    ImGui::SameLine();
    if (ImGui::Button(lang.GetTextCStr("Clear"))) {
        input = "";
    }

    ImGui::SameLine();
    if (ImGui::Button(lang.GetTextCStr("Search"))) {
        parseAndPostSearch(&input);
    }

    GuiHelper::ShowTooltip(lang.GetTextCStr("SearchBeatmapIdDesc"));

    ImGui::End();
}
