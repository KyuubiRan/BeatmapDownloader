#include "pch.h"
#include "Bancho.h"

#include "features/Downloader.h"
#include "network/HttpRequest.h"
#include "utils/Utils.h"
#include "utils/gui_utils.h"

api::Bancho::Bancho() : Provider("Bancho", "https://github.com/ppy/osu-api/wiki", features::downloader::DownloadMirror::OsuOfficial) {}

std::optional<osu::Beatmap> api::Bancho::SearchBeatmap(const features::downloader::BeatmapInfo &info) const {
    auto &dl = features::Downloader::GetInstance();

    auto un = dl.f_OsuAccount->username();
    auto pw = dl.f_OsuAccount->password().md5();

    if (un.empty() || pw.empty()) {
        LOGW("Search failed: Osu account not set");
        return {};
    }

    LOGD("Begin official search type=%s, id=%d", info.type == features::downloader::BeatmapType::Sid ? "Sid" : "Bid", info.id);

    // https://osu.ppy.sh/web/osu-search-set.php?u={username}&h={md5hash}&{type}={id};

    auto url = std::format("https://osu.ppy.sh/web/osu-search-set.php?u={0}&h={1}&{2}={3}", un, pw,
                           info.type == features::downloader::BeatmapType::Sid ? "s" : "b", info.id);
    // replace ' ' to "%20"
    for (size_t i = 0; i < url.size(); ++i) {
        if (url[i] == ' ') {
            url.replace(i, 1, "%20");
        }
    }

    std::string res;
    int ret = -1;
    if (const CURLcode code = net::curl_get(url.c_str(), res, &ret); code == CURLE_OK && ret == 200) {
        LOGD("Official search result: %s", res.c_str());
        if (res.empty()) {
            LOGW("Official search failed! response is empty!");
            return {};
        }

        const auto l = utils::split(res, '|');
        if (l.empty()) {
            LOGW("Official search failed! Cannot parse result!");
            return {};
        }

#pragma warning(push)
#pragma warning(disable : 4834)

        try {
            auto bm = osu::Beatmap{};
            size_t i = 0;
            l[i++]; // serverFilename
            bm.artist = l[i++];
            bm.title = l[i++];
            bm.author = l[i++];
            l[i++]; // status
            /*
            "1" => SubmissionStatus.Ranked,
            "2" => SubmissionStatus.Approved,
            "3" => SubmissionStatus.Qualified,
            "4" => SubmissionStatus.Loved,
            _ => SubmissionStatus.Pending
            */

            l[i++]; // rating
            l[i++]; // lastupdate

            bm.sid = std::stoi(l[i++]);
            l[i++]; // threadid

            bm.hasVideo = l[i++] == "1";
            l[i++]; // hasStoryboard
            l[i++]; // filesize

            LOGD("Parsed official search result: %d %s - %s", bm.sid, bm.artist.c_str(), bm.title.c_str());
            return bm;
        } catch (...) {
            LOGW("Cannot parse bancho search result, maybe beatmap not exists!");
        }

#pragma warning(pop)

    } else {
        LOGW("Osu official search failed: CURL_CODE=%d, RESPONSE_CODE=%d", code, ret);
    }

    return {};
}

bool api::Bancho::DownloadBeatmap(const osu::Beatmap &bm) const {
    auto &dl = features::Downloader::GetInstance();

    auto un = dl.f_OsuAccount->username();
    auto pw = dl.f_OsuAccount->password().md5();

    if (un.empty() || pw.empty()) {
        LOGW("Download failed: Osu account not set");
        return false;
    }

    const bool noVideo = dl.f_DownloadType.getValue() == features::downloader::DownloadType::NoVideo && bm.hasVideo;

    // https://osu.ppy.sh/d/{setid}?u={username}&h={md5hash}&vv=2
    // https://osu.ppy.sh/d/{0}{novideo}?u={username}&h={md5hash}&vv=2
    // https://osu.ppy.sh/d/22276?u=abcd&h=abcd&vv=2
    // https://osu.ppy.sh/d/22276n?u=abcd&h=abcd&vv=2
    auto url = std::format("https://osu.ppy.sh/d/{0}{1}?u={2}&h={3}&vv=2", bm.sid, noVideo ? "n" : "", un, pw);
    // replace ' ' to "%20"
    for (size_t i = 0; i < url.size(); ++i) {
        if (url[i] == ' ') {
            url.replace(i, 1, "%20");
        }
    }

    auto path = utils::GetCurrentDirPath() / L"downloads" / (std::to_string(bm.sid) + ".osz");
    auto *tsk = features::DownloadQueue::GetInstance().getTask(bm);
    int ret = -1;
    if (const CURLcode code = net::curl_download(url.c_str(), path, tsk, &ret); code == CURLE_OK && ret == 200) {
        LOGI("Success download beatmapsets: %d", bm.sid);
        return true;
    } else {
        LOGW("Osu official download failed: CURL_CODE=%d, RESPONSE_CODE=%d", ret, ret);
        return false;
    }
}
