#pragma once
#include <set>
#include <shared_mutex>

#include "osu/Beatmap.h"
#include "Feature.h"

namespace features {
struct DownloadTask {
    osu::Beatmap bm;
    double dlSize{};
    double totalSize{};
    bool started = false;
    uint64_t insertTime = UINT64_MAX;

    [[nodiscard]] double getProgress() const {
        return totalSize == 0 ? 0 : dlSize / totalSize;
    }

    [[nodiscard]] bool prepared() const {
        return dlSize != 0 && totalSize != 0;
    }

    bool operator==(const int sid) const {
        return bm.sid == sid;
    }

    bool operator<(const DownloadTask &dl) const {
        return insertTime < dl.insertTime;
    }
};

class DownloadQueue : public Feature {
    DownloadQueue();

    std::set<DownloadTask> m_TaskQueueSet;

    inline static std::shared_mutex m_Mutex{};

    void drawTaskItem(const DownloadTask &item) const;

    void cancel(int sid) const;

public:
    using TaskIter = std::set<DownloadTask>::iterator;
    using TaskIterConst = std::set<DownloadTask>::const_iterator;

    static DownloadQueue &GetInstance() {
        static DownloadQueue instance;
        return instance;
    }

    bool hasDownloadMapSet(const int id) const {
        return std::ranges::any_of(m_TaskQueueSet.begin(), m_TaskQueueSet.end(),
                                   [id](const DownloadTask &task) { return task.bm.sid == id; });
    }

    bool hasDownloadMap(const osu::Beatmap &bm) const {
        return hasDownloadMapSet(bm.sid);
    }

    TaskIter findTaskBySid(const int id) const {
        return std::ranges::find_if(m_TaskQueueSet, [id](const DownloadTask &task) { return task.bm.sid == id; });
    }

    bool addTask(const osu::Beatmap &bm);
    DownloadTask *getTask(const osu::Beatmap &bm);
    DownloadTask *getTask(int sid);

    void notifyFinished(int sid);

    void drawMain() override;
    FeatureInfo &getInfo() override;
};
}
