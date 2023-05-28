#include "pch.h"
#include "main.hpp"

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Run, new HMODULE(hModule), 0, NULL);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            if (renderer::g_msgHook != NULL) {
                UnhookWindowsHookEx(renderer::g_msgHook);
            }
            break;
    }
    return TRUE;
}
