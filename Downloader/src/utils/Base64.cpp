#include "Base64.h"

#include <vector>

namespace utils {

inline bool Base64::IsBase64(const unsigned char c) {
    return isalnum(c) || c == '+' || c == '/';
}

std::string Base64::Encode(const std::string &in) {
    std::string out;

    int val = 0, valb = -6;
    for (const unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(s_Base64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(s_Base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }

    return out;
}

std::string Base64::Decode(const std::string &in) {
    std::string out;

    std::vector T(256, -1);
    for (int i = 0; i < 64; i++) {
        T[s_Base64Chars[i]] = i;
    }

    int val = 0, valb = -8;
    for (const unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

}
