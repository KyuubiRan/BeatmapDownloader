﻿#include "pch.h"
#include "DownloadQueue.h"

#include <set>

#include "Downloader.h"
#include "config/I18nManager.h"
#include "utils/gui_utils.h"

namespace features {

std::set<int> removed{};

void DownloadQueue::cancel(const int sid) const {
    if (const auto iter = findTaskBySid(sid); iter == m_TaskQueueSet.end()) {
        LOGW("No download task: %d", sid);
        return;
    }
    removed.insert(sid);
    Downloader::CancelDownload(sid);
}

bool DownloadQueue::addTask(const osu::Beatmap &bm) {
    std::unique_lock _g(m_Mutex);

    if (const auto iter = findTaskBySid(bm.sid); iter != m_TaskQueueSet.end()) {
        LOGW("Already has download task: %d %s-%s (%s)", bm.sid, bm.artist.c_str(), bm.title.c_str(), bm.author.c_str());
        return false;
    }
    m_TaskQueueSet.insert(DownloadTask{
        .bm = bm,
        .insertTime = GetTickCount64(),
    });

    return true;
}

DownloadTask *DownloadQueue::getTask(const osu::Beatmap &bm) {
    return getTask(bm.sid);
}

DownloadTask *DownloadQueue::getTask(int sid) {
    std::unique_lock _g(m_Mutex);
    if (const auto iter = findTaskBySid(sid); iter != m_TaskQueueSet.end()) {
        auto *ret = &const_cast<DownloadTask &>(*iter);
        ret->started = true;
        return ret;
    }

    return nullptr;
}

void DownloadQueue::notifyFinished(const int sid) {
    std::unique_lock _g(m_Mutex);
    if (const auto iter = findTaskBySid(sid); iter != m_TaskQueueSet.end()) {
        m_TaskQueueSet.erase(iter);
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

    {
        std::shared_lock _g(m_Mutex);

        if (m_TaskQueueSet.empty()) {
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(lang.getTextCStr("Empty")).x) * 0.5f);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize(lang.getTextCStr("Empty")).y) * 0.5f);
            ImGui::Text(lang.getTextCStr("Empty"));
            return;
        }

        for (auto &item : m_TaskQueueSet) {
            drawTaskItem(item);
        }
    }
    
    {
        std::unique_lock _g(m_Mutex);
        for (auto &sid : removed) {
            m_TaskQueueSet.erase(findTaskBySid(sid));
        }
        removed.clear();
    }
}

FeatureInfo &DownloadQueue::getInfo() {
    static auto info = FeatureInfo{
        "DownloadQueue",
        ""
    };
    return info;
}

}
