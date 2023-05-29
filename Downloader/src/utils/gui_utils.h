#pragma once

#include <imgui.h>

namespace GuiHelper
{

    void ShowTooltip(const char *s, bool marked = true);

}

namespace ImGui
{

    bool Splitter(bool split_vertically, float thickness, float *size1, float *size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

    bool BeginGroupPanel(const char *label, bool node = false, const ImVec2 &size = ImVec2(-1.0f, 0.0f));
    void EndGroupPanel();

}