#include "pch.h"
#include "gui_utils.h"
#include <imgui_internal.h>
#include <shellapi.h>

#include "misc/Color.h"
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

bool PasswordInputText(const char *label, std::string *s, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data) {
    flags |= ImGuiInputTextFlags_Password;
    return InputText(label, s, flags, callback, user_data);
}

void TextUrl(const char *url) {
    TextColored(color::DodgerBlue, url);
    if (IsItemHovered()) {
        if (IsMouseClicked(ImGuiMouseButton_Left)) {
            ShellExecuteW(nullptr, L"open", utils::s2ws(url).c_str(), nullptr, nullptr, SW_HIDE);
        }
    }
}

// Modified version of: https://github.com/spirthack/CSGOSimple/blob/master/CSGOSimple/UI.cpp#L287
bool HotkeyWidget(const char *label, misc::Hotkey &hotkey, const ImVec2 &size) {
    // Init ImGui
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    ImGuiIO &io = g.IO;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    const ImVec2 item_size = CalcItemSize(size, CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + item_size);
    const ImRect total_bb(window->DC.CursorPos,
                          frame_bb.Max +
                          ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;


    const bool focus_requested =
        (GetItemStatusFlags() & ImGuiItemStatusFlags_FocusedByTabbing) != 0 || g.NavActivateId == id;
    const bool hovered = ItemHoverable(frame_bb, id);
    if (hovered) {
        SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_TextInput;
    }

    static misc::Hotkey _initHotkey;
    static misc::Hotkey _currHotkey;
    static misc::Hotkey _prevHotkey;

    const bool user_clicked = hovered && io.MouseClicked[0];
    if (focus_requested || user_clicked) {
        if (g.ActiveId != id) {
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            memset(io.KeysDown, 0, sizeof(io.KeysDown));

            _initHotkey = hotkey;
            _currHotkey = misc::Hotkey();
            _prevHotkey = misc::Hotkey();
        }

        SetActiveID(id, window);
        FocusWindow(window);
    } else if (io.MouseClicked[0] && g.ActiveId == id) {
        ClearActiveID();
    }

    bool valueChanged = false;

    if (g.ActiveId == id) {
        if (IsKeyPressed(ImGuiKey_Escape)) {
            ClearActiveID();
            if (hotkey != _initHotkey) {
                hotkey = _initHotkey;
                valueChanged = true;
            }
        } else {
            NavMoveRequestCancel();

            const auto newHotkey = misc::Hotkey::GetPressedHotkey();

            if (newHotkey.isEmpty() && !_currHotkey.isEmpty()) {
                ClearActiveID();
                valueChanged = false;
            } else if (newHotkey - _prevHotkey) {
                _currHotkey = newHotkey;
                hotkey = newHotkey;
                valueChanged = true;
            }

            _prevHotkey = newHotkey;
        }
    }

    // Render
    // Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is Set 'buf' might still be the
    // old value. We Set buf to NULL to prevent accidental usage from now on.

    char buf_display[128] = "Empty";

    const ImU32 frame_col = GetColorU32(g.ActiveId == id
        ? ImGuiCol_FrameBgActive
        : hovered
        ? ImGuiCol_FrameBgHovered
        : ImGuiCol_FrameBg);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

    if ((g.ActiveId == id && !_currHotkey.isEmpty()) || g.ActiveId != id)
        strcpy_s(buf_display, hotkey.toString().c_str());
    else if (g.ActiveId == id)
        strcpy_s(buf_display, "<Press a key>");

    const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + item_size.x,
                           frame_bb.Min.y + item_size.y); // Not using frame_bb.Max because we have adjusted size
    ImVec2 render_pos = frame_bb.Min + style.FramePadding;
    RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, nullptr,
                      nullptr, style.ButtonTextAlign, &clip_rect);
    // RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL,
    // GetColorU32(ImGuiCol_Text), style.ButtonTextAlign, &clip_rect); draw_window->DrawList->AddText(g.Font,
    // g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, NULL, 0.0f, &clip_rect);
    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    return valueChanged;
}

