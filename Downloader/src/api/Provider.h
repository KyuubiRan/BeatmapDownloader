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
    Provider(std::string, std::string, features::downloader::DownloadMirror);

    std::string GetDoc() const;
    std::string GetName() const;

    features::downloader::DownloadMirror GetEnum() const;

    virtual std::optional<osu::Beatmap> SearchBeatmap(const features::downloader::BeatmapInfo &) const = 0;
    virtual bool DownloadBeatmap(const osu::Beatmap &) const = 0;

    static const std::vector<const Provider *> &GetRegistered() { return Provider::_providers; }
    static Provider const *GetRegisteredByName(std::string);
    static Provider const *GetRegisteredByEnum(features::downloader::DownloadMirror);
    static void Register(const Provider *provider);

    static void UnRegisterAll();
};

}; // namespace api