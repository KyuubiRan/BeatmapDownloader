#pragma once
#include <any>
#include <filesystem>

namespace osu {

class OsuConfigManager {
    inline static std::filesystem::path s_Path{};

    inline static std::map<std::string, std::string> s_Config{};

    static void parseLine(const std::string &line);

public:
    static void Init(const std::filesystem::path &path);

    static int GetInt(const std::string &key, int def = 0);
    static std::string GetString(const std::string &key);
    static float GetFloat(const std::string &key, float def = 0.0f);
};

}
