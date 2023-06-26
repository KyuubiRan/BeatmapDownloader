#pragma once
//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     ChumiMapset data = nlohmann::json::parse(jsonString);

#pragma once

#include <optional>
#include "json.hpp"

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
template <typename T>
struct adl_serializer<std::shared_ptr<T>> {
    static void to_json(json &j, const std::shared_ptr<T> &opt) {
        if (!opt)
            j = nullptr;
        else
            j = *opt;
    }

    static std::shared_ptr<T> from_json(const json &j) {
        if (j.is_null())
            return std::make_shared<T>();
        else
            return std::make_shared<T>(j.get<T>());
    }
};
template <typename T>
struct adl_serializer<std::optional<T>> {
    static void to_json(json &j, const std::optional<T> &opt) {
        if (!opt)
            j = nullptr;
        else
            j = *opt;
    }

    static std::optional<T> from_json(const json &j) {
        if (j.is_null())
            return std::make_optional<T>();
        else
            return std::make_optional<T>(j.get<T>());
    }
};
} // namespace nlohmann
#endif

namespace api {
namespace chumi {
using nlohmann::json;

#ifndef NLOHMANN_UNTYPED_api_chumi_HELPER
#define NLOHMANN_UNTYPED_api_chumi_HELPER
inline json get_untyped(const json &j, const char *property) {
    if (j.find(property) != j.end()) {
        return j.at(property).get<json>();
    }
    return json();
}

inline json get_untyped(const json &j, std::string property) { return get_untyped(j, property.data()); }
#endif

#ifndef NLOHMANN_OPTIONAL_api_chumi_HELPER
#define NLOHMANN_OPTIONAL_api_chumi_HELPER
template <typename T>
inline std::shared_ptr<T> get_heap_optional(const json &j, const char *property) {
    auto it = j.find(property);
    if (it != j.end() && !it->is_null()) {
        return j.at(property).get<std::shared_ptr<T>>();
    }
    return std::shared_ptr<T>();
}

template <typename T>
inline std::shared_ptr<T> get_heap_optional(const json &j, std::string property) {
    return get_heap_optional<T>(j, property.data());
}
template <typename T>
inline std::optional<T> get_stack_optional(const json &j, const char *property) {
    auto it = j.find(property);
    if (it != j.end() && !it->is_null()) {
        return j.at(property).get<std::optional<T>>();
    }
    return std::optional<T>();
}

template <typename T>
inline std::optional<T> get_stack_optional(const json &j, std::string property) {
    return get_stack_optional<T>(j, property.data());
}
#endif

struct ChildrenBeatmap {
    std::optional<int64_t> beatmap_id;
    std::optional<int64_t> parent_set_id;
    std::optional<std::string> diff_name;
    std::optional<std::string> file_md5;
    std::optional<int64_t> mode;
    std::optional<double> bpm;
    std::optional<int64_t> ar;
    std::optional<int64_t> od;
    std::optional<int64_t> cs;
    std::optional<int64_t> hp;
    std::optional<int64_t> total_length;
    std::optional<int64_t> hit_length;
    std::optional<int64_t> playcount;
    std::optional<int64_t> passcount;
    std::optional<int64_t> max_combo;
    std::optional<double> difficulty_rating;
    std::optional<std::string> osu_file;
    std::optional<std::string> download_path;
};

struct ChumiMapset {
    std::optional<int64_t> set_id;
    std::optional<std::vector<ChildrenBeatmap>> children_beatmaps;
    std::optional<int64_t> ranked_status;
    std::optional<std::string> approved_date;
    std::optional<std::string> last_update;
    std::optional<std::string> last_checked;
    std::optional<std::string> artist;
    std::optional<std::string> title;
    std::optional<std::string> creator;
    std::optional<std::string> source;
    std::optional<std::string> tags;
    std::optional<bool> has_video;
    std::optional<int64_t> genre;
    std::optional<int64_t> language;
    std::optional<int64_t> favourites;
    std::optional<bool> disabled;
    std::optional<std::string> error_message;
    std::optional<std::string> error_code;

    osu::Beatmap to_beatmap() const;
};

osu::Beatmap ChumiMapset::to_beatmap() const {
    std::vector<int> ids;
    ids.reserve(children_beatmaps.value().size());
    for (auto it : children_beatmaps.value()) {
        ids.push_back(static_cast<int>(it.beatmap_id.value()));
    }
    return {title.value(), artist.value(), creator.value(), ids, static_cast<int>(set_id.value()), has_video.value() != 0};
}

} // namespace chumi
} // namespace api

