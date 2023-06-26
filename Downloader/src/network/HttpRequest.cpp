#include "pch.h"
#include "HttpRequest.h"

#include "features/Downloader.h"
#include "misc/glob.h"
#include "utils/gui_utils.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

size_t string_write_fn(char *data, size_t size, size_t nmemb, std::string *writerData) {
    if (!data)
        return 0;
    writerData->append(data, size * nmemb);
    return size * nmemb;
}

CURLcode net::curl_get(const char *url, std::string &response, const std::vector<std::string> &extraHeader, int32_t *resCode) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        LOGE("Cannot init curl");
        return CURLE_FAILED_INIT;
    }

    auto &dl = features::Downloader::GetInstance();
    
    curl_slist *headers = nullptr;

    const std::string ua = "User-Agent: " + (dl.f_EnableCustomUserAgent.getValue()
         ? dl.f_CustomUserAgent.getValue()
         : features::Downloader::DEFAULT_USER_AGENT);
    headers = curl_slist_append(headers, ua.c_str());
    
    for (const auto &s : extraHeader) {
        headers = curl_slist_append(headers, s.c_str());
    }
 
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_write_fn);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 30000);


    if (dl.f_ProxySeverType.getValue() != features::downloader::ProxyServerType::Disabled) {
        curl_easy_setopt(curl, CURLOPT_PROXY, dl.f_ProxySever->c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE,
                         dl.f_ProxySeverType == features::downloader::ProxyServerType::Socks5 ? CURLPROXY_SOCKS5 : CURLPROXY_HTTP);
        if (!dl.f_ProxySeverPassword->empty())
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, dl.f_ProxySeverPassword->c_str());
    }

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        int32_t code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (resCode)
            *resCode = code;
    } else {
        LOGW("Curl req(get) error: %d", res);
        GuiHelper::ShowErrorToast(i18n::I18nManager::GetTextCStr("CurlError"), res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
}

void net::curl_get_async(std::string url, std::vector<std::string> extraHeader, std::function<void(int, std::string &)> callback) {
    glob::s_ThreadPool.post([u = std::move(url), h = std::move(extraHeader), c = std::move(callback)] {
        std::string response;
        int32_t code;
        curl_get(u.c_str(), response, h, &code);
        c(code, response);
    });
}

size_t file_writer(char *data, size_t size, size_t nmemb, FILE *pFile) {
    if (!data)
        return 0;
    fwrite(data, size, nmemb, pFile);
    return size * nmemb;
}

int xferinfo_fn(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    const auto pTask = (features::DownloadTask *)clientp;
    pTask->dlSize = static_cast<double>(dlnow);
    pTask->totalSize = static_cast<double>(dltotal);
    return 0;
}

CURLcode net::curl_download(const char *url, std::filesystem::path &path, std::vector<std::string> &extraHeader, features::DownloadTask *task,
                            int32_t *resCode) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        LOGE("Cannot init curl");
        return CURLE_FAILED_INIT;
    }

    FILE *f;
    if (!exists(path.parent_path()))
        create_directories(path.parent_path());

    fopen_s(&f, path.string().c_str(), "wb");
    if (!f) {
        LOGW("Cannot open file %s", path.string().c_str());
        return CURLE_FAILED_INIT;
    }

    auto &dl = features::Downloader::GetInstance();

    curl_slist *headers = nullptr;
    const std::string ua = "User-Agent: " + (dl.f_EnableCustomUserAgent.getValue()
        ? dl.f_CustomUserAgent.getValue()
        : features::Downloader::DEFAULT_USER_AGENT);

    for (const auto &s : extraHeader) {
        curl_slist_append(headers, s.c_str());
    }
    curl_slist_append(headers, ua.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_writer);
    if (task != nullptr) {
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, task);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo_fn);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 30000);

    if (dl.f_ProxySeverType.getValue() != features::downloader::ProxyServerType::Disabled) {
        curl_easy_setopt(curl, CURLOPT_PROXY, dl.f_ProxySever->c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE,
                         dl.f_ProxySeverType == features::downloader::ProxyServerType::Socks5 ? CURLPROXY_SOCKS5 : CURLPROXY_HTTP);
        if (!dl.f_ProxySeverPassword->empty())
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, dl.f_ProxySeverPassword->c_str());
    }

    const CURLcode res = curl_easy_perform(curl);
    fclose(f);

    if (res == CURLE_OK) {
        int32_t code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (resCode)
            *resCode = code;
    } else {
        LOGW("Curl req(dl) error: %d", res);
        GuiHelper::ShowErrorToast(i18n::I18nManager::GetTextCStr("CurlError"), res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
}
