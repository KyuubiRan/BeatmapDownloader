#pragma once
#include "Feature.h"

namespace ui::misc {

class About : public main::Feature {
    About();
    
public:
    static About &GetInstance() {
        static About instance;
        return instance;
    }
    
    main::FeatureInfo &getInfo() override;

    void drawMain() override;
    
};
}

