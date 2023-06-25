#pragma once
#include "Feature.h"
#include "config/Field.h"
#include "misc/Hotkey.hpp"

namespace features {

class CustomHotkey : public Feature {
    CustomHotkey();

public:
    config::Field<misc::Hotkey> f_MainMenuHotkey;
    config::Field<misc::Hotkey> f_SearchHotkey;

    static CustomHotkey &GetInstance() {
        static CustomHotkey inst;
        return inst;
    }

    void drawMain() override;
    FeatureInfo &getInfo() override;
};


}
