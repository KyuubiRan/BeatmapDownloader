#pragma once

#include <Windows.h>
#include <string>
#include <filesystem>

namespace utils {

void SetMyModuleHandle(HMODULE hModule);
HMODULE GetMyModuleHandle();

void SetCurrentDirPath(const std::filesystem::path &path);
std::filesystem::path GetCurrentDirPath();
std::filesystem::path GetModulePath(HMODULE *hModule);
void SetOsuDirPath(const std::filesystem::path &path);
std::filesystem::path GetOsuDirPath();

std::wstring s2ws(const std::string &s);
std::string ws2s(const std::wstring &s);

std::string &trim_end(std::string &str);
std::string &trim_begin(std::string &str);
std::string &trim(std::string &str);

}
