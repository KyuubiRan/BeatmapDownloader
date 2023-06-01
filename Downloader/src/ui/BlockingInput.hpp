#pragma once

namespace ui {
class InputBlock {
    inline static int block = 0;

public:
    InputBlock() = delete;
    
    static void Push() {
        ++block;
    }

    static void Pop() {
        if (--block < 0)
            block = 0;
    }

    static bool IsBlocked() {
        return block;
    }
};
}
