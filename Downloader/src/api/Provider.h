#pragma once
#include <string>
#include <vector>
#include "features/Downloader.h"

namespace api {

class Provider {
private:
    static std::vector<const Provider *> _providers;

    std::string _name, _doc;
    features::downloader::DownloadMirror _enum;

public:
    virtual ~Provider() = default;
    Provider(std::string, std::string, features::downloader::DownloadMirror);

    std::string_view getDoc() const;
    std::string_view getName() const;

    features::downloader::DownloadMirror GetEnum() const;

    virtual std::optional<osu::Beatmap> searchBeatmap(const features::downloader::BeatmapInfo &) const = 0;
    virtual bool downloadBeatmap(const osu::Beatmap &) const = 0;

    static const std::vector<const Provider *> &GetRegistered() { return _providers; }
    static Provider const *GetRegisteredByName(std::string);
    static Provider const *GetRegisteredByEnum(features::downloader::DownloadMirror);
    static void Register(const Provider *provider);

    static void UnRegisterAll();
};

}; // namespace api
