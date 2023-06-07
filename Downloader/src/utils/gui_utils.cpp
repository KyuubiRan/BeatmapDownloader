#include "pch.h"
#include "gui_utils.h"
#include <imgui_internal.h>

#include "misc/ResourcesLoader.hpp"

namespace GuiHelper {

void ShowTooltip(const char *s, bool marked) {
    if (marked) {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
    }
    if (!ImGui::IsItemHovered() && marked)
        return;

    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(s);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

}

namespace ImGui {

struct GroupPanelHeaderBounds {
    ImRect left;
    ImRect right;
    bool collapsed;
};

static ImVector<GroupPanelHeaderBounds> _groupPanelStack;

bool Splitter(bool split_vertically, float thickness, float *size1, float *size2, float min_size1, float min_size2,
              float splitter_long_axis_size) {
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness),
                                   0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

static bool GroupPanelIsOpen(ImGuiID id) {
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    ImGuiStorage *storage = window->DC.StateStorage;

    return storage->GetInt(id, 1) != 0;
}

static void GroupPanelSetOpen(ImGuiID id, bool open) {
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    ImGuiStorage *storage = window->DC.StateStorage;

    storage->SetInt(id, open ? 1 : 0);
}

bool BeginGroupPanel(const char *label, bool node, const ImVec2 &size) {
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;

    const ImGuiID id = window->GetID(label);
    ImGui::PushID(id);

    auto groupPanelPos = window->DC.CursorPos;
    auto itemSpacing = ImGui::GetStyle().ItemSpacing;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::BeginGroup(); // Outer group

    ImVec2 effectiveSize = size;
    if (size.x < 0.0f)
        effectiveSize.x = ImGui::GetContentRegionAvail().x;
    else
        effectiveSize.x = size.x;

    ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f)); // Adjusting group x size

    auto frameHeight = ImGui::GetFrameHeight();
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f); // Inner group spacing
    ImGui::BeginGroup(); // Inner group

    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f); // Name text spacing
    ImGui::TextUnformatted(label);

    ImRect leftRect = {ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
    ImVec2 rightMax = ImVec2(groupPanelPos.x + effectiveSize.x - frameHeight, leftRect.Max.y);
    ImRect rightRect = {{rightMax.x, leftRect.Min.x}, rightMax};
    ImGui::SameLine(0.0f, 0.0f);

    ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));

    if (node) {
        leftRect.Min.x = groupPanelPos.x;

        const ImVec2 text_size = ImGui::CalcTextSize(label);
        bool isOpen = GroupPanelIsOpen(id);

        bool hovered;
        bool toggled = ImGui::ButtonBehavior(leftRect, id, &hovered, nullptr, ImGuiButtonFlags_PressedOnClick);
        if (toggled) {
            isOpen = !isOpen;
            GroupPanelSetOpen(id, isOpen);
        }

        const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        ImGui::RenderArrow(window->DrawList, {groupPanelPos.x, groupPanelPos.y + text_size.y * 0.15f}, text_col,
                           isOpen ? ImGuiDir_Down : ImGuiDir_Right, 0.7f);

        if (!isOpen) {
            ImGui::PopStyleVar(2);
            ImGui::EndGroup();
            ImGui::EndGroup();
            ImGui::PopID();

            _groupPanelStack.push_back({leftRect, rightRect, true});
            return false;
        }
    }

    ImGui::PopStyleVar(2);

    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->Size.x -= frameHeight;

    auto itemWidth = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

    _groupPanelStack.push_back({leftRect, rightRect, false});
    return true;
}

void EndGroupPanel() {
    IM_ASSERT(_groupPanelStack.Size > 0); // Mismatched BeginGroupPanel()/EndGroupPanel() calls
    auto &info = _groupPanelStack.back();
    _groupPanelStack.pop_back();

    if (info.collapsed)
        return;

    ImGui::PopItemWidth();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    ImGui::EndGroup(); // Inner group

    auto frameHeight = ImGui::GetFrameHeight();
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

    ImGui::EndGroup(); // Outer group

    // Outer group rect
    auto itemMin = ImGui::GetItemRectMin();
    auto itemMax = ImGui::GetItemRectMax();

    ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
    ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));

    auto &leftRect = info.left;
    leftRect.Min.x -= itemSpacing.x;
    leftRect.Max.x += itemSpacing.x;

    bool hasRightPart = info.right.Min.x != info.right.Max.x;
    auto &rightRect = info.right;

    if (hasRightPart) {
        rightRect.Min.x -= itemSpacing.x;
        rightRect.Max.x += itemSpacing.x;
    }

    // Drawing rectangle
    for (int i = 0; i < (hasRightPart ? 5 : 3); ++i) {
        switch (i) {
        // left half-plane
        case 0:
            ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(leftRect.Min.x, FLT_MAX), true);
            break;
        // right half-plane
        case 1:
            ImGui::PushClipRect(ImVec2(leftRect.Max.x, -FLT_MAX), ImVec2(hasRightPart ? rightRect.Min.x : FLT_MAX, FLT_MAX), true);
            break;
        // bottom
        case 2:
            ImGui::PushClipRect(ImVec2(leftRect.Min.x, leftRect.Max.y), ImVec2(leftRect.Max.x, FLT_MAX), true);
            break;
        // bottom select
        case 3:
            ImGui::PushClipRect(ImVec2(rightRect.Min.x, rightRect.Max.y), ImVec2(rightRect.Max.x, FLT_MAX), true);
            break;
        // right hand-plane
        case 4:
            ImGui::PushClipRect(ImVec2(rightRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true);
            break;
        }

        ImGui::GetWindowDrawList()->AddRect(
            frameRect.Min, frameRect.Max,
            ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
            halfFrame.x);

        ImGui::PopClipRect();
    }

    ImGui::PopStyleVar(2);

    // Restore content region
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->Size.x += frameHeight;

    // Add vertical spacing
    ImGui::Dummy(ImVec2(0.0f, 0.0f));

    ImGui::PopID();
}

#include "misc/cpp/imgui_stdlib.h"

static ImFont *pwFont = nullptr;

bool PasswordInputText(const char *label, std::string *s, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data) {
    if (pwFont == nullptr) {
        LOGE("Cannot initlize password font!");
        return false;
    }

    PushFont(pwFont);
    const bool ret = InputText(std::format("##{}", label).c_str(), s, flags, callback, user_data);
    PopFont();
    SameLine();
    Text(label);

    return ret;
}

void SetPasswordFont(void *ttf_data, int ttf_size, float size_pixels, const ImFontConfig *font_cfg_template, const ImWchar *glyph_ranges) {
    pwFont = GetIO().Fonts->AddFontFromMemoryTTF(ttf_data, ttf_size, size_pixels, font_cfg_template, glyph_ranges);
    pwFont->FallbackChar = '*';
}

}
