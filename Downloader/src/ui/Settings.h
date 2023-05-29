﻿#pragma once
#include "Feature.h"
#include "config/Field.h"
#include "config/I18nManager.h"

namespace ui::misc {

class Settings : public main::Feature{
    Settings();
public:
    static Settings &GetInstance() {
        static Settings instance;
        return instance;
    }

    void drawMain() override;
    virtual main::FeatureInfo& getInfo() override;
    
};

}


