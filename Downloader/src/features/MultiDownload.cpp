#include "pch.h"
#include "MultiDownload.h"

#include "network/HttpRequest.h"
#include "osu/LinkParser.hpp"
#include "utils/gui_utils.h"

using namespace features;

MultiDownload::MultiDownload() :
    Feature() {
    t_SearchThread = std::thread(SearchThread);
    t_SearchThread.detach();
}

void MultiDownload::doRequest(const Task &task) {
    m_Queue.push_back(task);
}

ui::main::FeatureInfo &MultiDownload::getInfo() {
    static auto info = ui::main::FeatureInfo{"MultiDL",
                                             ""};
    return info;
}

void clamp(int *i, int min, int max) {
    if (*i < min)
        *i = min;
    if (*i > max)
        *i = max;
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

void DoBPDownload(int id, int beg, int end) {
    // https://osu.ppy.sh/users/{}/scores/best?mode={}&limit={}&offset={}
    auto &lang = i18n::I18nManager::GetInstance();

    if (beg > end) {
        LOGW("Invalid download range: beg=%d, end=%d", beg, end);
        GuiHelper::ShowWarnToast(lang.getTextCStr("InvalidDLRange"), beg, end);
        return;
    }

    GuiHelper::ShowInfoToast(lang.getTextCStr("StartDownloadBP"), id, beg + 1, end);
    MultiDownload::Task req;
    req.url = std::format("https://osu.ppy.sh/users/{}/scores/best?mode={}&", id,
                          bpDownloadSelectItems[bpDownloadSelect]);
    req.id = id;
    req.beg = beg;
    req.end = end;
    req.callback = [&](const int code, std::string &response, MultiDownload::Task &thiz) {
        switch (code) {
        case 200: {
            try {
                if (nlohmann::json j = nlohmann::json::parse(response); j.is_array() && !j.empty()) {
                    if (j.size() < 100)
                        thiz.canceled = true;
                    downloader::BeatmapInfo bi{downloader::BeatmapType::Bid, -1, true};
                    for (auto &i : j) {
                        if (!i.contains("beatmap_id"))
                            continue;

                        bi.id = i["beatmap_id"];
                        LOGD("Post bp download: bid=%d", bi.id);
                        Downloader::GetInstance().postSearch(bi);
                    }
                }
            } catch (...) {
                LOGW("Cannot parse bp download response!");
                LOGD("Response: %s", response.c_str());
                thiz.canceled = true;
                GuiHelper::ShowWarnToast(lang.getTextCStr("BPDownloadFailedParseFailed"));
            }
            break;
        }
        case 404: {
            LOGW("BP Download failed: user(%d) not found!", thiz.id);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("BPDownloadFailedUserNotFound"), thiz.id);
            break;
        }
        default: {
            LOGW("BP Download failed with http code: %d", code);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("BPDownloadFailedResponseCodeNotOk"), code);
            break;
        }
        }
    };

    MultiDownload::GetInstance().doRequest(req);
}

void DoFavDownload(int id, int beg, int end) {
    // https://osu.ppy.sh/users/{}/beatmapsets/favourite?limit={}&offset={}
    auto &lang = i18n::I18nManager::GetInstance();

    if (beg > end) {
        LOGW("Invalid download range: beg=%d, end=%d", beg, end);
        GuiHelper::ShowWarnToast(lang.getTextCStr("InvalidDLRange"), beg + 1, end);
        return;
    }

    GuiHelper::ShowInfoToast(lang.getTextCStr("StartDownloadFav"), id, beg + 1, end);

    MultiDownload::Task req;
    req.url = std::format("https://osu.ppy.sh/users/{}/beatmapsets/favourite?", id);
    req.beg = beg;
    req.end = end;
    req.id = id;
    req.callback = [&](const int code, std::string &response, MultiDownload::Task &thiz) {
        switch (code) {
        case 200: {
            try {
                if (nlohmann::json j = nlohmann::json::parse(response); j.is_array() && !j.empty()) {
                    if (j.size() < 100)
                        thiz.canceled = true;
                    downloader::BeatmapInfo bi{downloader::BeatmapType::Sid, -1, true};
                    for (auto &i : j) {
                        if (!i.contains("id"))
                            continue;

                        bi.id = i["id"];
                        LOGD("Post bp download: bid=%d", bi.id);
                        Downloader::GetInstance().postSearch(bi);
                    }
                }
            } catch (...) {
                LOGW("Cannot parse Fav download response!");
                LOGD("Response: %s", response.c_str());
                thiz.canceled = true;
                GuiHelper::ShowWarnToast(lang.getTextCStr("FavDownloadFailedParseFailed"));
            }
            break;
        }
        case 404: {
            LOGW("Fav Download failed: user(%d) not found!", thiz.id);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("FavDownloadFailedUserNotFound"), thiz.id);
            break;
        }
        default: {
            LOGW("Fav Download failed with http code: %d", code);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("FavDownloadFailedResponseCodeNotOk"), code);
            break;
        }
        }
    };

    MultiDownload::GetInstance().doRequest(req);
}

