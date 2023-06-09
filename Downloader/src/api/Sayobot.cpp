﻿#include "pch.h"
#include "Sayobot.h"

#include "network/HttpRequest.h"
#include "utils/Utils.h"

namespace api::sayobot {
void BidData::to_json(nlohmann::json &j) const {
    j.at("bid") = bid;
    j.at("mode") = mode;
}

void BidData::from_json(const nlohmann::json &j) {
    j.at("bid").get_to(bid);
    j.at("mode").get_to(mode);
}

void SayoBeatmapDataV2::to_json(nlohmann::json &j) const {
    j.at("approved") = approved;
    j.at("artist") = artist;
    j.at("artistU") = artistU;
    j.at("creator") = creator;
    j.at("creator_id") = creatorId;
    j.at("title") = title;
    j.at("titleU") = titleU;
    j.at("sid") = sid;
    j.at("video") = video;
    j.at("bid_data") = bidData;
}

void SayoBeatmapDataV2::from_json(const nlohmann::json &j) {
    j.at("approved").get_to(approved);
    j.at("artist").get_to(artist);
    j.at("artistU").get_to(artistU);
    j.at("creator").get_to(creator);
    j.at("creator_id").get_to(creatorId);
    j.at("title").get_to(title);
    j.at("titleU").get_to(titleU);
    j.at("sid").get_to(sid);
    j.at("video").get_to(video);
    j.at("bid_data").get_to(bidData);
}

osu::Beatmap SayoBeatmapDataV2::to_beatmap() const {
    std::vector<int32_t> bids;
    bids.reserve(bidData.size());
    for (auto &bd : bidData) {
        bids.push_back(bd.bid);
    }
    return {title, artist, creator, bids, sid, video != 0};
}

};

void nlohmann::adl_serializer<api::sayobot::BidData>::to_json(nlohmann::json &j, const api::sayobot::BidData &data) { data.to_json(j); }

void nlohmann::adl_serializer<api::sayobot::BidData>::from_json(const nlohmann::json &j, api::sayobot::BidData &data) { data.from_json(j); }

api::Sayobot::Sayobot() :
    Provider("Sayobot", "https://www.showdoc.com.cn/SoulDee?page_id=3969517351482508", features::downloader::DownloadMirror::Sayobot) {}

std::optional<osu::Beatmap> api::Sayobot::searchBeatmap(const features::downloader::BeatmapInfo &info) const {
    const auto s = std::format("https://api.sayobot.cn/v2/beatmapinfo?K={0}&T={1}", info.id, (uint8_t)info.type);
    int resCode = -1;
    std::string response;

    LOGD("Begin sayobot search type=%s, id=%d", info.type == features::downloader::BeatmapType::Sid ? "Auto" : "Bid", info.id);
    
    if (const CURLcode code = net::curl_get(s.c_str(), response, &resCode); code == CURLE_OK && resCode == 200) {
        // LOGD("Sayo search response: %s", response.c_str());
        const auto j = nlohmann::json::parse(response);
        api::sayobot::SayoResult<api::sayobot::SayoBeatmapDataV2> data;
        data.from_json(j);

        if (data.status == 0) {
            if (data.data.has_value()) {
                return std::optional(data.data.value().to_beatmap());
            } else {
                return {};
            }
        }

        LOGW("Sayo search failed: code=%d", data.status);
    } else {
        LOGW("Sayo search failed: CURL_CODE=%d, RESPONSE_CODE=%d", code, resCode);
    }

    return {};
}

bool api::Sayobot::downloadBeatmap(const osu::Beatmap &bm) const {
    auto &dl = features::Downloader::GetInstance();

    const auto s = std::format("https://txy1.sayobot.cn/beatmaps/download/{0}/{1}?server=auto",
                               dl.f_DownloadType.getValue() == features::downloader::DownloadType::NoVideo ? "novideo" : "full", bm.sid);

    auto path = utils::GetCurrentDirPath() / L"downloads" / (std::to_string(bm.sid) + ".osz");
    auto *tsk = features::DownloadQueue::GetInstance().getTask(bm);

    int resCode = -1;
    if (const auto code = net::curl_download(s.c_str(), path, tsk, &resCode); code == CURLE_OK && resCode == 200) {
        LOGI("Success download beatmapsets: %d", bm.sid);
        return true;
    } else {
        LOGW("Sayo download failed: CURL_CODE=%d, RESPONSE_CODE=%d", code, resCode);
    }

    return false;
}
