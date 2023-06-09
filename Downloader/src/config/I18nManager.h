#pragma once

#include <map>
#include <json.hpp>
#include "Field.h"
#include "misc/ResourcesLoader.hpp"
#include "../resource.h"

namespace i18n {
enum class Language : uint8_t {
    EN_US = 0,
    ZH_CN = 1,
};

class I18nManager {
    // i18nMap["en_us"]["Downloader"] -> "Downloader"
    inline static std::map<std::string, std::map<std::string, std::string>> i18nMap;

    static const char *GetLanguageName(Language lang) {
        switch (lang) {
        case Language::EN_US:
            return "EN_US";
        case Language::ZH_CN:
            return "ZH_CN";
        }

        return "EN_US";
    }

public:
    config::Field<Language> lang;

    static I18nManager &GetInstance() {
        static I18nManager instance;
        return instance;
    }

    std::string_view getText(std::string_view key) {
        if (auto &map = i18nMap[GetLanguageName(lang)]; map.contains(key.data())) {
            return map[key.data()];
        }
        if (i18nMap["EN_US"].contains(key.data())) {
            return i18nMap["EN_US"][key.data()];
        }
        return key;
    }

    static std::string_view GetText(std::string_view key) {
        return GetInstance().getText(key);
    }

    std::string_view getText(const char *key) {
        if (auto &map = i18nMap[GetLanguageName(lang)]; map.contains(key)) {
            return map[key];
        }
        if (i18nMap["EN_US"].contains(key)) {
            return i18nMap["EN_US"][key];
        }
        return key;
    }

    static std::string_view GetText(const char *key) {
        return GetInstance().getText(key);
    }

    const char *getTextCStr(std::string_view key) {
        if (auto &map = i18nMap[GetLanguageName(lang)]; map.contains(key.data())) {
            return map[key.data()].c_str();
        }
        if (i18nMap["EN_US"].contains(key.data())) {
            return i18nMap["EN_US"][key.data()].c_str();
        }
        return key.data();
    }

    static const char *GetTextCStr(std::string_view key) {
        return GetInstance().getTextCStr(key);
    }

    const char *getTextCStr(const char *key) {
        if (auto &map = i18nMap[GetLanguageName(lang)]; map.contains(key)) {
            return map[key].c_str();
        }
        if (i18nMap["EN_US"].contains(key)) {
            return i18nMap["EN_US"][key].c_str();
        }
        return key;
    }

    static const char *GetTextCStr(const char *key) {
        return GetInstance().getTextCStr(key);
    }

private:
    I18nManager() :
        lang(config::Field("Language", Language::EN_US)) {
        BYTE *data = nullptr;

#define LOAD_LANG_FILE(lang) try { \
            if (res::LoadEx(IDR_LANG_##lang, L"LANG", &data)) { \
                std::string s = std::string(reinterpret_cast<char *>(data)); \
                nlohmann::json j = nlohmann::json::parse(s); \
                for (auto &i : j.items()) i18nMap[#lang][i.key()] = i.value(); \
                LOGD("Loaded language file: %s, json size: %zu, map size: %zu", #lang, j.size(), i18nMap[#lang].size()); \
            } else { \
                LOGD("Cannot load language file: %s", #lang); \
            } \
        } catch (...) { \
            LOGE("Cannot initialize language file: %s", #lang); \
        }

        LOAD_LANG_FILE(EN_US)
        LOAD_LANG_FILE(ZH_CN)

#undef LOAD_LANG_FILE
    }
};
}