enum class BeatmapStatus : int {
    Ranked = 0,
    Loved,
    Guest,
    Pending,
    Graveyard,
    Nominated,
};

static const char *s_StatusStr[] = {"ranked",
                                    "loved",
                                    "guest",
                                    "pending",
                                    "graveyard",
                                    "nominated"};

void DoMapperDownload(int id, int beg, int end, const BeatmapStatus &stat) {
    // https://osu.ppy.sh/users/{}/beatmapsets/{}?limit={}&offset={}

    auto &lang = i18n::I18nManager::GetInstance();

    if (beg > end) {
        LOGW("Invalid bp download range: beg=%d, end=%d", beg + 1, end);
        GuiHelper::ShowWarnToast(lang.getTextCStr("InvalidDLRange"), beg + 1, end);
        return;
    }

    GuiHelper::ShowInfoToast(lang.getTextCStr("StartDownloadMapper"), id, beg + 1, end, s_StatusStr[(int)stat]);

    MultiDownload::Task req;
    req.url = std::format("https://osu.ppy.sh/users/{}/beatmapsets/{}?", id,
                          s_StatusStr[(int)stat]);
    req.beg = beg;
    req.end = end;
    req.id = id;
    req.callback = [&](const int code, std::string &response, MultiDownload::Task &thiz) {
        switch (code) {
        case 200: {
            try {
                if (nlohmann::json j = nlohmann::json::parse(response); j.is_array() && !j.empty()) {
                    if (j.size() < 100)
                        thiz.canceled = true;

                    downloader::BeatmapInfo bi{downloader::BeatmapType::Sid, -1, true};
                    for (auto &i : j) {
                        if (!i.contains("id"))
                            continue;

                        bi.id = i["id"];
                        LOGD("Post Mapper download: bid=%d", bi.id);
                        Downloader::GetInstance().postSearch(bi);
                    }
                }
            } catch (...) {
                LOGW("Cannot parse Mapper download response!");
                LOGD("Response: %s", response.c_str());
                thiz.canceled = true;
                GuiHelper::ShowWarnToast(lang.getTextCStr("MapperDownloadFailedParseFailed"));
            }
            break;
        }
        case 404: {
            LOGW("Mapper Download failed: user(%d) not found!", thiz.id);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("MapperDownloadFailedUserNotFound"), thiz.id);
            break;
        }
        default: {
            LOGW("Mapper Download failed with http code: %d", code);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("MapperDownloadFailedResponseCodeNotOk"), code);
            break;
        }
        }
    };

    MultiDownload::GetInstance().doRequest(req);
}

void DoMostPlayedDownload(int id, int beg, int end) {
    // https://osu.ppy.sh/users/{}/beatmapsets/most_played?

    auto &lang = i18n::I18nManager::GetInstance();

    if (beg > end) {
        LOGW("Invalid bp download range: beg=%d, end=%d", beg + 1, end);
        GuiHelper::ShowWarnToast(lang.getTextCStr("InvalidDLRange"), beg + 1, end);
        return;
    }

    GuiHelper::ShowInfoToast(lang.getTextCStr("StartDownloadMostPlayed"), id, beg + 1, end);

    MultiDownload::Task req;
    req.url = std::format("https://osu.ppy.sh/users/{}/beatmapsets/most_played?", id);
    req.beg = beg;
    req.end = end;
    req.id = id;
    req.callback = [&](const int code, std::string &response, MultiDownload::Task &thiz) {
        switch (code) {
        case 200: {
            try {
                if (nlohmann::json j = nlohmann::json::parse(response); j.is_array() && !j.empty()) {
                    if (j.size() < 100)
                        thiz.canceled = true;
                    downloader::BeatmapInfo bi{downloader::BeatmapType::Bid, -1, true};
                    for (auto &i : j) {
                        if (!i.contains("beatmap_id"))
                            continue;

                        bi.id = i["beatmap_id"];
                        LOGD("Post MostPlayed download: bid=%d", bi.id);
                        Downloader::GetInstance().postSearch(bi);
                    }
                }
            } catch (...) {
                LOGW("Cannot parse most played download response!");
                LOGD("Response: %s", response.c_str());
                thiz.canceled = true;
                GuiHelper::ShowWarnToast(lang.getTextCStr("MostPlayedDownloadFailedParseFailed"));
            }
            break;
        }
        case 404: {
            LOGW("Most played Download failed: user(%d) not found!", thiz.id);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("MostPlayedDownloadFailedUserNotFound"), thiz.id);
            break;
        }
        default: {
            LOGW("Most played Download failed with http code: %d", code);
            thiz.canceled = true;
            GuiHelper::ShowWarnToast(lang.getTextCStr("MostPlayedDownloadFailedResponseCodeNotOk"), code);
            break;
        }
        }
    };

    MultiDownload::GetInstance().doRequest(req);
}

void MultiDownload::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
#pragma region Multi DL
    {
        ImGui::BeginGroupPanel(lang.getTextCStr("MultiDownloader"));

        static const char *idtypes[] = {"Sid", "Bid"};
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
        ImGui::Combo(lang.getTextCStr("IDSearchDefaultType"), &multiDLSelected, idtypes, IM_ARRAYSIZE(idtypes));
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
    }
