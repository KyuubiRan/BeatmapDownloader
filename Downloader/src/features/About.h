#pragma once
#include "Feature.h"

namespace features {

class About : public Feature {
    About();
    
public:
    static About &GetInstance() {
        static About instance;
        return instance;
    }

    FeatureInfo &getInfo() override;

    void drawMain() override;
    
};
}

