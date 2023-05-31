#pragma once
#include <string>

namespace utils {
class Base64 {
    inline static constexpr std::string s_Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static inline bool IsBase64(unsigned char c);

public:
    static std::string Encode(const std::string &in);

    static std::string Decode(const std::string &in);
};
}
