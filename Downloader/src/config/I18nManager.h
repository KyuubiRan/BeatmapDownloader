#pragma once

#include <map>
#include <json.hpp>
#include "Field.h"
#include "misc/ResourcesLoader.hpp"
#include "../resource.h"

namespace i18n {

enum class Language : uint8_t {
    EN_US = 1,
    ZH_CN = 2,
};

class I18nManager {
    // i18nMap["en_us"]["Downloader"] -> "Downloader"
    inline static std::map<std::string, std::map<std::string, std::string>> i18nMap;

    config::Field<Language> lang;

    static const char *GetLanguageName(Language lang) {
        switch (lang) {
            case Language::EN_US:
                return "EN_US";
            case Language::ZH_CN:
                return "ZH_CN";
            default:
                return "EN_US";
        }
    }

public:
    static I18nManager &GetInstance() {
        static I18nManager instance;
        return instance;
    }

    std::string_view GetText(std::string_view key) {
        auto &map = i18nMap[GetLanguageName(lang)];
        if (map.contains(key.data())) {
            return map[key.data()];
        }
        return key;
    }

private:
    I18nManager() : lang(config::Field("Language", Language::EN_US)) {
        BYTE * data = nullptr;

#define LOAD_LANG_FILE(lang) \
        if (res::LoadEx(IDR_LANG_##lang, RT_RCDATA, &data)) { \
            std::string s = std::string(reinterpret_cast<char *>(data)); \
            nlohmann::json j = nlohmann::json::parse(s); \
            i18nMap[#lang] = j.get<std::map<std::string, std::string>>(); \
        }

        LOAD_LANG_FILE(EN_US)
        LOAD_LANG_FILE(ZH_CN)

#undef LOAD_LANG_FILE
    }
};

}
