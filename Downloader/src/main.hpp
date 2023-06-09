#pragma once

#include <Windows.h>
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include "Logger.h"
#include "renderer/renderer.h"
#include "utils/Utils.h"
#include "config/Field.h"
#include "misc/ResourcesLoader.hpp"
#include "osu/BeatmapManager.h"
#include "osu/OsuConfigManager.h"
#include "ui/MainUi.h"
#include "features/Settings.h"
#include "misc/VersionManager.h"
#include "utils/gui_utils.h"

void InitFeatures();

void Run(HMODULE *phModule) {
    utils::SetMyModuleHandle(*phModule);
    utils::SetCurrentDirPath(utils::GetModulePath(phModule).parent_path());

    config::Init();

    auto &settings = features::Settings::GetInstance();

    if (settings.f_EnableConsole.getValue()) {
        AllocConsole();
        freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE **)stdin, "CONIN$", "r", stdin);
        freopen_s((FILE **)stderr, "CONOUT$", "w", stderr);
        DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
    }

    LOGI("Waiting for osu! initialization...");
    Sleep(5000);

    osu::BeatmapManager::GetInstance();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    static auto s = (utils::GetCurrentDirPath() / "imgui.ini").string();
    LOGD("Imgui config path: %s", s.c_str());
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = s.c_str();
    io.SetPlatformImeDataFn = nullptr;
    settings.applyTheme();

    InitFeatures();
    
    LPBYTE bytes = nullptr;
    if (const DWORD size = res::LoadEx(IDR_FONT, RT_FONT, &bytes)) {
        renderer::SetCurrentFont(bytes, size, 18.0f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
        ImGui::MergeIconsWithLatestFont(18.f, false);
    }

    LOGD("Check graphics api...");

    wchar_t processPath[MAX_PATH];
    GetModuleFileNameW(nullptr, processPath, MAX_PATH);
    auto path = std::filesystem::path(processPath);
    utils::SetOsuDirPath(path.parent_path());

    // set custom osu path
    if (!settings.f_OsuPath.getValue().empty()) {
        if (const auto op = std::filesystem::path(settings.f_OsuPath.getValue()) / "osu!.exe"; exists(op)) {
            utils::SetOsuDirPath(op.parent_path());
            LOGI("Using custom osu path: %s", op.string().c_str());
        } else {
            LOGW("Osu! path not correct, use deteceted path.");
        }
    }

    wchar_t username[256 + 1];
    DWORD usernameLen = 256 + 1;
    GetUserNameW(username, &usernameLen);
    std::wstring cfgName = L"osu!.";
    cfgName.append(username).append(L".cfg");
    path = path.parent_path() / cfgName;
    osu::OsuConfigManager::Init(path);

    const renderer::GraphicsApiType graphicsApiType = osu::OsuConfigManager::GetInt("CompatibilityContext") == 0
        ? renderer::GraphicsApiType::OpenGL3
        : renderer::GraphicsApiType::D3D11;

    LOGD("Api detected: %s", graphicsApiType == renderer::GraphicsApiType::OpenGL3 ? "OpenGL" : "DirectX");

    renderer::Init(graphicsApiType);
    ui::main::ToggleShow();

    GuiHelper::ShowSuccessToast(i18n::I18nManager::GetInstance().getTextCStr("DownloaderLoadSuccess"));
    misc::VersionManager::CheckUpdate();
}

#include "features/About.h"
#include "features/HandleLinkHook.h"
#include "features/Downloader.h"
#include "features/DownloadQueue.h"
#include "features/MultiDownload.h"
#include "features/CustomHotkey.h"
#include "api/Provider.h"
#include "api/Bancho.h"
#include "api/Sayobot.h"
#include "api/Chimu.h"

#define INIT_FEAT(NAME) ui::main::AddFeature(&features::NAME::GetInstance())

inline void InitFeatures() {
    INIT_FEAT(About);
    INIT_FEAT(CustomHotkey);
    INIT_FEAT(Settings);
    INIT_FEAT(Downloader);
    INIT_FEAT(HandleLinkHook);
    INIT_FEAT(MultiDownload);
    INIT_FEAT(DownloadQueue);

    api::Provider::Register(new api::Bancho());
    api::Provider::Register(new api::Sayobot());
    api::Provider::Register(new api::Chimu());
}
