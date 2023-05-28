#pragma once

#include <cstdio>

#ifdef _DEBUG
#define LOGD(msg, ...)  printf_s("[DEBUG] " msg "\n", ##__VA_ARGS__)
#define WLOGD(msg, ...)  wprintf_s(L"[DEBUG] " msg L"\n", ##__VA_ARGS__)
#else 
#define LOGD(msg, ...)
#define WLOGD(msg, ...)
#endif

#define LOGI(msg, ...)  printf_s("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOGW(msg, ...)  printf_s("[WARNING] " msg "\n", ##__VA_ARGS__)
#define LOGE(msg, ...)  printf_s("[ERROR] " msg "\n", ##__VA_ARGS__)

#define WLOGI(msg, ...)  wprintf_s(L"[INFO] " msg L"\n", ##__VA_ARGS__)
#define WLOGW(msg, ...)  wprintf_s(L"[WARNING] " msg L"\n", ##__VA_ARGS__)
#define WLOGE(msg, ...)  wprintf_s(L"[ERROR] " msg L"\n", ##__VA_ARGS__)

