#pragma once

#include <imgui.h>

#include "Feature.h"

namespace ui::main {
bool IsShowed();
void ToggleShow();
void Update();

void AddFeature(Feature *feature);
}
