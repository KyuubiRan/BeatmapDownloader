#include "pch.h"
#include "Sayobot.h"

#include <winhttp.h>

#include "utils/Utils.h"

void api::sayobot::BidData::to_json(nlohmann::json &j) const {
    j.at("bid") = bid;
    j.at("mode") = mode;
}

void api::sayobot::BidData::from_json(const nlohmann::json &j) {
    j.at("bid").get_to(bid);
    j.at("mode").get_to(mode);
}

void api::sayobot::SayoBeatmapDataV2::to_json(nlohmann::json &j) const {
    j.at("approved") = approved;
    j.at("artist") = artist;
    j.at("artistU") = artistU;
    j.at("creator") = creator;
    j.at("creator_id") = creatorId;
    j.at("title") = title;
    j.at("titleU") = titleU;
    j.at("sid") = sid;
    j.at("video") = video;
    j.at("bid_data") = bidData;
}

void api::sayobot::SayoBeatmapDataV2::from_json(const nlohmann::json &j) {
    j.at("approved").get_to(approved);
    j.at("artist").get_to(artist);
    j.at("artistU").get_to(artistU);
    j.at("creator").get_to(creator);
    j.at("creator_id").get_to(creatorId);
    j.at("title").get_to(title);
    j.at("titleU").get_to(titleU);
    j.at("sid").get_to(sid);
    j.at("video").get_to(video);
    j.at("bid_data").get_to(bidData);
}

std::optional<api::sayobot::SayoResult<api::sayobot::SayoBeatmapDataV2>> api::sayobot::SearchBeatmapV2(
    const features::downloader::BeatmapInfo &info) {
    auto &dl = features::Downloader::GetInstance();

    std::optional<SayoResult<SayoBeatmapDataV2>> ret = {};

    HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;


    const std::wstring ua = dl.f_EnableCustomUserAgent.getValue()
                                ? utils::s2ws(dl.f_CustomUserAgent.getValue())
                                : features::Downloader::DEFAULT_USER_AGENT;

    const std::wstring proxy = dl.f_EnableProxy.getValue()
                                   ? utils::s2ws(dl.f_ProxySever.getValue())
                                   : L"";

    bool useProxy = dl.f_EnableProxy.getValue() && !proxy.empty();
    if (dl.f_EnableProxy.getValue() && proxy.empty()) {
        LOGW("Proxy is enabled but proxy server is empty, using default proxy");
    }

    const std::wstring proxyPass = dl.f_EnableProxy.getValue()
                                       ? utils::s2ws(dl.f_ProxySeverPassword.getValue())
                                       : L"";

    hSession = WinHttpOpen(ua.c_str(),
                           useProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           useProxy ? proxy.c_str() : WINHTTP_NO_PROXY_NAME,
                           useProxy && !proxyPass.empty() ? proxyPass.c_str() : WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        std::wstring req = std::format(L"https://api.sayobot.cn//v2/beatmapinfo?K={0}&T={1}", info.id, (uint8_t)info.type);

        hConnect = WinHttpConnect(hSession, req.c_str(),
                                  INTERNET_DEFAULT_HTTP_PORT, 0);
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/",
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      0);
    }

    if (hRequest) {
        if (!WinHttpSendRequest(hRequest,
                                WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0,
                                0, 0)) {
            LOGW("Cannot send request to Sayobot API");
            goto quit;
        }
    }

    if (WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        std::ostringstream responseStream;

        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                printf("Error %lu in WinHttpQueryDataAvailable.\n", GetLastError());
                goto quit;
            }

            std::vector<char> buffer(dwSize + 1, 0);

            if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                LOGW("Error %lu in WinHttpReadData.", GetLastError());
                goto quit;
            }

            responseStream.write(buffer.data(), dwDownloaded);
        } while (dwSize > 0);

        std::string response = responseStream.str();
        LOGD("Sayo search beatmap response: %s", response.c_str());
        nlohmann::json j = nlohmann::json::parse(response);
        SayoResult<SayoBeatmapDataV2> result{};
        result.from_json(j);
        ret = result;
    } else {
        LOGW("Cannot receive response from Sayobot API");
    }

quit:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return ret;
}

std::vector<BYTE> api::sayobot::DownloadBeatmap(features::downloader::BeatmapInfo &info, uint64_t *cur, uint64_t *ttl) {
    std::vector<BYTE> ret;
    bool enableProgress = cur != nullptr && ttl != nullptr;
    auto &dl = features::Downloader::GetInstance();
    bool succeed = false;

    HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;


    const std::wstring ua = dl.f_EnableCustomUserAgent.getValue()
                                ? utils::s2ws(dl.f_CustomUserAgent.getValue())
                                : features::Downloader::DEFAULT_USER_AGENT;

    const std::wstring proxy = dl.f_EnableProxy.getValue()
                                   ? utils::s2ws(dl.f_ProxySever.getValue())
                                   : L"";

    bool useProxy = dl.f_EnableProxy.getValue() && !proxy.empty();
    if (dl.f_EnableProxy.getValue() && proxy.empty()) {
        LOGW("Proxy is enabled but proxy server is empty, using default proxy");
    }

    const std::wstring proxyPass = dl.f_EnableProxy.getValue()
                                       ? utils::s2ws(dl.f_ProxySeverPassword.getValue())
                                       : L"";

    hSession = WinHttpOpen(ua.c_str(),
                           useProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           useProxy ? proxy.c_str() : WINHTTP_NO_PROXY_NAME,
                           useProxy && !proxyPass.empty() ? proxyPass.c_str() : WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        auto &dltype = dl.f_DownloadType.getValue();
        std::string req = std::format("https://txy1.sayobot.cn/beatmaps/download/{0}/{1}?server=auto",
                                       dltype == features::downloader::DownloadType::Full ? "full" : "novideo", info.id);
        

        hConnect = WinHttpConnect(hSession, utils::s2ws(req).c_str(),
                                  INTERNET_DEFAULT_HTTP_PORT, 0);
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/",
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      0);
    }

    if (hRequest) {
        if (!WinHttpSendRequest(hRequest,
                                WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0,
                                0, 0)) {
            LOGW("Cannot send request to Sayobot API");
            goto quit;
        }
    }

    if (WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                printf("Error %lu in WinHttpQueryDataAvailable.\n", GetLastError());
                goto quit;
            }

            std::vector<char> buffer(dwSize + 1, 0);

            if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                LOGW("Error %lu in WinHttpReadData.", GetLastError());
                goto quit;
            }

            ret.insert(ret.end(), buffer.begin(), buffer.end());
        } while (dwSize > 0);

        succeed = true;
    } else {
        LOGW("Cannot receive response from Sayobot API");
    }

quit:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return succeed ? ret : std::vector<BYTE>();
}

void nlohmann::adl_serializer<api::sayobot::BidData, void>::to_json(json &j, const api::sayobot::BidData &data) {
    data.to_json(j);
}

void nlohmann::adl_serializer<api::sayobot::BidData, void>::from_json(const json &j, api::sayobot::BidData &data) {
    data.from_json(j);
}
