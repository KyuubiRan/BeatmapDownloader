#pragma once

#include <optional>
#include <regex>

#include "features/Downloader.h"

namespace osu {

class LinkParser {
public:
    static std::optional<features::downloader::BeatmapInfo> ParseLink(
        const std::string &link,
        const features::downloader::BeatmapType defIdType = features::downloader::BeatmapType::Sid) {

        const static std::string sidRegex = R"(/?(beatmapsets|s)/?(\d+))";
        std::smatch match;
        if (const std::regex re1(sidRegex); std::regex_search(link, match, re1) && match.size() > 2) {
            const int id = std::stoi(match[2].str());
            return features::downloader::BeatmapInfo{features::downloader::BeatmapType::Sid, id};
        }

        const static std::string bidRegex = R"(/?(beatmaps|b)/?(\d+))";
        if (const std::regex re2(bidRegex); std::regex_search(link, match, re2) && match.size() > 2) {
            const int id = std::stoi(match[2].str());
            return features::downloader::BeatmapInfo{features::downloader::BeatmapType::Bid, id};
        }

        const static std::string idRegex = R"(^(\d+)$)";
        if (const std::regex re3(idRegex); std::regex_search(link, match, re3) && match.size() > 1) {
            const int id = std::stoi(match[1].str());
            return features::downloader::BeatmapInfo{defIdType, id};
        }

        return {};
    }
};

}
