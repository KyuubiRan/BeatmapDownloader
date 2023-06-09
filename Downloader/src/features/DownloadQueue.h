#pragma once
#include <shared_mutex>
#include <unordered_map>

#include "osu/Beatmap.h"
#include "ui/Feature.h"

namespace features {
struct DownloadTask {
    osu::Beatmap bm;
    double currentDownloaded{};
    double totalSize{};

    [[nodiscard]] double getProgress() const {
        return totalSize == 0 ? 0 : currentDownloaded / totalSize;
    }
};

class DownloadQueue : public ui::main::Feature {
    DownloadQueue();

    std::unordered_map<int, DownloadTask> m_InQueueMap;
    
    inline static std::shared_mutex m_Mutex{};

    void drawTaskItem(const DownloadTask &item);

    void cancel(int sid);

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

    DownloadTask *addTask(const osu::Beatmap &bm);
    DownloadTask *getTask(const osu::Beatmap &bm);
    DownloadTask *getTask(int sid);

    void notifyFinished(int sid);

    void drawMain() override;
    ui::main::FeatureInfo &getInfo() override;
};
}
