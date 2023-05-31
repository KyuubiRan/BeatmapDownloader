#pragma once

#include "config/Field.h"
#include "ui/Feature.h"

namespace features {

using namespace ui::main;

class HandleLinkHook : public Feature {
public:
    config::Field<bool> f_Enabled;
    config::Field<std::string> f_Domain;

    static HandleLinkHook &GetInstance() {
        static HandleLinkHook instance;
        return instance;
    }

    void drawMain() override;
    virtual FeatureInfo& getInfo() override;

private:
    HandleLinkHook();

};

}