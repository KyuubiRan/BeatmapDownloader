#include "pch.h"
#include "Downloader.h"

#include "api/Sayobot.h"
#include "config/I18nManager.h"
#include "misc/Color.h"
#include "ui/SearchResultUi.h"
#include "utils/gui_utils.h"

#pragma comment(lib, "winhttp.lib")

features::Downloader::Downloader() :
    Feature(),
    f_GrantOsuAccount("GrantOsuAccount", false),
    f_Mirror("DownloadMirror", downloader::DownloadMirror::Sayobot),
    f_DownloadType("DownloadType", downloader::DownloadType::Full),
    f_EnableProxy("EnableProxy", false),
    f_ProxySever("ProxyServer", ""),
    f_ProxySeverPassword("ProxyServerPassword", ""),
    f_EnableCustomUserAgent("EnableCustomUserAgent", false),
    f_CustomUserAgent("CustomUserAgent",
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36") {
    t_DownloadThread = std::thread(DownloadThread);
    t_SearchThread = std::thread(SearchThread);

    t_DownloadThread.detach();
    t_SearchThread.detach();
}

[[noreturn]] void features::Downloader::DownloadThread() {
    auto &inst = GetInstance();

    while (true) {
        const auto id = inst.m_DownloadQueue.pop_front();
        if (id <= 0) {
            LOGW("Invalid beatmapsets id: %d, skipped download.", id);
            continue;
        }
        std::vector<BYTE> data;

        switch (inst.f_Mirror.getValue()) {
        case downloader::DownloadMirror::OsuOfficial:
            if (!inst.f_GrantOsuAccount.getValue()) {
                LOGW("Permission denied on download beatmap by osu!(Official) mirror.");
                continue;
            }

            break;
        case downloader::DownloadMirror::Sayobot:
            break;
        }

        LOGD("Finished download beatmapsets(id=%d) size=%zu", id, data.size());
    }
}

[[noreturn]] void features::Downloader::SearchThread() {
    auto &inst = GetInstance();

    while (true) {
        auto info = inst.m_SearchQueue.pop_front();
        switch (inst.f_Mirror.getValue()) {
        case downloader::DownloadMirror::OsuOfficial:
            break;
        case downloader::DownloadMirror::Sayobot:
            auto ret = api::sayobot::SearchBeatmapV2(info);
            if (ret.has_value()) {
                if (auto &sayo = ret.value(); sayo.status == 0 && sayo.data.has_value()) {
                    const auto &data = *sayo.data;
                    std::vector<int32_t> bids;
                    bids.reserve(data.bidData.size());
                    for (auto &bd : data.bidData) {
                        bids.push_back(bd.bid);
                    }
                    osu::Beatmap bm = {data.title, data.artist, data.creator, bids, data.sid};
                    ui::search::ShowSearchInfo(bm);
                } else {
                    LOGW("No such map found on Sayobot. ID=%d, Type=%s", info.id,
                         info.type == downloader::BeatmapType::Sid ? "beatmapsets" : "beatmapid");
                }
            }
            break;
        }
    }
}

void features::Downloader::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::Checkbox(lang.GetTextCStr("GrantOsuAccount"), f_GrantOsuAccount.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("GrantOsuAccountDesc"));

#pragma region Mirror
    static const char *mirrorNames[] = {
        "Osu! (official)",
        "Sayobot",
        // "Chimu",
    };
    static int mirrorIdx = (int)f_Mirror.getValue();
    if (ImGui::Combo(lang.GetTextCStr("Mirror"), &mirrorIdx, mirrorNames, IM_ARRAYSIZE(mirrorNames))) {
        f_Mirror.setValue(static_cast<downloader::DownloadMirror>(mirrorIdx));
    }
#pragma endregion

#pragma region DownloadType
    static const char *downloadTypeNames[] = {
        "Full",
        "NoVideo",
    };
    static int downloadTypeIdx = (int)f_DownloadType.getValue();
    if (ImGui::Combo(lang.GetTextCStr("DownloadType"), &downloadTypeIdx, downloadTypeNames, IM_ARRAYSIZE(downloadTypeNames))) {
        f_DownloadType.setValue(static_cast<downloader::DownloadType>(downloadTypeIdx));
    }
#pragma endregion

    if (f_Mirror.getValue() == downloader::DownloadMirror::OsuOfficial && !f_GrantOsuAccount.getValue()) {
        ImGui::TextColored(color::Orange, "%s", lang.GetTextCStr("NotGrantOsuAccountButUseOfficialWarn"));
    }

    ImGui::Checkbox(lang.GetTextCStr("EnableProxyServer"), f_EnableProxy.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("ProxyServerDesc"));

    if (f_EnableProxy.getValue()) {
        ImGui::Indent();
        ImGui::InputText(lang.GetTextCStr("ProxyServer"), f_ProxySever.getPtr());
        ImGui::InputText(lang.GetTextCStr("ProxyServerPassword"), f_ProxySeverPassword.getPtr());
        ImGui::Unindent();
    }

    ImGui::Checkbox(lang.GetTextCStr("EnableCustomUserAgent"), f_EnableCustomUserAgent.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("CustomUserAgentDesc"));
    if (f_EnableCustomUserAgent.getValue()) {
        ImGui::Indent();
        ImGui::InputText("User-Agent", f_CustomUserAgent.getPtr());
        ImGui::Unindent();
    }
}

ui::main::FeatureInfo& features::Downloader::getInfo() {
    static ui::main::FeatureInfo info = {"Downloader", "Download"};
    return info;
}

void features::Downloader::postSearch(const downloader::BeatmapInfo info) {
    m_SearchQueue.push_back(info);
}

void features::Downloader::postDownload(const int id) {
    m_DownloadQueue.push_back(id);
}
