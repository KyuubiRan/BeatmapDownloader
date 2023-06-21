#include "pch.h"
#include "Downloader.h"

#include <set>
#include <shellapi.h>

#include "DownloadQueue.h"
#include "HandleLinkHook.h"
#include "api/Bancho.h"
#include "api/Sayobot.h"
#include "config/I18nManager.h"
#include "misc/Color.h"
#include "osu/BeatmapManager.h"
#include "ui/SearchResultUi.h"
#include "utils/gui_utils.h"

static std::set<int> s_Canceled{};

features::Downloader::Downloader() :
    Feature(),
    f_GrantOsuAccount("GrantOsuAccount", false),
    f_OsuAccount("OsuAccount", osu::Account{}),
    f_Mirror("DownloadMirror", downloader::DownloadMirror::Sayobot),
    f_DownloadType("DownloadType", downloader::DownloadType::Full),
    f_MoveToOsuFolder("MoveToOsuFolder", false),
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
    auto &lang = i18n::I18nManager::GetInstance();

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
        GuiHelper::ShowInfoToast(lang.getTextCStr("StartDownload"), bm.sid);
        while (!success && retry++ < 3) {
            switch (inst.f_Mirror.getValue()) {
            case downloader::DownloadMirror::OsuOfficial: {
                if (!inst.f_GrantOsuAccount.getValue()) {
                    LOGW("Permission denied on download beatmap by osu!(Official) mirror.");
                    goto beg;
                }
                success = api::bancho::DownloadBeatmap(bm);
            }
            break;
            case downloader::DownloadMirror::Sayobot: {
                success = api::sayobot::DownloadBeatmap(bm);
            }
            break;
            }
        }

        DownloadQueue::GetInstance().notifyFinished(bm.sid);

        if (!success) {
            LOGW("Download failed (out of retry times): %d", bm.sid);
            GuiHelper::ShowWarnToast(lang.getTextCStr("DownloadFailedOORT"), bm.sid);
            continue;
        }

        std::string fn = std::to_string(bm.sid) + ".osz";
        auto path = utils::GetCurrentDirPath() / L"downloads";
        if (!exists(path))
            create_directories(path);

        path /= fn;

        if (!exists(path)) {
            LOGW("Cannot open file (file not exists): %s", path.string().c_str());
            continue;
        }

        if (inst.f_MoveToOsuFolder.getValue()) {
            auto song = utils::GetOsuDirPath() / "Songs";
            if (!exists(song))
                create_directories(song);
            song /= fn;

            if (exists(song))
                remove(song);

            MoveFileW(path.wstring().c_str(), song.wstring().c_str());
        } else {
            ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_HIDE);
        }

        osu::BeatmapManager::GetInstance().addBeatmap(bm);

        LOGD("Finished download beatmapsets: %d", bm.sid);
        GuiHelper::ShowSuccessToast(lang.getTextCStr("DownloadSuccess"), bm.sid);
    }
}

[[noreturn]] void features::Downloader::SearchThread() {
    auto &inst = GetInstance();
    auto &lang = i18n::I18nManager::GetInstance();

    while (true) {
        auto info = inst.m_SearchQueue.pop_front();
        LOGD("Poped search: %d", info.id);

        switch (inst.f_Mirror.getValue()) {
        case downloader::DownloadMirror::OsuOfficial: {
            if (auto ret = api::bancho::SearchBeatmap(info); ret.has_value()) {
                auto &bm = *ret;
                if (osu::BeatmapManager::GetInstance().hasBeatmap(bm) && info.directDownload) {
                    LOGI("Already has beatmapsets: %d, skipped direct download.", bm.sid);
                    GuiHelper::ShowInfoToast(lang.getTextCStr("ExistsBeatmapSkipAutoDownload"), bm.sid);
                    break;
                }

                info.directDownload ? inst.postDownload(bm) : ui::search::result::ShowSearchInfo(bm);
            } else {
                LOGW("No such map found on bancho. ID=%d, Type=%s", info.id,
                     info.type == downloader::BeatmapType::Sid ? "beatmapsets" : "beatmapid");
                GuiHelper::ShowWarnToast(lang.getTextCStr("SearchFailed"), info.type == downloader::BeatmapType::Sid ? "sid" : "bid",
                                         info.id);
            }
        }
        break;
        case downloader::DownloadMirror::Sayobot: {
            if (auto ret = api::sayobot::SearchBeatmapV2(info); ret.has_value()) {
                if (auto &sayo = ret.value(); sayo.status == 0 && sayo.data.has_value()) {
                    auto bm = sayo.data->to_beatmap();
                    if (osu::BeatmapManager::GetInstance().hasBeatmap(bm) && info.directDownload) {
                        LOGI("Already has beatmapsets: %d, skipped direct download.", bm.sid);
                        GuiHelper::ShowInfoToast(lang.getTextCStr("ExistsMapSkipAutoDownload"), bm.sid);
                        break;
                    }

                    info.directDownload ? inst.postDownload(bm) : ui::search::result::ShowSearchInfo(bm);
                } else {
                    LOGW("No such map found on Sayobot. ID=%d, Type=%s", info.id,
                         info.type == downloader::BeatmapType::Sid ? "beatmapsets" : "beatmapid");
                    GuiHelper::ShowWarnToast(lang.getTextCStr("SearchFailed"), info.type == downloader::BeatmapType::Sid ? "sid" : "bid",
                                             info.id);
                }
            }
        }
        break;
        }
    }
}

