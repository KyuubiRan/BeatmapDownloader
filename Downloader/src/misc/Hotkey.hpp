#pragma once

#include <set>

namespace misc {

class Hotkey {
    inline static std::set<short> s_KeyDowns{};

public:
    static bool IsDown(short k) {
        return s_KeyDowns.contains(k);
    }

    static void AddKeyDown(short k) {
        s_KeyDowns.insert(k);
    }

    static void RemoveKeyDown(short k) {
        s_KeyDowns.erase(k);
    }

    static void Clear() {
        s_KeyDowns.clear();
    }
};

}


