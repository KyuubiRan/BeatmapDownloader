#pragma once
#include "config/Field.h"
#include "ui/Feature.h"

namespace features {
enum class DownloadMirror : uint8_t {
    OsuOfficial = 0,
    Sayobot,
    // Chimu,
};

enum class BeatmapType {
    Bid,
    Sid
};

struct DownloadInfo {
    BeatmapType type;
    int32_t id;
};

class Downloader : public ui::main::Feature {
    Downloader();

public:
    config::Field<bool> f_GrantOsuAccount;
    config::Field<DownloadMirror> f_Mirror;

    static Downloader& GetInstance() {
        static Downloader instance;
        return instance;
    }

    void drawMain() override;
    ui::main::FeatureInfo& getInfo() override;
};
}
