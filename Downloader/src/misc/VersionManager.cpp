#include "pch.h"
#include "VersionManager.h"

#include "network/HttpRequest.h"
#include "utils/gui_utils.h"

void misc::VersionManager::CheckUpdate(const bool force) {
#ifdef SKIP_VERSION_CHECK
    LOGI("Skipped version check!");
    GuiHelper::ShowInfoToast("Skipped version check!");
#endif
    
    static bool checked = false;
    static bool inCheck = false;

    if (inCheck)
        return;

    if (checked && !force)
        return;

    GET_LANG();
    inCheck = true;
    GuiHelper::ShowInfoToast(lang.getTextCStr("VersionChecking"));
    static std::vector<std::string> headers = {
        "Accept: application/vnd.github+json",
        "X-GitHub-Api-Version: 2022-11-28"
    };
    net::curl_get_async(
        "https://api.github.com/repos/KyuubiRan/BeatmapDownloader/releases/latest", headers,
        [&](const int code, std::string &res) {
            if (code != 200) {
                LOGW("Cannot check update, response code: %d, response body: %s", code, res.c_str());
                GuiHelper::ShowWarnToast(lang.getTextCStr("VersionCheckFailed"), code);
                inCheck = false;
                return;
            }
            nlohmann::json j = nlohmann::json::parse(res);
            s_LatestVersionName = j["name"];
            const std::string vc = j["tag_name"];
            s_LatestVersionCode = std::stoi(vc);
            checked = true;
            inCheck = false;
            if (IsLatest()) {
                LOGI("Downloader is up to date");
                GuiHelper::ShowSuccessToast(lang.getTextCStr("VersionIsLatest"));
            } else {
                LOGI("Found new version: %s(%d)", s_LatestVersionName.c_str(), s_LatestVersionCode);
                GuiHelper::ShowInfoToast(lang.getTextCStr("FoundNewVersion"), s_LatestVersionName.c_str(),
                                            s_LatestVersionCode);
            }
        });
}
