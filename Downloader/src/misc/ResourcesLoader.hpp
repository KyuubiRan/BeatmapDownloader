#pragma once

#include <Windows.h>
#include "utils/Utils.h"

namespace res {

static DWORD LoadEx(LPCWSTR name, LPWSTR type, BYTE **pDest) {
    if (!utils::GetMyModuleHandle())
        return 0;
    
    if (!name)
        return 0;

    if (!type)
        return 0;

    const HRSRC hResource = FindResourceW(utils::GetMyModuleHandle(), name, type);
    if (!hResource)
        return 0;

    const HGLOBAL hGlob = LoadResource(utils::GetMyModuleHandle(), hResource);
    if (!hGlob)
        return 0;

    const DWORD size = SizeofResource(utils::GetMyModuleHandle(), hResource);
    *pDest = static_cast<BYTE *>(LockResource(hGlob));
    if (size > 0 && *pDest)
        return size;

    return 0;
}

static DWORD LoadEx(int id, LPWSTR type, BYTE **pDest) {
    if (!type)
        return 0;

    return LoadEx(MAKEINTRESOURCEW(id), type, pDest);
}

}
