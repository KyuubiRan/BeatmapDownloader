#pragma once
#include "features/Downloader.h"
#include "osu/Beatmap.h"

namespace api::bancho {
std::optional<osu::Beatmap> SearchBeatmap(features::downloader::BeatmapInfo &info);

bool DownloadBeatmap(osu::Beatmap &bm);
}