void features::Downloader::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::Checkbox(lang.getTextCStr("GrantOsuAccount"), f_GrantOsuAccount.getPtr());
    GuiHelper::ShowTooltip(lang.getTextCStr("GrantOsuAccountDesc"));

    if (f_GrantOsuAccount.getValue()) {
        static std::string un = f_OsuAccount->username();
        static std::string pw = f_OsuAccount->password().md5();
        ImGui::Indent();
        if (ImGui::InputText(lang.getTextCStr("Username"), &un)) {
            f_OsuAccount->setUsername(un);
        }
        if (ImGui::PasswordInputText(lang.getTextCStr("Password"), &pw)) {
            f_OsuAccount->setPassword(pw);
        }
        ImGui::Unindent();
    }

    ImGui::Checkbox(lang.GetTextCStr("MoveToOsuFolder"), f_MoveToOsuFolder.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("MoveToOsuFolderDesc"));

#pragma region Mirror
    static const char *mirrorNames[] = {
        "Osu! (official)",
        "Sayobot",
        // "Chimu",
    };
    static int mirrorIdx = (int)f_Mirror.getValue();
    if (ImGui::Combo(lang.getTextCStr("Mirror"), &mirrorIdx, mirrorNames, IM_ARRAYSIZE(mirrorNames))) {
        f_Mirror.setValue(static_cast<downloader::DownloadMirror>(mirrorIdx));
    }
#pragma endregion

#pragma region DownloadType
    static const char *downloadTypeNames[] = {
        "Full",
        "NoVideo",
    };
    static int downloadTypeIdx = (int)f_DownloadType.getValue();
    if (ImGui::Combo(lang.getTextCStr("DownloadType"), &downloadTypeIdx, downloadTypeNames, IM_ARRAYSIZE(downloadTypeNames))) {
        f_DownloadType.setValue(static_cast<downloader::DownloadType>(downloadTypeIdx));
    }
#pragma endregion

    if (f_Mirror.getValue() == downloader::DownloadMirror::OsuOfficial && !f_GrantOsuAccount.getValue()) {
        ImGui::TextColored(color::Orange, "%s", lang.getTextCStr("NotGrantOsuAccountButUseOfficialWarn"));
    }

    static const char *proxyTypeNames[] = {
        "Disabled",
        "HTTP",
        "Socks5"
    };
    static int proxyTypeIdx = (int)f_ProxySeverType.getValue();
    if (ImGui::Combo(lang.getTextCStr("ProxyServerType"), &proxyTypeIdx, proxyTypeNames, IM_ARRAYSIZE(proxyTypeNames))) {
        f_ProxySeverType.setValue(static_cast<downloader::ProxyServerType>(proxyTypeIdx));
    };
    GuiHelper::ShowTooltip(lang.getTextCStr("ProxyServerDesc"));

    if (f_ProxySeverType.getValue() != downloader::ProxyServerType::Disabled) {
        ImGui::Indent();
        ImGui::InputText(lang.getTextCStr("ProxyServer"), f_ProxySever.getPtr());
        GuiHelper::ShowTooltip("e.g.: localhost:7890");
        ImGui::InputText(lang.getTextCStr("ProxyServerPassword"), f_ProxySeverPassword.getPtr());
        GuiHelper::ShowTooltip("e.g.: username:password");
        ImGui::Unindent();
    }

    ImGui::Checkbox(lang.getTextCStr("EnableCustomUserAgent"), f_EnableCustomUserAgent.getPtr());
    GuiHelper::ShowTooltip(lang.getTextCStr("CustomUserAgentDesc"));
    if (f_EnableCustomUserAgent.getValue()) {
        ImGui::Indent();
        ImGui::InputText("User-Agent", f_CustomUserAgent.getPtr());
        ImGui::Unindent();
    }
}

ui::main::FeatureInfo &features::Downloader::getInfo() {
    static FeatureInfo info = {"Downloader", "Download"};
    return info;
}

void features::Downloader::CancelDownload(int sid) {
    s_Canceled.insert(sid);
}

void features::Downloader::RemoveCancelDownload(int sid) {
    if (s_Canceled.contains(sid)) {
        s_Canceled.erase(sid);
    }
}

void features::Downloader::postSearch(const downloader::BeatmapInfo &info) {
    m_SearchQueue.push_back(info);
}

void features::Downloader::postDownload(const osu::Beatmap &bm) {
    if (bm.sid <= 0) {
        LOGW("Invalid beatmapsets id: %d, skipped download.", bm.sid);
        return;
    }

    if (!DownloadQueue::GetInstance().addTask(bm))
        return;
    
    m_DownloadQueue.push_back(bm);
}
