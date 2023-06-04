#pragma once
#include "config/Field.h"
#include "ui/Feature.h"
#include <BlockingContainer.hpp>
#include <deque>
#include <thread>

#include "osu/Beatmap.h"

namespace features {
namespace downloader {

enum class DownloadMirror : uint8_t {
    OsuOfficial = 0,
    Sayobot,
    // Chimu,
};

enum class BeatmapType : uint8_t {
    Sid = 0,
    Bid,
};

enum class DownloadType : uint8_t {
    Full = 0,
    NoVideo
};

struct BeatmapInfo {
    BeatmapType type;
    int32_t id;
    bool directDownload = false;
};

struct DownloadInfo {
    DownloadType type;
    int32_t id;
};

enum class ProxyServerType : uint8_t {
    Disabled = 0,
    Http,
    Socks5,
};

}


class Downloader : public ui::main::Feature {
    Downloader();

    container::Blocking<std::deque<osu::Beatmap>> m_DownloadQueue;
    container::Blocking<std::deque<downloader::BeatmapInfo>> m_SearchQueue;

    [[noreturn]] static void DownloadThread();
    [[noreturn]] static void SearchThread();

    std::thread t_DownloadThread;
    std::thread t_SearchThread;

public:
    inline const static char *DEFAULT_USER_AGENT =
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36";

    config::Field<bool> f_GrantOsuAccount;
    config::Field<downloader::DownloadMirror> f_Mirror;
    config::Field<downloader::DownloadType> f_DownloadType;

    config::Field<downloader::ProxyServerType> f_ProxySeverType;
    config::Field<std::string> f_ProxySever;
    config::Field<std::string> f_ProxySeverPassword;

    config::Field<bool> f_EnableCustomUserAgent;
    config::Field<std::string> f_CustomUserAgent;

    static Downloader &GetInstance() {
        static Downloader instance;
        return instance;
    }

    void drawMain() override;
    ui::main::FeatureInfo &getInfo() override;

    static void cancelDownload(int sid);
    static void removeCancelDownload(int sid);

    void postSearch(downloader::BeatmapInfo info);
    void postDownload(const osu::Beatmap &bm);
};
}
