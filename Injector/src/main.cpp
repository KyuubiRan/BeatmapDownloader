#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <filesystem>
#include <fstream>
using namespace std;

#define LOGD(msg, ...)  printf_s("[DEBUG] " msg "\n", ##__VA_ARGS__)
#define LOGI(msg, ...)  printf_s("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOGW(msg, ...)  printf_s("[WARNING] " msg "\n", ##__VA_ARGS__)
#define LOGE(msg, ...)  printf_s("[ERROR] " msg "\n", ##__VA_ARGS__)

bool InjectDll(HANDLE hProc, const char *path) {

    LPVOID lpBaseAddress = NULL;
    HMODULE hKernel32 = GetModuleHandleA("Kernel32.dll");
    if (hKernel32 == NULL) {
        LOGE("Failed to get kernel32.dll handle, error code: %d", GetLastError());
        return false;
    }
    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibrary == NULL) {
        LOGE("Failed to get LoadLibraryA address, error code: %d", GetLastError());
        return false;
    }
    lpBaseAddress = VirtualAllocEx(hProc, NULL, strlen(path) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (lpBaseAddress == NULL) {
        LOGE("Failed to allocate memory in target process, error code: %d", GetLastError());
        return false;
    }
    if (!WriteProcessMemory(hProc, lpBaseAddress, path, strlen(path) + 1, NULL)) {
        LOGE("Failed to write dll name to target process, error code: %d", GetLastError());
        return false;
    }
    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, lpBaseAddress, 0, NULL);
    if (hThread == NULL) {
        LOGE("Failed to create remote thread, error code: %d", GetLastError());
        return false;
    }
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProc, lpBaseAddress, 0, MEM_RELEASE);
    return true;
}

DWORD GetPidByName(LPCWSTR name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof pe;

    if (Process32FirstW(hSnapshot, &pe)) {
        if (lstrcmpW(pe.szExeFile, name) == 0) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    } else {
        CloseHandle(hSnapshot);
        return 0;
    }

    while (Process32NextW(hSnapshot, &pe)) {
        if (lstrcmpW(pe.szExeFile, name) == 0) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    }

    CloseHandle(hSnapshot);
    return 0;
}

std::string ws2s(const std::wstring &s) {
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    char *buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
    std::string r(buf);
    delete[] buf;
    return r;
}

int main(int argc, char *argv[]) {
    bool noAutoStart = false;

    if (argc > 1) {
        if (strcmp(argv[1], "--no-auto-start") == 0) {
            noAutoStart = true;
        }
    }

    const auto dllPath = std::filesystem::current_path() / "Downloader.dll";

    if (noAutoStart) {
        LOGI("Waiting for osu! to start...");
        DWORD pid = 0;
        while ((pid = GetPidByName(L"osu!.exe")) == 0) {
            Sleep(1000);
        }
        LOGI("osu! found, pid: %d", pid);
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (hProc == NULL) {
            LOGE("Failed to open process, error code: %d", GetLastError());
            Sleep(5000);
            return 1;
        }

        LOGI("Injecting dll...");
        const auto sPath = dllPath.string();
        LOGI("DLL path: %s", sPath.c_str());

        if (InjectDll(hProc, sPath.c_str())) {
            LOGI("Dll injected successfully");
        }
    } else {
        auto path = std::filesystem::current_path() / "osupath.txt";
        std::string line;

        if (exists(path)) {
            std::ifstream ifs(path);
            if (!ifs) {
                LOGE("Cannot open osupath.txt!");
                Sleep(3000);
                exit(1);
            }
            std::getline(ifs, line);
        }

        if (line.empty() || !filesystem::exists(line)) {
            // Show select osu!.exe dialog
            OPENFILENAME ofn;
            TCHAR szFile[MAX_PATH] = {0};
            ZeroMemory(&ofn, sizeof ofn);
            ofn.lStructSize = sizeof ofn;
            ofn.hwndOwner = nullptr;
            ofn.lpstrFilter = _T("osu!.exe\0osu!.exe\0");
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrTitle = _T("Select osu!.exe");
            ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                line = ws2s(ofn.lpstrFile);
                std::ofstream ofs(path, std::ios::out | std::ios::trunc);
                ofs << line;
                LOGD("Line: %s", line.c_str());
            } else {
                LOGE("Failed to select osu!.exe!");
                Sleep(3000);
                exit(1);
            }
        }

        if (line.empty() || !filesystem::exists(line)) {
            LOGE("Cannot find osu!.exe!");
            Sleep(3000);
            exit(1);
        }

        std::filesystem::path osupath = line;
        LOGI("osu! path: %s", osupath.string().c_str());

        // Start osu!
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof si;
        if (!CreateProcessW(osupath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            LOGE("Failed to start osu!, error code: %d", GetLastError());
            Sleep(3000);
            exit(1);
        }

        LOGI("osu! started, pid: %d", pi.dwProcessId);
        if (InjectDll(pi.hProcess, dllPath.string().c_str())) {
            LOGI("Dll injected successfully");
        }
    }

    Sleep(3000);
}
