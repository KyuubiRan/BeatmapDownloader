#pragma once

#include <string>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace ui::main {

struct FeatureInfo {
    std::string_view category;
    std::string_view groupName;
};

class Feature {
public:
    Feature() = default;
    virtual ~Feature() = default;

    virtual void drawMain() = 0;
    virtual FeatureInfo& getInfo() = 0;
};
}
