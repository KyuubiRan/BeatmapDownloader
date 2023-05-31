#pragma once

#include <string>
#include <wincrypt.h>

namespace osu {
class Crypto {
    static void InitPrompt(CRYPTPROTECT_PROMPTSTRUCT &ps);
    static void InitBLOB(unsigned char *data, DATA_BLOB &blob);

public:
    static std::string Decrypt(std::string_view cipherText, std::string_view entropy, std::wstring &outDescription);
};
}
