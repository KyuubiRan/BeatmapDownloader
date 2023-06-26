#pragma once
#include "features/Downloader.h"
#include "osu/Beatmap.h"
#include "Provider.h"

namespace api {
class Bancho : public Provider {
public:
    Bancho();

    // Inherited via Provider
    std::optional<osu::Beatmap> searchBeatmap(const features::downloader::BeatmapInfo &) const override;
    bool downloadBeatmap(const osu::Beatmap &) const override;
};
}
