#pragma once

#include <set>
#include <shared_mutex>

namespace osu {
class BeatmapManager {
    BeatmapManager();

    std::set<int32_t> m_BeatmapSetIds = {};
    std::set<int32_t> m_BeatmapIds = {};
    std::shared_mutex m_rwlock;

    void initDatabase();

public:
    static BeatmapManager& GetInstance() {
        static BeatmapManager instance;
        return instance;
    }

    bool hasBeatmap(int32_t beatmapId) {
        std::shared_lock _g(m_rwlock);
        return m_BeatmapIds.contains(beatmapId);
    }

    bool hasBeatmapSet(int32_t beatmapSetId) {
        std::shared_lock _g(m_rwlock);
        return m_BeatmapSetIds.contains(beatmapSetId);
    }
};
}
