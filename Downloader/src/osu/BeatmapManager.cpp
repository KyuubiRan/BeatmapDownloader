#include "pch.h"
#include <fstream>

#include <shellapi.h>
#include "BeatmapManager.h"

#include "features/HandleLinkHook.h"
#include "utils/Utils.h"

osu::BeatmapManager::BeatmapManager() {
    initDatabase();
}

// taken from https://github.com/veritas501/Osu-Ingame-Downloader/blob/master/OsuIngameDownloader/map_db.cpp#L15
#define fr(obj) fs.read((char*) &(obj), sizeof(obj))
#define fp(size) fs.seekg(size, std::ios::cur)

unsigned int UnpackULEB128(std::ifstream &fs) {
    char flag = 0;
    fr(flag);
    if (flag != 0xb) {
        return 0;
    }
    unsigned char tmpByte = 0;
    unsigned int decodeNum = 0;
    int i = 0;
    do {
        fr(tmpByte);
        decodeNum += (tmpByte & 0x7f) << 7 * i;
        i++;
    } while (tmpByte >= 0x80);
    return decodeNum;
}

std::string UnpackOsuStr(std::ifstream &fs) {
    const uint32_t size = UnpackULEB128(fs);
    if (!size)
        return "";

    char *tmpStr = new char[size + 1];
    fs.read(tmpStr, size);
    tmpStr[size] = 0;

    std::string ans(tmpStr);
    delete[] tmpStr;
    return ans;
}

void PassOsuStr(std::ifstream &fs) {
    const uint32_t size = UnpackULEB128(fs);
    if (!size)
        return;
    fp(size);
}

void osu::BeatmapManager::initDatabase() {
    const auto osuDatabase = utils::GetOsuDirPath() / "osu!.db";
    if (!exists(osuDatabase)) {
        LOGW("osu!.db not found!");
        return;
    }

    LOGI("Start initialize osu! database...");
    std::ifstream fs;
    int bid, sid;

    fs.open(osuDatabase, std::ios::binary | std::ios::in);
    fp(0x11);
    PassOsuStr(fs);
    int sumBeatmaps;
    fr(sumBeatmaps);
    for (int i = 0; i < sumBeatmaps; i++) {
        for (int j = 0; j < 9; j++) {
            PassOsuStr(fs);
        }
        fp(1 + 2 * 3 + 8 + 4 * 4 + 8);
        for (int j = 0; j < 4; j++) {
            int length = 0;
            fr(length);
            for (int k = 0; k < length; k++) {
                fp(1 + 4 + 1 + 8);
            }
        }
        fp(4 * 3);
        int timingPointsLength;
        fr(timingPointsLength);
        for (int j = 0; j < timingPointsLength; j++) {
            fp(8 * 2 + 1);
        }
        fr(bid);
        fr(sid);
        fp(4 + 1 * 4 + 2 + 4 + 1);
        PassOsuStr(fs);
        PassOsuStr(fs);
        fp(2);
        PassOsuStr(fs);
        fp(1 + 8 + 1);
        PassOsuStr(fs);
        fp(8 + 1 * 5 + 4 + 1);
        if (bid != -1 && bid != 0) {
            m_BeatmapIds.insert(bid);
        }
        if (sid != -1 && sid != 0) {
            m_BeatmapSetIds.insert(sid);
        }
    }
    LOGI("Finished initialize osu! database! sid size = %zu, bid size = %zu", m_BeatmapSetIds.size(), m_BeatmapIds.size());
}

void osu::BeatmapManager::openBeatmapPage(Beatmap &bm) {
    if (bm.sid <= 0) {
        LOGW("Cannot open website: Invalid beatmap id %d!", bm.sid);
        return;
    }
    std::wstring s = L"https://osu.ppy.sh/s/";
    s.append(std::to_wstring(bm.sid));
    WLOGI(L"Opening website: %s", s.c_str());
    
    SHELLEXECUTEINFOW sei = {
        sizeof(sei),
        0,
        nullptr,
        L"open",
        s.c_str(),
        nullptr,
        nullptr,
        SW_HIDE,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr
    };

    // prevent trigger search
    HookManager::CallOriginal(features::HandleLinkHook::ShellExecuteExW_Hook, &sei);
}

#undef fr
#undef fp
