#pragma once

#include <Windows.h>
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include "Logger.h"
#include "renderer/renderer.h"
#include "ui/MainUi.h"

void Run(HMODULE *phModule) {
    AllocConsole();
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE **)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE **)stderr, "CONOUT$", "w", stderr);

    LOGI("Waiting for osu! initialization...");
    Sleep(3000);
    
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    auto s = (std::filesystem::current_path() / "imgui.ini").string();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = s.c_str();
    io.SetPlatformImeDataFn = nullptr;
    ImGui::StyleColorsDark();

    LOGD("Check graphics api...");

    renderer::GraphicsApiType graphicsApiType = renderer::GraphicsApiType::OpenGL3;

    wchar_t processPath[MAX_PATH];
    GetModuleFileNameW(nullptr, processPath, MAX_PATH);
    auto path = std::filesystem::path(processPath);
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