#pragma endregion

#pragma region BP DL
    {
        ImGui::BeginGroupPanel(lang.getTextCStr("BPDownloader"));

        static int id = 0;
        ImGui::Combo(lang.getTextCStr("Mode"), &bpDownloadSelect, bpDownloadSelectItems, IM_ARRAYSIZE(bpDownloadSelectItems));
        ImGui::InputInt("UID", &id);

        static int begend[] = {0, 100};
        ImGui::DragInt2(lang.getTextCStr("Range"), begend, 1, 1, 100, "%d");
        // GuiHelper::ShowTooltip(lang.getTextCStr("BPDLRangeDesc"));
        clamp(begend, 1, begend[1]);
        clamp(begend + 1, begend[0], 100);

        if (ImGui::Button(lang.getTextCStr("Download"))) {
            DoBPDownload(id, begend[0] - 1, begend[1]);
        }

        ImGui::EndGroupPanel();
    }
#pragma endregion

#pragma region Fav DL
#define MAX_FAV_DL_COUNT 300
    {
        ImGui::BeginGroupPanel(lang.getTextCStr("FavoriteDownloader"));

        static int id = 0;
        ImGui::InputInt("UID", &id);

        static int begend[] = {1, 100};
        ImGui::DragInt2(lang.getTextCStr("Range"), begend, 1, 1, MAX_FAV_DL_COUNT, "%d");
        clamp(begend, 1, begend[1]);
        clamp(begend + 1, begend[0], MAX_FAV_DL_COUNT);

        if (ImGui::Button(lang.getTextCStr("Download"))) {
            DoFavDownload(id, begend[0] - 1, begend[1]);
        }
        ImGui::EndGroupPanel();
    }
#undef MAX_FAV_DL_COUNT
#pragma endregion

#pragma region Mapper DL
#define MAX_MAPPER_DL_COUNT 99999
    {
        ImGui::BeginGroupPanel(lang.getTextCStr("MapperDownloader"));

        static BeatmapStatus stat = BeatmapStatus::Ranked;
        ImGui::Combo(lang.getTextCStr("BeatmapStatus"), (int *)&stat, s_StatusStr, IM_ARRAYSIZE(s_StatusStr));
        GuiHelper::ShowTooltip(lang.getTextCStr("BeatmapStatusDesc"));

        static int id = 0;
        ImGui::InputInt("UID", &id);

        static int begend[] = {1, 100};
        ImGui::DragInt2(lang.getTextCStr("Range"), begend, 1, 1, MAX_MAPPER_DL_COUNT, "%d");
        clamp(begend, 1, begend[1]);
        clamp(begend + 1, begend[0], MAX_MAPPER_DL_COUNT);

        if (ImGui::Button(lang.getTextCStr("Download"))) {
            DoMapperDownload(id, begend[0] - 1, begend[1], stat);
        }

        ImGui::EndGroupPanel();
    }
#undef MAX_MAPPER_DL_COUNT
#pragma endregion

#pragma region Most Play DL
#define MAX_MOST_PLAY_DL_COUNT 99999
    {
        ImGui::BeginGroupPanel(lang.getTextCStr("MostPlayedDownloader"));

        static int id = 0;
        ImGui::InputInt("UID", &id);

        static int begend[] = {1, 100};
        ImGui::DragInt2(lang.getTextCStr("Range"), begend, 1, 1, MAX_MOST_PLAY_DL_COUNT, "%d");
        clamp(begend, 1, begend[1]);
        clamp(begend + 1, begend[0], MAX_MOST_PLAY_DL_COUNT);
        ImGui::EndGroupPanel();

        if (ImGui::Button(lang.getTextCStr("Download"))) {
            DoMostPlayedDownload(id, begend[0] - 1, begend[1]);
        }
    }
#undef MAX_MOST_PLAY_DL_COUNT
#pragma endregion
}

[[noreturn]] void MultiDownload::SearchThread() {
    // auto &lang = i18n::I18nManager::GetInstance();
    auto &inst = GetInstance();
    while (true) {
        auto tsk = inst.m_Queue.pop_front();

        int limit = tsk.end - tsk.beg;

        int code = -1;
        std::string response;

        if (limit <= 100) {
            auto url = tsk.url;
            url.append("limit=").append(std::to_string(limit)).append("&offset=").append(std::to_string(tsk.beg));
            net::curl_get(url.c_str(), response, &code);

            tsk.callback(code, response, tsk);
        } else {
            const int count = (tsk.end - tsk.beg) / 100 + 1;
            int offset = tsk.beg;
            for (int i = 0; i < count; ++i) {
                response.clear();
                code = -1;
                auto url = tsk.url;
                const int curLimit = limit < 100 ? limit : 100;
                url.append("limit=").append(std::to_string(curLimit)).append("&offset=").append(std::to_string(offset));
                net::curl_get(url.c_str(), response, &code);
                tsk.callback(code, response, tsk);
                if (tsk.canceled) {
                    LOGD("MultiDL search canceled");
                    break;
                }

                offset += 100;
                limit -= 100;
            }
        }
        LOGI("MultiDL search finished");
    }
}
