#include "pch.h"
#include "Provider.h"
#include "Provider.h"

namespace api {

Provider::Provider(std::string name, std::string doc, features::downloader::DownloadMirror mirror) :
    _name(std::move(name)), _doc(std::move(doc)), _enum(mirror) {
}

std::string_view Provider::getDoc() const { return _doc; }
std::string_view Provider::getName() const { return _name; }

features::downloader::DownloadMirror Provider::getEnum() const { return _enum; }

Provider const *Provider::GetRegisteredByName(std::string name) {
    auto &vec = GetRegistered();
    return *std::ranges::find_if(vec, [&name](auto p) { return p->getName() == name; });
}

Provider const *Provider::GetRegisteredByEnum(features::downloader::DownloadMirror mirror) {
    auto &vec = GetRegistered();
    return *std::ranges::find_if(vec, [&mirror](auto p) { return p->getEnum() == mirror; });
}

std::vector<const Provider *> Provider::_providers;

void Provider::Register(const Provider *provider) { _providers.push_back(provider); }

void Provider::UnRegisterAll() {
    for (const auto it : _providers) {
        delete it;
    }
    _providers.clear();
}

} // namespace api
