#include "pch.h"
#include "SearchResultUi.h"

#include <optional>

#include "BlockingInput.hpp"
#include "config/I18nManager.h"
#include "features/Downloader.h"
#include "osu/BeatmapManager.h"
#include "utils/gui_utils.h"


namespace ui::search::result {

static bool isShow = false;

bool IsShowed() {
    return isShow;
}

std::optional<osu::Beatmap> curShowBm;

void ShowSearchInfo(osu::Beatmap &bm) {
    curShowBm = bm;
    if (isShow) return;
    isShow = true;
    InputBlock::Push();
}

void Update() {
    if (!isShow) return;
    if (!curShowBm.has_value()) return;

    auto &bm = *curShowBm;
    ImGui::SetNextWindowSize(ImVec2(300, 225), ImGuiCond_FirstUseEver);
    auto &lang = i18n::I18nManager::GetInstance();

    ImGui::Begin(lang.getTextCStr("BeatmapInfo"), nullptr, ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text(lang.getTextCStr("Title"), bm.title.c_str());
    ImGui::Text(lang.getTextCStr("Artist"), bm.artist.c_str());
    ImGui::Text(lang.getTextCStr("Mapper"), bm.author.c_str());

    ImGui::NewLine();

    const bool hasMap = osu::BeatmapManager::GetInstance().hasBeatmap(bm);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.getTextCStr(hasMap ? "ReDownload" : "Download")).x) * 0.5f);
    if (ImGui::Button(lang.getTextCStr(hasMap ? "ReDownload" : "Download"))) {
        features::Downloader::GetInstance().postDownload(bm);
        isShow = false;
        InputBlock::Pop();
    }

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.getTextCStr("ViewWebsite")).x) * 0.5f);
    if (ImGui::Button(lang.getTextCStr("ViewWebsite"))) {
        osu::BeatmapManager::GetInstance().openBeatmapPage(bm);
    }

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.getTextCStr("Cancel")).x) * 0.5f);
    if (ImGui::Button(lang.getTextCStr("Cancel"))) {
        curShowBm = {};
        isShow = false;
        InputBlock::Pop();
    }
    
    ImGui::End();
}
}


