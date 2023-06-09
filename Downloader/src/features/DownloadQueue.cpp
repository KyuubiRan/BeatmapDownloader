#include "pch.h"
#include "DownloadQueue.h"

#include <ranges>

#include "Downloader.h"
#include "config/I18nManager.h"
#include "utils/gui_utils.h"

void features::DownloadQueue::cancel(const int sid) {
    if (!m_InQueueMap.contains(sid)) {
        LOGW("No download task: %d", sid);
        return;
    }
    m_InQueueMap.erase(sid);
    Downloader::cancelDownload(sid);
}

features::DownloadTask *features::DownloadQueue::addTask(const osu::Beatmap &bm) {
    std::unique_lock _g(m_Mutex);

    if (m_InQueueMap.contains(bm.sid)) {
        LOGW("Already has download task: %d %s-%s (%s)", bm.sid, bm.artist.c_str(), bm.title.c_str(), bm.author.c_str());
        return &m_InQueueMap[bm.sid];
    }
    m_InQueueMap.insert({
        bm.sid, DownloadTask{
            .bm = bm
        }
    });

    return &m_InQueueMap[bm.sid];
}

features::DownloadTask *features::DownloadQueue::getTask(const osu::Beatmap &bm) {
    std::shared_lock _g(m_Mutex);
    if (m_InQueueMap.contains(bm.sid)) {
        return &m_InQueueMap[bm.sid];
    }

    return nullptr;
}

features::DownloadTask * features::DownloadQueue::getTask(int sid) {
    std::shared_lock _g(m_Mutex);
    if (m_InQueueMap.contains(sid)) {
        return &m_InQueueMap[sid];
    }

    return nullptr;
}

void features::DownloadQueue::notifyFinished(int sid) {
    std::unique_lock _g(m_Mutex);

    if (m_InQueueMap.contains(sid)) {
        m_InQueueMap.erase(sid);
    }
    Downloader::removeCancelDownload(sid);
}

features::DownloadQueue::DownloadQueue() {
    LOGI("Initied Download Queue");
}

void features::DownloadQueue::drawTaskItem(const DownloadTask &item) {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::BeginGroupPanel(std::to_string(item.bm.sid).c_str());

    ImGui::Text("%s - %s [%s]", item.bm.artist.c_str(), item.bm.title.c_str(), item.bm.author.c_str());

    if (item.currentDownloaded != 0.0 && item.totalSize != 0.0) {
        const auto progress = static_cast<float>(item.getProgress());
        const std::string t = std::format("{:.2f}MB / {:.2f}MB ({:.2f}%%)", item.currentDownloaded / 1024 / 1024,
                                          item.totalSize / 1024 / 1024, progress * 100);
        ImGui::Text(t.c_str());
        ImGui::ProgressBar(progress, ImVec2(-1, 1));
    } else {
        const std::string s = std::format("{}##cancel-{}", lang.getTextCStr("CancelDownload"), item.bm.sid);
        if (ImGui::Button(s.c_str())) {
            cancel(item.bm.sid);
        }
    }

    ImGui::EndGroupPanel();
    ImGui::PopStyleVar();
}


void features::DownloadQueue::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();

    std::shared_lock _g(m_Mutex);

    if (m_InQueueMap.empty()) {
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.getTextCStr("Empty")).x) * 0.5f);
        ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize(lang.getTextCStr("Empty")).y) * 0.5f);
        ImGui::Text(lang.getTextCStr("Empty"));
        return;
    }

    for (auto &item : m_InQueueMap | std::views::values) {
        drawTaskItem(item);
    }

    /*
    static auto example = DownloadTask{
        osu::Beatmap{
            .title = "Test title",
            .artist = "Test artist",
            .author = "Test mapper",
            .bid = {123456, 789546},
            .sid = 655352
        },
        4069000, 8192000
    };
    static auto example2 = DownloadTask{
        osu::Beatmap{
            .title = "Test title",
            .artist = "Test artist",
            .author = "Test mapper",
            .bid = {123456, 789546},
            .sid = 655351
        },
        0, 0
    };

    drawTaskItem(example);
    drawTaskItem(example2);
    */
}

ui::main::FeatureInfo &features::DownloadQueue::getInfo() {
    static auto info = ui::main::FeatureInfo{
        "DownloadQueue",
        ""
    };
    return info;
}
