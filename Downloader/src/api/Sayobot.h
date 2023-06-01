#pragma once

#include <string>
#include <json.hpp>
#include "misc/ISerializable.h"
#include "features/Downloader.h"

namespace api::sayobot {

struct BidData : ISerializable {
    int bid;
    int mode;

    void to_json(nlohmann::json &j) const override;
    void from_json(const nlohmann::json &j) override;
};

struct SayoBeatmapDataV2 : ISerializable {
    int approved;
    std::string artist;
    std::string artistU;
    std::string creator;
    int creatorId;
    std::string title;
    std::string titleU;
    int sid;
    int video;
    std::vector<BidData> bidData;

    void to_json(nlohmann::json &j) const override;
    void from_json(const nlohmann::json &j) override;
};

template <typename T, std::enable_if_t<std::is_base_of_v<ISerializable, T>>* = nullptr>
struct SayoResult : ISerializable {
    int status;
    std::optional<T> data;

    void to_json(nlohmann::json &j) const override;
    void from_json(const nlohmann::json &j) override;
};

template <typename T, std::enable_if_t<std::is_base_of_v<ISerializable, T>>*E0>
void SayoResult<T, E0>::to_json(nlohmann::json &j) const {
    j["status"] = status;
    if (data) {
        ((ISerializable)*data).to_json(j);
    }
}

template <typename T, std::enable_if_t<std::is_base_of_v<ISerializable, T>>*E0>
void SayoResult<T, E0>::from_json(const nlohmann::json &j) {
    status = j["status"];
    if (status == 0) {
        ((ISerializable)*data).from_json(j);
    }
}

std::optional<SayoResult<SayoBeatmapDataV2>> SearchBeatmapV2(const features::downloader::BeatmapInfo &info);

std::vector<BYTE> DownloadBeatmap(features::downloader::BeatmapInfo &info, uint64_t *cur = nullptr, uint64_t *ttl = nullptr);
}

template <>
struct nlohmann::adl_serializer<api::sayobot::BidData> {
    static void to_json(nlohmann::json &j, const api::sayobot::BidData &data);

    static void from_json(const nlohmann::json &j, api::sayobot::BidData &data);
};
