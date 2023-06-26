#pragma once
//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     ChumiMap data = nlohmann::json::parse(jsonString);

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

struct ChumiMap {
    std::optional<int64_t> beatmap_id;
    std::optional<int64_t> parent_set_id;
    std::optional<std::string> diff_name;
    std::optional<std::string> file_md5;
    std::optional<int64_t> mode;
    std::optional<int64_t> bpm;
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
    std::optional<std::string> error_message;
    std::optional<std::string> error_code;
};
} // namespace chumi
} // namespace api

namespace api {
namespace chumi {
void from_json(const json &j, ChumiMap &x);
void to_json(json &j, const ChumiMap &x);

inline void from_json(const json &j, ChumiMap &x) {
    x.beatmap_id = get_stack_optional<int64_t>(j, "BeatmapId");
    x.parent_set_id = get_stack_optional<int64_t>(j, "ParentSetId");
    x.diff_name = get_stack_optional<std::string>(j, "DiffName");
    x.file_md5 = get_stack_optional<std::string>(j, "FileMD5");
    x.mode = get_stack_optional<int64_t>(j, "Mode");
    x.bpm = get_stack_optional<int64_t>(j, "BPM");
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
    x.error_message = get_stack_optional<std::string>(j, "error_message");
    x.error_code = get_stack_optional<std::string>(j, "error_code");
}

inline void to_json(json &j, const ChumiMap &x) {
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
    j["error_message"] = x.error_message;
    j["error_code"] = x.error_code;
}
} // namespace chumi
} // namespace api
