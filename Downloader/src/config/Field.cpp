#include "pch.h"
#include <filesystem>
#include <fstream>
#include "Field.h"
#include "utils/Utils.h"

static nlohmann::json g_config;
static std::vector<config::ISerializable *> g_fields;
static std::mutex g_lock;

void config::Register(config::ISerializable *cfg) {
    g_fields.push_back(cfg);
}

void config::Init() {
    std::lock_guard lock(g_lock);

    auto cfgPath = utils::GetCurrentDirPath() / "cfg.json";
    if (exists(cfgPath)) {
        std::ifstream ifs(cfgPath, std::ios::binary);
        try {
            ifs >> g_config;
        } catch (...) {
            LOGW("Config file broken, reset to default");
        }
    }
}

void config::Load(ISerializable *cfg) {
    cfg->from_json(g_config);
}

void config::Save() {
    std::lock_guard lock(g_lock);

    for (auto &i : g_fields) {
        i->to_json(g_config);
    }
    auto cfgPath = utils::GetCurrentDirPath() / "cfg.json";
    std::ofstream ofs(cfgPath, std::ios::binary | std::ios::trunc);
    ofs << g_config.dump();
}