namespace api {
namespace chumi {
void from_json(const json &j, ChildrenBeatmap &x);
void to_json(json &j, const ChildrenBeatmap &x);

void from_json(const json &j, ChumiMapset &x);
void to_json(json &j, const ChumiMapset &x);

inline void from_json(const json &j, ChildrenBeatmap &x) {
    x.beatmap_id = get_stack_optional<int64_t>(j, "BeatmapId");
    x.parent_set_id = get_stack_optional<int64_t>(j, "ParentSetId");
    x.diff_name = get_stack_optional<std::string>(j, "DiffName");
    x.file_md5 = get_stack_optional<std::string>(j, "FileMD5");
    x.mode = get_stack_optional<int64_t>(j, "Mode");
    x.bpm = get_stack_optional<double>(j, "BPM");
    x.ar = get_stack_optional<int64_t>(j, "AR");
    x.od = get_stack_optional<int64_t>(j, "OD");
    x.cs = get_stack_optional<int64_t>(j, "CS");
    x.hp = get_stack_optional<int64_t>(j, "HP");
    x.total_length = get_stack_optional<int64_t>(j, "TotalLength");
    x.hit_length = get_stack_optional<int64_t>(j, "HitLength");
    x.playcount = get_stack_optional<int64_t>(j, "Playcount");
    x.passcount = get_stack_optional<int64_t>(j, "Passcount");
    x.max_combo = get_stack_optional<int64_t>(j, "MaxCombo");
    x.difficulty_rating = get_stack_optional<double>(j, "DifficultyRating");
    x.osu_file = get_stack_optional<std::string>(j, "OsuFile");
    x.download_path = get_stack_optional<std::string>(j, "DownloadPath");
}

inline void to_json(json &j, const ChildrenBeatmap &x) {
    j = json::object();
    j["BeatmapId"] = x.beatmap_id;
    j["ParentSetId"] = x.parent_set_id;
    j["DiffName"] = x.diff_name;
    j["FileMD5"] = x.file_md5;
    j["Mode"] = x.mode;
    j["BPM"] = x.bpm;
    j["AR"] = x.ar;
    j["OD"] = x.od;
    j["CS"] = x.cs;
    j["HP"] = x.hp;
    j["TotalLength"] = x.total_length;
    j["HitLength"] = x.hit_length;
    j["Playcount"] = x.playcount;
    j["Passcount"] = x.passcount;
    j["MaxCombo"] = x.max_combo;
    j["DifficultyRating"] = x.difficulty_rating;
    j["OsuFile"] = x.osu_file;
    j["DownloadPath"] = x.download_path;
}

inline void from_json(const json &j, ChumiMapset &x) {
    x.set_id = get_stack_optional<int64_t>(j, "SetId");
    x.children_beatmaps = get_stack_optional<std::vector<ChildrenBeatmap>>(j, "ChildrenBeatmaps");
    x.ranked_status = get_stack_optional<int64_t>(j, "RankedStatus");
    x.approved_date = get_stack_optional<std::string>(j, "ApprovedDate");
    x.last_update = get_stack_optional<std::string>(j, "LastUpdate");
    x.last_checked = get_stack_optional<std::string>(j, "LastChecked");
    x.artist = get_stack_optional<std::string>(j, "Artist");
    x.title = get_stack_optional<std::string>(j, "Title");
    x.creator = get_stack_optional<std::string>(j, "Creator");
    x.source = get_stack_optional<std::string>(j, "Source");
    x.tags = get_stack_optional<std::string>(j, "Tags");
    x.has_video = get_stack_optional<bool>(j, "HasVideo");
    x.genre = get_stack_optional<int64_t>(j, "Genre");
    x.language = get_stack_optional<int64_t>(j, "Language");
    x.favourites = get_stack_optional<int64_t>(j, "Favourites");
    x.disabled = get_stack_optional<bool>(j, "Disabled");
    x.error_message = get_stack_optional<std::string>(j, "error_message");
    x.error_code = get_stack_optional<std::string>(j, "error_code");
}

inline void to_json(json &j, const ChumiMapset &x) {
    j = json::object();
    j["SetId"] = x.set_id;
    j["ChildrenBeatmaps"] = x.children_beatmaps;
    j["RankedStatus"] = x.ranked_status;
    j["ApprovedDate"] = x.approved_date;
    j["LastUpdate"] = x.last_update;
    j["LastChecked"] = x.last_checked;
    j["Artist"] = x.artist;
    j["Title"] = x.title;
    j["Creator"] = x.creator;
    j["Source"] = x.source;
    j["Tags"] = x.tags;
    j["HasVideo"] = x.has_video;
    j["Genre"] = x.genre;
    j["Language"] = x.language;
    j["Favourites"] = x.favourites;
    j["Disabled"] = x.disabled;
    j["error_message"] = x.error_message;
    j["error_code"] = x.error_code;
}
} // namespace chumi
} // namespace api
