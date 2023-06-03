#include "pch.h"
#include "Downloader.h"

#include <fstream>
#include <set>
#include <shellapi.h>

#include "DownloadQueue.h"
#include "api/Sayobot.h"
#include "config/I18nManager.h"
#include "misc/Color.h"
#include "ui/SearchResultUi.h"
#include "utils/gui_utils.h"

static std::set<int> s_Canceled{};

features::Downloader::Downloader() :
    Feature(),
    f_GrantOsuAccount("GrantOsuAccount", false),
    f_Mirror("DownloadMirror", downloader::DownloadMirror::Sayobot),
    f_DownloadType("DownloadType", downloader::DownloadType::Full),
    f_ProxySeverType("ProxyServerType", downloader::ProxyServerType::Disabled),
    f_ProxySever("ProxyServer", "localhost:7890"),
    f_ProxySeverPassword("ProxyServerPassword", ""),
    f_EnableCustomUserAgent("EnableCustomUserAgent", false),
    f_CustomUserAgent("CustomUserAgent",
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36") {
    t_DownloadThread = std::thread(DownloadThread);
    t_SearchThread = std::thread(SearchThread);

    t_DownloadThread.detach();
    t_SearchThread.detach();
    LOGI("Initied Downloader");
}

[[noreturn]] void features::Downloader::DownloadThread() {
    auto &inst = GetInstance();

    while (true) {
    beg:
        auto bm = inst.m_DownloadQueue.pop_front();
        LOGD("Poped dl: %d", bm.sid);
        if (bm.sid <= 0) {
            LOGW("Invalid beatmapsets id: %d, skipped download.", bm.sid);
            continue;
        }
        if (s_Canceled.contains(bm.sid)) {
            LOGI("Canceled: %d, skipped download.", bm.sid);
            s_Canceled.erase(bm.sid);
            continue;
        }

        uint8_t retry = 0;
        bool success = false;
        LOGI("Start download: %d %s-%s (%s)", bm.sid, bm.artist.c_str(), bm.title.c_str(), bm.author.c_str());
        while (!success && ++retry < 3) {
            switch (inst.f_Mirror.getValue()) {
            case downloader::DownloadMirror::OsuOfficial:
                if (!inst.f_GrantOsuAccount.getValue()) {
                    LOGW("Permission denied on download beatmap by osu!(Official) mirror.");
                    goto beg;
                }

                break;
            case downloader::DownloadMirror::Sayobot:
                success = api::sayobot::DownloadBeatmap(bm);
                break;
            }
        }

        DownloadQueue::GetInstance().notifyFinished(bm.sid);

        if (!success) {
            LOGW("Download failed (out of retry times): %d", bm.sid);
            continue;
        }

        std::string fn = std::to_string(bm.sid) + ".osz";
        auto path = utils::GetCurrentDirPath() / L"downloads";
        if (!exists(path)) create_directory(path);
        path /= fn;

        if (!exists(path)) {
            LOGW("Cannot open file (file not exists): %s", path.string().c_str());
            continue;
        }

        ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_HIDE);

        LOGD("Finished download beatmapsets: %d", bm.sid);
    }
}

[[noreturn]] void features::Downloader::SearchThread() {
    auto &inst = GetInstance();

    while (true) {
        auto info = inst.m_SearchQueue.pop_front();
        LOGD("Poped search: %d", info.id);
        switch (inst.f_Mirror.getValue()) {
        case downloader::DownloadMirror::OsuOfficial:
            break;
        case downloader::DownloadMirror::Sayobot:
            auto ret = api::sayobot::SearchBeatmapV2(info);
            if (ret.has_value()) {
                if (auto &sayo = ret.value(); sayo.status == 0 && sayo.data.has_value()) {
                    auto bm = sayo.data->to_beatmap();
                    ui::search::result::ShowSearchInfo(bm);
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

    static const char *proxyTypeNames[] = {
        "Disabled",
        "HTTP",
        "Socks5"
    };
    static int proxyTypeIdx = (int)f_ProxySeverType.getValue();
    if (ImGui::Combo(lang.GetTextCStr("ProxyServerType"), &proxyTypeIdx, proxyTypeNames, IM_ARRAYSIZE(proxyTypeNames))) {
        f_ProxySeverType.setValue(static_cast<downloader::ProxyServerType>(proxyTypeIdx));
    };
    GuiHelper::ShowTooltip(lang.GetTextCStr("ProxyServerDesc"));

    if (f_ProxySeverType.getValue() != downloader::ProxyServerType::Disabled) {
        ImGui::Indent();
        ImGui::InputText(lang.GetTextCStr("ProxyServer"), f_ProxySever.getPtr());
        GuiHelper::ShowTooltip("e.g.: localhost:7890");
        ImGui::InputText(lang.GetTextCStr("ProxyServerPassword"), f_ProxySeverPassword.getPtr());
        GuiHelper::ShowTooltip("e.g.: username:password");
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

void features::Downloader::cancelDownload(int sid) {
    s_Canceled.insert(sid);
}

void features::Downloader::removeCancelDownload(int sid) {
    if (s_Canceled.contains(sid)) {
        s_Canceled.erase(sid);
    }
}

void features::Downloader::postSearch(const downloader::BeatmapInfo info) {
    m_SearchQueue.push_back(info);
}

void features::Downloader::postDownload(const osu::Beatmap &bm) {
    m_DownloadQueue.push_back(bm);
}
