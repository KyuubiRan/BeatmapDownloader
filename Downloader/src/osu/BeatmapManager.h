#pragma once

#include <set>
#include <shared_mutex>

#include "Beatmap.h"
#include "features/Downloader.h"

namespace osu {
class BeatmapManager {
    BeatmapManager();

    std::set<int32_t> m_BeatmapSetIds = {};
    std::set<int32_t> m_BeatmapIds = {};
    std::shared_mutex m_rwlock;

    void initDatabase();

public:
    static BeatmapManager &GetInstance() {
        static BeatmapManager instance;
        return instance;
    }

    bool hasBeatmap(int32_t beatmapId) {
        std::shared_lock _g(m_rwlock);
        return beatmapId > 0 && m_BeatmapIds.contains(beatmapId);
    }

    bool hasBeatmapSet(int32_t beatmapSetId) {
        std::shared_lock _g(m_rwlock);
        return beatmapSetId > 0 && m_BeatmapSetIds.contains(beatmapSetId);
    }

    bool hasBeatmap(const Beatmap &bm) {
        std::shared_lock _g(m_rwlock);
        if (bm.sid > 0 && m_BeatmapSetIds.contains(bm.sid))
            return true;
        if (!bm.bid.empty() && std::ranges::any_of(bm.bid, [&](const int32_t i) {
            return m_BeatmapIds.contains(i);
        })) {
            return true;
        }
        return false;
    }

    void openBeatmapPage(Beatmap &bm);

    void addBeatmap(const Beatmap &bm) {
        std::unique_lock _g(m_rwlock);
        if (bm.sid > 0)
            m_BeatmapSetIds.insert(bm.sid);

        for (auto i : bm.bid) {
            if (i > 0)
                m_BeatmapIds.insert(i);
        }
    }
};
}
