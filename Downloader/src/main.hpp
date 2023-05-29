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
#include "ui/MainUi.h"
#include "ui/Settings.h"

void InitFeatures();

void Run(HMODULE *phModule) {
    utils::SetMyModuleHandle(*phModule);
    utils::SetCurrentDirPath(utils::GetModulePath(phModule).parent_path());

    config::Init();

    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);

    LOGI("Waiting for osu! initialization...");
    Sleep(5000);

    osu::BeatmapManager::GetInstance();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    auto s = (utils::GetCurrentDirPath() / "imgui.ini").string();
    LOGD("Imgui config path: %s", s.c_str());
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = s.c_str();
    io.SetPlatformImeDataFn = nullptr;
    ImGui::StyleColorsDark();

    InitFeatures();

    LPBYTE bytes = nullptr;
    if (const DWORD size = res::LoadEx(IDR_FONT, RT_FONT, &bytes)) {
        renderer::SetCurrentFont(bytes, size, 18.0f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
    }

    LOGD("Check graphics api...");

    renderer::GraphicsApiType graphicsApiType = renderer::GraphicsApiType::OpenGL3;

    wchar_t processPath[MAX_PATH];
    GetModuleFileNameW(nullptr, processPath, MAX_PATH);
    auto path = std::filesystem::path(processPath);
    utils::SetOsuDirPath(path.parent_path());
    wchar_t username[256 + 1];
    DWORD usernameLen = 256 + 1;
    GetUserNameW(username, &usernameLen);
    std::wstring cfgName = L"osu!.";
    cfgName.append(username).append(L".cfg");
    path = path.parent_path() / cfgName;
    LOGD("Osu! config path: %s", path.string().c_str());

    std::ifstream ifs(path);
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.starts_with("CompatibilityContext")) {
            LOGD("%s", line.c_str());
            graphicsApiType = line.find("0") != std::string::npos ? renderer::GraphicsApiType::OpenGL3 : renderer::GraphicsApiType::D3D11;
            break;
        }
    }
    ifs.close();

    LOGD("Api detected: %s", graphicsApiType == renderer::GraphicsApiType::OpenGL3 ? "OpenGL" : "DirectX");

    renderer::Init(graphicsApiType);
}

#include "ui/About.h"
#include "hook/HandleLinkHook.h"

inline void InitFeatures() {
    AddFeature(&ui::misc::About::GetInstance());
    AddFeature(&ui::misc::Settings::GetInstance());
    AddFeature(&hook::HandleLinkHook::GetInstance());
}