#pragma once
#include <shared_mutex>
#include <unordered_map>

#include "osu/Beatmap.h"
#include "ui/Feature.h"

namespace features {
struct DownloadTask {
    osu::Beatmap bm;
    double dlSize{};
    double totalSize{};
    bool started = false;

    [[nodiscard]] double getProgress() const {
        return totalSize == 0 ? 0 : dlSize / totalSize;
    }

    [[nodiscard]] bool prepared() const {
        return dlSize != 0 && totalSize != 0;
    }
};

class DownloadQueue : public ui::main::Feature {
    DownloadQueue();

    std::unordered_map<int, DownloadTask> m_InQueueMap;

    inline static std::shared_mutex m_Mutex{};

    void drawTaskItem(const DownloadTask &item) const;

    void cancel(int sid) const;

public:
    static DownloadQueue &GetInstance() {
        static DownloadQueue instance;
        return instance;
    }

    bool hasDownloadMapSet(const int id) const {
        std::shared_lock _g(m_Mutex);
        return m_InQueueMap.contains(id);
    }

    bool hasDownloadMap(const osu::Beatmap &bm) const {
        std::shared_lock _g(m_Mutex);
        return m_InQueueMap.contains(bm.sid);
    }

    bool addTask(const osu::Beatmap &bm);
    DownloadTask *getTask(const osu::Beatmap &bm);
    DownloadTask *getTask(int sid);

    void notifyFinished(int sid);

    void drawMain() override;
    ui::main::FeatureInfo &getInfo() override;
};
}
