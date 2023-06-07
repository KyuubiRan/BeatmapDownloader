#pragma once

#include <curl.h>
#include <string>
#include <fstream>
#include <vector>

#include "features/DownloadQueue.h"

#pragma comment(lib, "crypt32.lib")

namespace net {
CURLcode curl_get(const char *url, std::string &response, std::vector<std::string> &extraHeader, int32_t *resCode = nullptr);

inline CURLcode curl_get(const char *url, std::string &response, int32_t *resCode = nullptr) {
    std::vector<std::string> empty = {};
    return curl_get(url, response, empty, resCode);
}

CURLcode curl_download(const char *url, std::filesystem::path &path, std::vector<std::string> &extraHeader, features::DownloadTask *task = nullptr,
                       int32_t *resCode = nullptr);

inline CURLcode curl_download(const char *url, std::filesystem::path &path, features::DownloadTask *task = nullptr, int32_t *resCode = nullptr) {
    std::vector<std::string> empty = {};
    return curl_download(url, path, empty, task, resCode);
}
}
