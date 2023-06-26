#pragma once
#include "Provider.h"

namespace api {
class Chimu : public Provider {
    int32_t _convertToSetId(const features::downloader::BeatmapInfo &info) const;

public:
    Chimu();

    // Inherited via Provider
    std::optional<osu::Beatmap> SearchBeatmap(const features::downloader::BeatmapInfo &) const override;
    bool DownloadBeatmap(const osu::Beatmap &) const override;
};
}; // namespace api
