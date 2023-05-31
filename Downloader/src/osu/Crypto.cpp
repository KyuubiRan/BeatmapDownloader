#include "pch.h"
#include "Crypto.h"

#pragma comment(lib, "crypt32.lib")

void osu::Crypto::InitPrompt(CRYPTPROTECT_PROMPTSTRUCT &ps) {
    ps.cbSize = sizeof CRYPTPROTECT_PROMPTSTRUCT;
    ps.dwPromptFlags = 0;
    ps.hwndApp = nullptr;
    ps.szPrompt = nullptr;
}

void osu::Crypto::InitBLOB(unsigned char *data, DATA_BLOB &blob) {
    if (data == nullptr) {
        LOGE("Invalid data pointer");
        return;
    }

    blob.pbData = data;
    blob.cbData = static_cast<DWORD>(strlen(reinterpret_cast<const char*>(data)));
}

std::string osu::Crypto::Decrypt(std::string_view cipherText, std::string_view entropy, std::wstring &outDescription) {
    DATA_BLOB plainTextBlob{};
    DATA_BLOB cipherTextBlob{};
    DATA_BLOB entropyBlob{};

    CRYPTPROTECT_PROMPTSTRUCT prompt{};
    InitPrompt(prompt);


    InitBLOB((unsigned char*)cipherText.data(), cipherTextBlob);
    InitBLOB((unsigned char*)entropy.data(), entropyBlob);

    int flag = CRYPTPROTECT_UI_FORBIDDEN;

    LPWSTR description = nullptr;
    bool success = CryptUnprotectData(&cipherTextBlob, &description, &entropyBlob, nullptr, &prompt, flag, &plainTextBlob);
    if (success) {
        outDescription = description;

        std::string ret = {reinterpret_cast<const char*>(plainTextBlob.pbData), plainTextBlob.cbData};
        LocalFree(description);
        LocalFree(plainTextBlob.pbData);
        LocalFree(cipherTextBlob.pbData);
        LocalFree(entropyBlob.pbData);

        return ret;
    } else {
        LOGE("Cannot decrypt data, error code: %d", GetLastError());
        return "";
    }
}