constexpr ImVec4 operator""_rgba(const char *str, std::size_t len) {
    uint32_t hex = 0;
    for (std::size_t i = 0; i < len; i++) {
        hex *= 16;
        if (str[i] >= '0' && str[i] <= '9')
            hex += str[i] - '0';
        else if (str[i] >= 'a' && str[i] <= 'f')
            hex += str[i] - 'a' + 10;
        else if (str[i] >= 'A' && str[i] <= 'F')
            hex += str[i] - 'A' + 10;
        else
            throw std::invalid_argument("Invalid hexadecimal digit");
    }
    return {static_cast<float>((hex >> 24) & 0xFF) / 255.0f, static_cast<float>((hex >> 16) & 0xFF) / 255.0f,
            static_cast<float>((hex >> 8) & 0xFF) / 255.0f, static_cast<float>((hex) & 0xFF) / 255.0f};
}

void StyleSakura(ImGuiStyle *dst = nullptr) {
    ImGuiStyle *style = dst ? dst : &ImGui::GetStyle();
    ImVec4 *colors = style->Colors;

    style->Alpha = 1.0f;
    style->DisabledAlpha = 0.60f;
    style->WindowPadding = ImVec2(8, 8);
    style->WindowRounding = 8.0f;
    style->WindowBorderSize = 0.0f;
    style->WindowMinSize = ImVec2(32, 32);
    style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style->WindowMenuButtonPosition = ImGuiDir_Left;
    style->ChildRounding = 0.0f;
    style->ChildBorderSize = 0.0f;
    style->PopupRounding = 6.0f;
    style->PopupBorderSize = 1.0f;
    style->FramePadding = ImVec2(4.0, 3.0);
    style->FrameRounding = 6.0f;
    style->FrameBorderSize = 0.0f;
    style->ItemSpacing = ImVec2(8, 4);
    style->ItemInnerSpacing = ImVec2(4, 4);
    style->CellPadding = ImVec2(4, 2);
    style->TouchExtraPadding = ImVec2(0, 0);
    style->IndentSpacing = 21.0f;
    style->ColumnsMinSpacing = 6.0f;
    style->ScrollbarSize = 14.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 12.0f;
    style->GrabRounding = 6.0f;
    style->LogSliderDeadzone = 4.0f;
    style->TabRounding = 6.0f;
    style->TabBorderSize = 0.0f;
    style->TabMinWidthForCloseButton = 0.0f;
    style->ColorButtonPosition = ImGuiDir_Right;
    style->ButtonTextAlign = ImVec2(0, 0.5);
    style->SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style->SeparatorTextBorderSize = 3.0f;
    style->SeparatorTextAlign = ImVec2(0.0f, 0.5f);
    style->SeparatorTextPadding = ImVec2(20.0f, 3.f);
    style->DisplayWindowPadding = ImVec2(19, 19);
    style->DisplaySafeAreaPadding = ImVec2(3, 3);
    style->MouseCursorScale = 1.0f;
    style->AntiAliasedLines = true;
    style->AntiAliasedLinesUseTex = true;
    style->AntiAliasedFill = true;
    style->CurveTessellationTol = 1.25f;
    style->CircleTessellationMaxError = 0.30f;

    colors[ImGuiCol_Border] = "301E2DFF"_rgba;
    colors[ImGuiCol_BorderShadow] = "301E2D30"_rgba;
    colors[ImGuiCol_Button] = "D56A87FF"_rgba;
    colors[ImGuiCol_ButtonActive] = "FF9DBEFF"_rgba;
    colors[ImGuiCol_ButtonHovered] = "F96E9EFF"_rgba;
    colors[ImGuiCol_CheckMark] = "301E2DFF"_rgba;
    colors[ImGuiCol_ChildBg] = "453743FF"_rgba;
    // colors[ImGuiCol_DockingEmptyBg] = "301E2DFF"_rgba;
    // colors[ImGuiCol_DockingPreview] = "FF9DBEB3"_rgba;
    colors[ImGuiCol_DragDropTarget] = "FFCDCCE6"_rgba;
    colors[ImGuiCol_FrameBg] = "31293660"_rgba;
    colors[ImGuiCol_FrameBgActive] = "6B284D90"_rgba;
    colors[ImGuiCol_FrameBgHovered] = "882650FF"_rgba;
    colors[ImGuiCol_Header] = "D56A84FF"_rgba;
    colors[ImGuiCol_HeaderActive] = "FF9DBEDD"_rgba;
    colors[ImGuiCol_HeaderHovered] = "F96E9EFF"_rgba;
    colors[ImGuiCol_MenuBarBg] = "312936FF"_rgba;
    colors[ImGuiCol_ModalWindowDimBg] = "3333335A"_rgba;
    colors[ImGuiCol_NavHighlight] = "FFFFFF0A"_rgba;
    colors[ImGuiCol_NavWindowingDimBg] = "CCCCCC33"_rgba;
    colors[ImGuiCol_NavWindowingHighlight] = "FFFFFFB3"_rgba;
    colors[ImGuiCol_PlotHistogram] = "FFCDCCFF"_rgba;
    colors[ImGuiCol_PlotHistogramHovered] = "D56A87FF"_rgba;
    colors[ImGuiCol_PlotLines] = "FFCDCCFF"_rgba;
    colors[ImGuiCol_PlotLinesHovered] = "D56A87FF"_rgba;
    colors[ImGuiCol_PopupBg] = "453743FF"_rgba;
    colors[ImGuiCol_ResizeGrip] = "FFCDCCFF"_rgba;
    colors[ImGuiCol_ResizeGripActive] = "D56A87FF"_rgba;
    colors[ImGuiCol_ResizeGripHovered] = "FF9DBEFF"_rgba;
    colors[ImGuiCol_ScrollbarBg] = "301E2D00"_rgba;
    colors[ImGuiCol_ScrollbarGrab] = "6B284DFF"_rgba;
    colors[ImGuiCol_ScrollbarGrabActive] = "FF9DBEFF"_rgba;
    colors[ImGuiCol_ScrollbarGrabHovered] = "D56A87FF"_rgba;
    colors[ImGuiCol_Separator] = "FFCDCCFF"_rgba;
    colors[ImGuiCol_SeparatorActive] = "D56A87FF"_rgba;
    colors[ImGuiCol_SeparatorHovered] = "FF9DBEFF"_rgba;
    colors[ImGuiCol_SliderGrab] = "D56A87FF"_rgba;
    colors[ImGuiCol_SliderGrabActive] = "FFCDCCFF"_rgba;
    colors[ImGuiCol_Tab] = "D56A87FF"_rgba;
    colors[ImGuiCol_TabActive] = "FF9DBECC"_rgba;
    colors[ImGuiCol_TabHovered] = "F96E9EFF"_rgba;
    colors[ImGuiCol_TabUnfocused] = "D56A87F8"_rgba;
    colors[ImGuiCol_TabUnfocusedActive] = "D56A87FF"_rgba;
    colors[ImGuiCol_TableBorderLight] = "3517276E"_rgba;
    colors[ImGuiCol_TableBorderStrong] = "301E2D30"_rgba;
    colors[ImGuiCol_TableHeaderBg] = "D56A87FF"_rgba;
    colors[ImGuiCol_TableRowBg] = "453743F8"_rgba;
    colors[ImGuiCol_TableRowBgAlt] = "453743EC"_rgba;
    colors[ImGuiCol_Text] = "FFFFFFFF"_rgba;
    colors[ImGuiCol_TextDisabled] = "909090FF"_rgba;
    colors[ImGuiCol_TextSelectedBg] = "FF9DBE33"_rgba;
    colors[ImGuiCol_TitleBg] = "453743FF"_rgba;
    colors[ImGuiCol_TitleBgActive] = "D56A87FF"_rgba;
    colors[ImGuiCol_TitleBgCollapsed] = "D56A87FF"_rgba;
    colors[ImGuiCol_WindowBg] = "301E2DFF"_rgba;
}

void SetTheme(const std::string_view theme) {
    if (theme == "ImGuiDark")
        StyleColorsDark();
    else if (theme == "ImGuiLight")
        StyleColorsLight();
    else if (theme == "ImGuiClassic")
        StyleColorsClassic();
    else
        StyleSakura();
}
}
