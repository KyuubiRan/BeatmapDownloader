#include "pch.h"
#include "OsuConfigManager.h"

#include <fstream>

#include "utils/Utils.h"

void osu::OsuConfigManager::parseLine(const std::string &line) {
    const size_t pos = line.find('=');

    if (pos == std::string::npos) // Not k v pair
        return;

    std::string key = line.substr(0, pos);
    utils::trim(key);
    std::string value = line.substr(pos + 1);
    utils::trim(value);

    // Int: VolumeEffect = 85
    // String: keyOsuLeft = Z
    // Float: ScoreMeterScale = 1.5

    s_Config[key] = value;
    // LOGD("Parsed config: %s=%s", key.c_str(), value.c_str());
}

void osu::OsuConfigManager::Init(const std::filesystem::path &path) {
    if (!exists(path)) {
        LOGE("Config file not exists!");
        return;
    }
    s_Path = path;
    std::ifstream ifs(path, std::ios::binary);
    if (ifs.bad()) {
        LOGE("Cannot open config file!");
        return;
    }

    LOGD("Osu! config path: %s", path.string().c_str());

    std::string line;
    while (std::getline(ifs, line)) {
        parseLine(line);
    }
    LOGD("Finished parse config file!");
}

int osu::OsuConfigManager::GetInt(const std::string &key, const int def) {
    if (!s_Config.contains(key)) {
        LOGW("No such key '%s' found in config", key.c_str());
        return def;
    }

    try {
        return std::stoi(s_Config[key]);
    } catch (...) {
        LOGW("Not int value '%s' in config", key.c_str());
    }

    return def;
}

std::string osu::OsuConfigManager::GetString(const std::string &key) {
    if (!s_Config.contains(key)) {
        LOGW("No such key '%s' found in config", key.c_str());
        return {};
    }

    return s_Config[key];
}

float osu::OsuConfigManager::GetFloat(const std::string &key, const float def) {
    if (!s_Config.contains(key)) {
        LOGW("No such key '%s' found in config", key.c_str());
        return def;
    }

    try {
        return std::stof(s_Config[key]);
    } catch (...) {
        LOGW("Not float value '%s' in config", key.c_str());
    }

    return def;
}
