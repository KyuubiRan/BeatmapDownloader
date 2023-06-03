#pragma once
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

    std::map<int, DownloadTask> m_InQueueMap;

    void drawTaskItem(const DownloadTask &item);

    void cancel(int sid);

public:
    static DownloadQueue& GetInstance() {
        static DownloadQueue instance;
        return instance;
    }

    bool hasDownloadMapSet(const int id) const {
        return m_InQueueMap.contains(id);
    }

    bool hasDownloadMap(const osu::Beatmap &bm) const {
        return m_InQueueMap.contains(bm.sid);
    }

    DownloadTask* addTask(const osu::Beatmap &bm);

    void notifyFinished(int sid);

    void drawMain() override;
    ui::main::FeatureInfo& getInfo() override;
};
}
