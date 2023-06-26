#include "pch.h"
#include "BeatmapIdSearchUi.h"

#include <regex>

#include "BlockingInput.hpp"
#include "config/I18nManager.h"
#include "features/Downloader.h"
#include "osu/LinkParser.hpp"
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

void ui::search::beatmapid::Update() {
    if (!show)
        return;

    auto &lang = i18n::I18nManager::GetInstance();

    constexpr static ImVec2 windowSize = ImVec2(425, 65);
    if (static bool inited = false; !inited) {
        const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        const ImVec2 windowPos(screenSize.x / 2 - windowSize.x / 2, screenSize.y / 2 - windowSize.y / 2);
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        inited = true;
    }

    ImGui::Begin(lang.getTextCStr("SearchBeatmapId"), nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
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
    if (ImGui::Button(lang.getTextCStr("Clear"))) {
        input = "";
    }
    ImGui::SameLine();
    if (ImGui::Button(lang.getTextCStr("Paste"))) {
        input = ImGui::GetClipboardText();
    }

    ImGui::SameLine();
    if (ImGui::Button(lang.getTextCStr("Search"))) {
        if (const auto bi = osu::LinkParser::ParseLink(input, (features::downloader::BeatmapType)selected); !bi) {
            GuiHelper::ShowWarnToast(lang.getTextCStr("InvalidInput"), input.c_str());
        } else {
            features::Downloader::GetInstance().postSearch(*bi);
        }
    }

    GuiHelper::ShowTooltip(lang.getTextCStr("SearchBeatmapIdDesc"));

    ImGui::End();
}
