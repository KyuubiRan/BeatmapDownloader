#pragma once
#include <string>
#include <vector>


namespace osu {

struct Beatmap {
    std::string title;
    std::string artist;
    std::string author;
    std::vector<int32_t> bid;
    int32_t sid;
    bool hasVideo;
};

}
