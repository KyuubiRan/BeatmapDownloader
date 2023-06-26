#include "pch.h"
#include "Provider.h"
#include "Provider.h"

namespace api {

Provider::Provider(std::string name, std::string doc, features::downloader::DownloadMirror mirror) : _name(name), _doc(doc), _enum(mirror) {}

std::string Provider::GetDoc() const { return _doc; }
std::string Provider::GetName() const { return _name; }

features::downloader::DownloadMirror Provider::GetEnum() const { return _enum; }

Provider const *Provider::GetRegisteredByName(std::string name) {
    auto vec = Provider::GetRegistered();
    return *std::find_if(vec.begin(), vec.end(), [&name](auto p) { return p->GetName() == name; });
}

Provider const *Provider::GetRegisteredByEnum(features::downloader::DownloadMirror mirror) {
    auto vec = Provider::GetRegistered();
    return *std::find_if(vec.begin(), vec.end(), [&mirror](auto p) { return p->GetEnum() == mirror; });
}

std::vector<const Provider *> Provider::_providers;

void Provider::Register(const Provider *provider) { _providers.push_back(provider); }

void Provider::UnRegisterAll() {
    for (auto it : Provider::_providers) {
        delete it;
    }
}

}; // namespace api