#pragma once

#include <shellapi.h>
#include "config/Field.h"
#include "Feature.h"

namespace features {

class HandleLinkHook : public Feature {
public:
    config::Field<bool> f_Enabled;
    config::Field<std::string> f_Domain;

    static BOOL __stdcall ShellExecuteExW_Hook(SHELLEXECUTEINFOW *pExecInfo);

    static HandleLinkHook &GetInstance() {
        static HandleLinkHook instance;
        return instance;
    }

    void drawMain() override;
    virtual FeatureInfo &getInfo() override;

private:
    HandleLinkHook();

};

}
