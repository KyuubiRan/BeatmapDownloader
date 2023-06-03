#include "pch.h"
#include "SearchResultUi.h"

#include <optional>

#include "BlockingInput.hpp"
#include "config/I18nManager.h"
#include "features/Downloader.h"
#include "osu/BeatmapManager.h"
#include "utils/gui_utils.h"

static bool isShow = false;

bool ui::search::IsShowed() {
    return isShow;
}

std::optional<osu::Beatmap> curShowBm;

void ui::search::ShowSearchInfo(osu::Beatmap &bm) {
    curShowBm = bm;
    if (isShow) return;
    isShow = true;
    InputBlock::Push();
}

void ui::search::Update() {
    if (!isShow) return;
    if (!curShowBm.has_value()) return;

    auto &bm = *curShowBm;
    ImGui::SetNextWindowSize(ImVec2(300, 225), ImGuiCond_FirstUseEver);
    auto &lang = i18n::I18nManager::GetInstance();

    ImGui::Begin(lang.GetTextCStr("BeatmapInfo"), nullptr, ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text(lang.GetTextCStr("Title"), bm.title.c_str());
    ImGui::Text(lang.GetTextCStr("Artist"), bm.artist.c_str());
    ImGui::Text(lang.GetTextCStr("Mapper"), bm.author.c_str());

    ImGui::NewLine();

    const bool hasMap = osu::BeatmapManager::GetInstance().hasBeatmap(bm);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.GetTextCStr(hasMap ? "ReDownload" : "Download")).x) * 0.5f);
    if (ImGui::Button(lang.GetTextCStr(hasMap ? "ReDownload" : "Download"))) {
        features::Downloader::GetInstance().postDownload(bm);
        isShow = false;
        InputBlock::Pop();
    }

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.GetTextCStr("ViewWebsite")).x) * 0.5f);
    if (ImGui::Button(lang.GetTextCStr("ViewWebsite"))) {
        osu::BeatmapManager::GetInstance().openBeatmapPage(bm);
    }

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.GetTextCStr("Cancel")).x) * 0.5f);
    if (ImGui::Button(lang.GetTextCStr("Cancel"))) {
        curShowBm = {};
        isShow = false;
        InputBlock::Pop();
    }
    
    ImGui::End();
}
