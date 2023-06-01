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
    if (!curShowBm) return;
    auto &bm = *curShowBm;
    ImGui::SetNextWindowSize(ImVec2(100, 200));
    auto &lang = i18n::I18nManager::GetInstance();
    if (ImGui::Begin(lang.GetTextCStr("BeatmapInfo"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
        ImGui::Indent();
        ImGui::Text(lang.GetTextCStr("Title"), bm.title.c_str());
        ImGui::Text(lang.GetTextCStr("Artist"), bm.artist.c_str());
        ImGui::Text(lang.GetTextCStr("Mapper"), bm.author.c_str());
        ImGui::Indent();

        ImGui::NewLine();
        ImGui::NewLine();

        const bool hasMap = osu::BeatmapManager::GetInstance().hasBeatmap(bm);
        
        bool downloadPressed;
        if (!hasMap) {
            downloadPressed = ImGui::Button(lang.GetTextCStr("Download"));
        } else {
            downloadPressed = ImGui::Button(lang.GetTextCStr("ReDownload"));
        }

        features::downloader::BeatmapInfo bi{.id = -1};
        if (bm.sid > 0) {
            bi = {features::downloader::BeatmapType::Sid, bm.sid};
        } else if (!bm.bid.empty()) {
            bi = {features::downloader::BeatmapType::Bid, bm.bid[0]};
        }

        if (downloadPressed) {
            features::Downloader::GetInstance().postDownload(bm.sid);
            isShow = false;
            InputBlock::Pop();
        }

        if (ImGui::Button(lang.GetTextCStr("ViewWebsite"))) {
            osu::BeatmapManager::GetInstance().openBeatmapPage(bi);
        }

        if (ImGui::Button(lang.GetTextCStr("Cancel"))) {
            curShowBm = {};
            isShow = false;
            InputBlock::Pop();
        }

        ImGui::Unindent();
        ImGui::Unindent();

        ImGui::End();
    }
}
