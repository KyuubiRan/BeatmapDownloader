#pragma once

#include <imgui.h>

#include "features/Feature.h"

namespace ui::main {
bool IsShowed();
void ToggleShow();
void Update();

void AddFeature(features::Feature *feature);
}
