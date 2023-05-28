#pragma once

#include "ui/Feature.h"

namespace hook {

using namespace ui::main;

class HandleLinkHook : public Feature {

    static HandleLinkHook &GetInstance() {
        static HandleLinkHook instance;
        return instance;
    }

    void DrawMain() override;
    virtual FeatureInfo& GetInfo() override;

private:
    HandleLinkHook();

};

}