#pragma once

#include <array>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

class MD5 {
    static constexpr unsigned S[64] = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    static constexpr unsigned K[64] = {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
            0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
            0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
            0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
            0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
            0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
            0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    unsigned a0 = 0x67452301;
    unsigned b0 = 0xefcdab89;
    unsigned c0 = 0x98badcfe;
    unsigned d0 = 0x10325476;

    unsigned long long bitlen = 0;
    std::vector<unsigned char> buffer;
    bool finalized = false;

    static unsigned F(unsigned x, unsigned y, unsigned z) { return (x & y) | (~x & z); }

    static unsigned G(unsigned x, unsigned y, unsigned z) { return (x & z) | (y & ~z); }

    static unsigned H(unsigned x, unsigned y, unsigned z) { return x ^ y ^ z; }

    static unsigned I(unsigned x, unsigned y, unsigned z) { return y ^ (x | ~z); }

    static unsigned rotateLeft(unsigned x, unsigned n) { return (x << n) | (x >> (32 - n)); }

    void transform(const unsigned char block[64]) {
        unsigned a = a0, b = b0, c = c0, d = d0;
        unsigned M[16];

        for (int i = 0; i < 16; i++) {
            M[i] = (block[i * 4] & 0xFF) |
                   (block[i * 4 + 1] & 0xFF) << 8 |
                   (block[i * 4 + 2] & 0xFF) << 16 |
                   (block[i * 4 + 3] & 0xFF) << 24;
        }

        for (unsigned i = 0; i < 64; i++) {
            unsigned f, g;
            if (i < 16) {
                f = F(b, c, d);
                g = i;
            } else if (i < 32) {
                f = G(b, c, d);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = H(b, c, d);
                g = (3 * i + 5) % 16;
            } else {
                f = I(b, c, d);
                g = (7 * i) % 16;
            }

            unsigned temp = d;
            d = c;
            c = b;
            b = b + rotateLeft((a + f + K[i] + M[g]), S[i]);
            a = temp;
        }

        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }

public:
    void update(const std::string &s) {
        const auto *input = reinterpret_cast<const unsigned char *>(s.c_str());
        unsigned len = s.length();
        unsigned i = 0, idx = bitlen / 8 % 64;
        bitlen += len << 3;

        if (idx) {
            unsigned fill = 64 - idx;
            if (fill > len) fill = len;
            for (unsigned j = 0; j < fill; j++)
                buffer.push_back(input[j]);
            i += fill;
            if (idx + fill < 64)
                return;
            transform(&buffer[0]);
            buffer.clear();
        }

        for (; i + 64 <= len; i += 64)
            transform(input + i);

        if (i < len)
            for (unsigned j = 0; j < len - i; j++)
                buffer.push_back(input[i + j]);
    }

    void finalize() {
        if (!finalized) {
            buffer.push_back(0x80);
            unsigned idx = buffer.size();
            if (idx > 56) {
                buffer.resize(64, 0);
                transform(&buffer[0]);
                buffer.clear();
                idx = 0;
            }

            buffer.resize(56, 0);
            for (int i = 0; i < 8; i++) buffer.push_back((bitlen >> (i * 8)) & 0xFF);
            transform(&buffer[0]);

            finalized = true;
        }
    }

    std::string str() {
        finalize();
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(2) << ((a0 >> 0) & 0xFF) << std::setw(2) << ((a0 >> 8) & 0xFF)
           << std::setw(2) << ((a0 >> 16) & 0xFF) << std::setw(2) << ((a0 >> 24) & 0xFF);
        ss << std::setw(2) << ((b0 >> 0) & 0xFF) << std::setw(2) << ((b0 >> 8) & 0xFF)
           << std::setw(2) << ((b0 >> 16) & 0xFF) << std::setw(2) << ((b0 >> 24) & 0xFF);
        ss << std::setw(2) << ((c0 >> 0) & 0xFF) << std::setw(2) << ((c0 >> 8) & 0xFF)
           << std::setw(2) << ((c0 >> 16) & 0xFF) << std::setw(2) << ((c0 >> 24) & 0xFF);
        ss << std::setw(2) << ((d0 >> 0) & 0xFF) << std::setw(2) << ((d0 >> 8) & 0xFF)
           << std::setw(2) << ((d0 >> 16) & 0xFF) << std::setw(2) << ((d0 >> 24) & 0xFF);
        return ss.str();
    }

    std::array<unsigned char, 16> digest() {
        finalize();
        std::array<unsigned char, 16> out{};
        for (int i = 0; i < 4; i++) {
            out[i] = (a0 >> (i * 8)) & 0xFF;
            out[i + 4] = (b0 >> (i * 8)) & 0xFF;
            out[i + 8] = (c0 >> (i * 8)) & 0xFF;
            out[i + 12] = (d0 >> (i * 8)) & 0xFF;
        }
        return out;
    }
};
