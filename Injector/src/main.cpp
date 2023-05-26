#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <filesystem>
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

int main() {
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
    auto path = std::filesystem::current_path() / "Downloader.dll";
    auto sPath = path.string();
    LOGI("DLL path: %s", sPath.c_str());

    if (InjectDll(hProc, sPath.c_str())) {
        LOGI("Dll injected successfully");
    }

    Sleep(3000);
}
