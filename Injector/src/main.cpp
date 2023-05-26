#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <tlhelp32.h>
#include <tchar.h>

using namespace std;

#define LOGI(msg, ...)  printf("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOGE(msg, ...)  printf("[ERROR] " msg "\n", ##__VA_ARGS__)
#define LOGW(msg, ...)  printf("[WARNING] " msg "\n", ##__VA_ARGS__)

bool InjectDll(HANDLE hProc, const char *dllName) {
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
    lpBaseAddress = VirtualAllocEx(hProc, NULL, strlen(dllName) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (lpBaseAddress == NULL) {
        LOGE("Failed to allocate memory in target process, error code: %d", GetLastError());
        return false;
    }
    if (!WriteProcessMemory(hProc, lpBaseAddress, dllName, strlen(dllName) + 1, NULL)) {
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

DWORD GetPidByName(const char *name) {
    TCHAR tszProcess[64] = { 0 };

    STARTUPINFO st;
    PROCESS_INFORMATION pi;
    PROCESSENTRY32 ps;
    HANDLE hSnapshot;

    memset(&st, 0, sizeof st);
    st.cb = sizeof st;
    memset(&ps, 0, sizeof ps);
    ps.dwSize = sizeof ps;
    memset(&pi, 0, sizeof pi);

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) return 0;
    if (!Process32First(hSnapshot, &ps)) return 0;
    do {
        if (lstrcmp(ps.szExeFile, tszProcess) == 0) {
            CloseHandle(hSnapshot);
            return ps.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &ps));
    CloseHandle(hSnapshot);
    return 0;
}
int main() {
    LOGI("Waiting for osu! to start...");
    DWORD pid = 0;
    while ((pid = GetPidByName("osu!.exe")) == 0) {
        Sleep(1000);
    }
    LOGI("osu! found, pid: %d", pid);
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProc == NULL) {
        LOGE("Failed to open process, error code: %d", GetLastError());
        Sleep(5000);
        return 1;
    }

    if (InjectDll(hProc, "Downloader.dll")) {
        LOGI("Dll injected successfully");
    }

    Sleep(3000);
}
