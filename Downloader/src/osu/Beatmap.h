#pragma once
#include <string>


namespace osu {

struct Beatmap {
    std::string title;
    std::string artist;
    std::string author;
    int32_t bid;
    int32_t sid;
};

}
