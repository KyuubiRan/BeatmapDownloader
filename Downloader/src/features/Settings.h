#pragma once
#include "Feature.h"
#include "config/Field.h"
#include "config/I18nManager.h"
#include "misc/Hotkey.hpp"

namespace features {

class Settings : public Feature {
    Settings();

public:
    config::Field<bool> f_EnableToast;
    config::Field<int> f_ToastDuration;
    config::Field<bool> f_EnableConsole;
    config::Field<std::string> f_OsuPath;
    config::Field<int> f_Theme;

    static Settings &GetInstance() {
        static Settings instance;
        return instance;
    }

    void applyTheme();
    
    void drawMain() override;
    virtual FeatureInfo &getInfo() override;
};

}
