#pragma once
#include "ui/Feature.h"


namespace features {

class MultiDownload : public ui::main::Feature {
    MultiDownload();

public:
    static MultiDownload &GetInstance() {
        static MultiDownload instance;
        return instance;
    }

    ui::main::FeatureInfo &getInfo() override;
    void drawMain() override;
};

}
