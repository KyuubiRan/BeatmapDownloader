#include "pch.h"
#include "DownloadQueue.h"

#include <ranges>
#include <set>

#include "Downloader.h"
#include "config/I18nManager.h"
#include "utils/gui_utils.h"

namespace features {

std::set<int> removed{};

void DownloadQueue::cancel(const int sid) const {
    if (!m_InQueueMap.contains(sid)) {
        LOGW("No download task: %d", sid);
        return;
    }
    removed.insert(sid);
    Downloader::CancelDownload(sid);
}

bool DownloadQueue::addTask(const osu::Beatmap &bm) {
    std::unique_lock _g(m_Mutex);

    if (m_InQueueMap.contains(bm.sid)) {
        LOGW("Already has download task: %d %s-%s (%s)", bm.sid, bm.artist.c_str(), bm.title.c_str(), bm.author.c_str());
        return false;
    }
    m_InQueueMap.insert({
        bm.sid, DownloadTask{
            .bm = bm
        }
    });

    return true;
}

DownloadTask *DownloadQueue::getTask(const osu::Beatmap &bm) {
    std::shared_lock _g(m_Mutex);
    if (m_InQueueMap.contains(bm.sid)) {
        auto &ret = m_InQueueMap[bm.sid];
        ret.started = true;
        return &ret;
    }

    return nullptr;
}

DownloadTask *DownloadQueue::getTask(int sid) {
    std::shared_lock _g(m_Mutex);
    if (m_InQueueMap.contains(sid)) {
        auto &ret = m_InQueueMap[sid];
        ret.started = true;
        return &ret;
    }

    return nullptr;
}

void DownloadQueue::notifyFinished(int sid) {
    std::unique_lock _g(m_Mutex);
    if (m_InQueueMap.contains(sid)) {
        m_InQueueMap.erase(sid);
    }
    Downloader::RemoveCancelDownload(sid);
}

DownloadQueue::DownloadQueue() {
    LOGI("Inited Download Queue");
}

void DownloadQueue::drawTaskItem(const DownloadTask &item) const {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::BeginGroupPanel(std::to_string(item.bm.sid).c_str());

    ImGui::Text("%s - %s [%s]", item.bm.artist.c_str(), item.bm.title.c_str(), item.bm.author.c_str());

    if (item.started) {
        const auto progress = static_cast<float>(item.getProgress());
        const std::string t = !item.prepared()
            ? lang.getTextCStr("Connecting")
            : std::format("{:.2f}MB / {:.2f}MB ({:.2f}%%)", item.dlSize / 1024 / 1024,
                          item.totalSize / 1024 / 1024, progress * 100);
        ImGui::Text(t.c_str());
        ImGui::ProgressBar(progress, ImVec2(-1, 1));
    } else {
        if (ImGui::Button(lang.getTextCStr("CancelDownload"))) {
            cancel(item.bm.sid);
        }
    }

    ImGui::EndGroupPanel();
}


void DownloadQueue::drawMain() {
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

    for (auto &sid : removed) {
        m_InQueueMap.erase(sid);
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

FeatureInfo &DownloadQueue::getInfo() {
    static auto info = FeatureInfo{
        "DownloadQueue",
        ""
    };
    return info;
}

}
