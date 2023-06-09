#include "pch.h"
#include "MultiDownload.h"

#include "network/HttpRequest.h"
#include "osu/LinkParser.hpp"
#include "utils/gui_utils.h"

using namespace features;

MultiDownload::MultiDownload() :
    Feature() {
}

ui::main::FeatureInfo &MultiDownload::getInfo() {
    static auto info = ui::main::FeatureInfo{"MultiDL",
                                             ""};
    return info;
}

// Sid = 0, Bid = 1
int multiDLSelected = 0;

// osu = 0, taiko = 1, fruits = 2, mania = 3
int bpDownloadSelect = 0;

const char *bpDownloadSelectItems[] = {
    "osu",
    "taiko",
    "fruits",
    "mania"
};

void DoMultiLineDownload(const std::string &s) {
    const auto vec = utils::split(s, "\n");
    LOGD("DoMultiLineDownload split size = %zu", vec.size());

    for (auto &link : vec) {
        auto bi = osu::LinkParser::ParseLink(link, (downloader::BeatmapType)multiDLSelected);
        if (!bi.has_value())
            continue;

        auto &b = *bi;
        b.directDownload = true;
        Downloader::GetInstance().postSearch(b);
        LOGD("DoMultiLineDownload postSearch %s=%d", b.type == downloader::BeatmapType::Sid ? "s" : "b", b.id);
    }
}

void DoBPDownload(int id) {
    const std::string link = std::format("https://osu.ppy.sh/users/{}/scores/best?mode={}&limit=100", id,
                                         bpDownloadSelectItems[bpDownloadSelect]);
    std::string response;
    int code;
    if (net::curl_get(link.c_str(), response, &code) != CURLE_OK)
        return;

    auto &lang = i18n::I18nManager::GetInstance();

    switch (code) {
    case 200: {
        try {
            if (nlohmann::json j = nlohmann::json::parse(response); j.is_array() && !j.empty()) {
                GuiHelper::ShowInfoToast(lang.getTextCStr("StartDownloadBP"), id);

                downloader::BeatmapInfo bi{downloader::BeatmapType::Bid, -1, true};
                for (auto &i : j) {
                    bi.id = i["beatmap_id"];
                    LOGD("Post bp download: bid=%d", bi.id);
                    Downloader::GetInstance().postSearch(bi);
                }
            }
        } catch (...) {
            LOGW("Cannot parse bp download response!");
            GuiHelper::ShowWarnToast(lang.getTextCStr("BPDownloadFailedParseFailed"));
        }
        break;
    }
    case 404: {
        LOGW("BP Download failed: user(%d) not found!", id);
        GuiHelper::ShowWarnToast(lang.getTextCStr("BPDownloadFailedUserNotFound"), id);
        break;
    }
    default: {
        LOGW("BP Download failed with http code: %d", code);
        GuiHelper::ShowWarnToast(lang.getTextCStr("BPDownloadFailedResponseCodeNotOk"), code);
        break;
    }
    }
}

void MultiDownload::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
#pragma region Multi DL
    ImGui::BeginGroupPanel(lang.getTextCStr("MultiDownloader"));

    static const char *items[] = {"Sid", "Bid"};
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
    ImGui::Combo(lang.getTextCStr("IDSearchDefaultType"), &multiDLSelected, items, IM_ARRAYSIZE(items));
    static std::string multiDLText;

    ImGui::InputTextMultiline("##linklist", &multiDLText, ImVec2(0, 150));
    if (ImGui::Button(lang.getTextCStr("Download"))) {
        DoMultiLineDownload(multiDLText);
    }
    ImGui::SameLine();
    if (ImGui::Button(lang.getTextCStr("Clear"))) {
        multiDLText = "";
    }
    ImGui::Text(lang.getTextCStr("MultiDownloaderDesc"));

    ImGui::EndGroupPanel();
#pragma endregion

#pragma region BP DL
    ImGui::BeginGroupPanel(lang.getTextCStr("BPDownloader"));

    static int id = 0;
    ImGui::Combo(lang.getTextCStr("Mode"), &bpDownloadSelect, bpDownloadSelectItems, IM_ARRAYSIZE(bpDownloadSelectItems));
    ImGui::InputInt("UID", &id);

    if (ImGui::Button(lang.getTextCStr("Download"))) {
        DoBPDownload(id);
    }

    ImGui::EndGroupPanel();
#pragma endregion
}
