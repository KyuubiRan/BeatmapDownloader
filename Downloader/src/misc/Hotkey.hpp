#pragma once

#include <algorithm>
#include <set>
#include <unordered_set>
#include "ISerializable.h"

namespace misc {
const char *GetKeyName(short key);

class Hotkey : public ISerializable {
    inline static std::set<short> s_KeyDowns{};
    inline static std::set<Hotkey *> s_Hotkeys{};

    using KeyDownEventFunc = void (*)();

    std::unordered_set<short> m_Keys{};
    std::vector<KeyDownEventFunc> m_KeyDownEvents{};

    void notifyKeyDownChanged() {
        if (!isDown())
            return;

        for (const auto &f : m_KeyDownEvents)
            f();
    }

public:
    static bool IsDown(short k) {
        return s_KeyDowns.contains(k);
    }

    static void AddKeyDown(short k) {
        if (!s_KeyDowns.insert(k).second) {
            return;
        }
        for (const auto htk : s_Hotkeys) {
            htk->notifyKeyDownChanged();
        }
    }

    static void RemoveKeyDown(short k) {
        s_KeyDowns.erase(k);
    }

    static void ClearPressedKey() {
        s_KeyDowns.clear();
    }

    Hotkey(const Hotkey &htk) {
        this->m_Keys = htk.m_Keys;
    }

    Hotkey(Hotkey &&htk) noexcept {
        this->m_Keys = htk.m_Keys;
    }

    Hotkey &operator=(const Hotkey &htk) {
        this->m_Keys = htk.m_Keys;
        return *this;
    }

    static Hotkey GetPressedHotkey() {
        Hotkey htk;
        htk.m_Keys.insert(s_KeyDowns.begin(), s_KeyDowns.end());
        return htk;
    }

    Hotkey() {
        s_Hotkeys.insert(this);
    }

    ~Hotkey() override {
        s_Hotkeys.erase(this);
    }

    explicit Hotkey(const short key) :
        Hotkey() {
        m_Keys.insert(key);
    }

    Hotkey(const std::initializer_list<short> keys) :
        Hotkey() {
        this->m_Keys.insert(keys);
    }

    bool isDown() {
        if (m_Keys.empty())
            return false;

        return std::ranges::all_of(m_Keys, [](const short k) { return IsDown(k); });
    }

    void addOnKeyDownEvent(const KeyDownEventFunc f) {
        m_KeyDownEvents.push_back(f);
    }

    Hotkey &operator+=(const KeyDownEventFunc f) {
        addOnKeyDownEvent(f);
        return *this;
    }

    void to_json(nlohmann::json &j) const override {
        j["Keys"] = m_Keys;
    }

    void from_json(const nlohmann::json &j) override {
        m_Keys = j["Keys"].get<std::unordered_set<short>>();
    }

    std::unordered_set<short> const &getKeys() {
        return m_Keys;
    }

    bool operator==(const Hotkey &rhs) const {
        return this->m_Keys == rhs.m_Keys;
    }

    bool operator-(const Hotkey &rhs) const {
        return std::ranges::any_of(m_Keys, [&rhs](const short k) { return !rhs.m_Keys.contains(k); });
    }

    bool isEmpty() const {
        return m_Keys.empty();
    }

    std::string toString() const {
        if (m_Keys.empty())
            return "";

        std::string str;
        const auto first = m_Keys.begin();
        str += GetKeyName(*first);
        for (auto it = std::next(first); it != m_Keys.end(); ++it) {
            str += " + ";
            str += GetKeyName(*it);
        }

        return str;
    }
};

#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

static ImGuiKey LegacyToInput(short key) {
    switch (key) {
    case VK_TAB:
        return ImGuiKey_Tab;
    case VK_LEFT:
        return ImGuiKey_LeftArrow;
    case VK_RIGHT:
        return ImGuiKey_RightArrow;
    case VK_UP:
        return ImGuiKey_UpArrow;
    case VK_DOWN:
        return ImGuiKey_DownArrow;
    case VK_PRIOR:
        return ImGuiKey_PageUp;
    case VK_NEXT:
        return ImGuiKey_PageDown;
    case VK_HOME:
        return ImGuiKey_Home;
    case VK_END:
        return ImGuiKey_End;
    case VK_INSERT:
        return ImGuiKey_Insert;
    case VK_DELETE:
        return ImGuiKey_Delete;
    case VK_BACK:
        return ImGuiKey_Backspace;
    case VK_SPACE:
        return ImGuiKey_Space;
    case VK_RETURN:
        return ImGuiKey_Enter;
    case VK_ESCAPE:
        return ImGuiKey_Escape;
    case VK_OEM_7:
        return ImGuiKey_Apostrophe;
    case VK_OEM_COMMA:
        return ImGuiKey_Comma;
    case VK_OEM_MINUS:
        return ImGuiKey_Minus;
    case VK_OEM_PERIOD:
        return ImGuiKey_Period;
    case VK_OEM_2:
        return ImGuiKey_Slash;
    case VK_OEM_1:
        return ImGuiKey_Semicolon;
    case VK_OEM_PLUS:
        return ImGuiKey_Equal;
    case VK_OEM_4:
        return ImGuiKey_LeftBracket;
    case VK_OEM_5:
        return ImGuiKey_Backslash;
    case VK_OEM_6:
        return ImGuiKey_RightBracket;
    case VK_OEM_3:
        return ImGuiKey_GraveAccent;
    case VK_CAPITAL:
        return ImGuiKey_CapsLock;
    case VK_SCROLL:
        return ImGuiKey_ScrollLock;
    case VK_NUMLOCK:
        return ImGuiKey_NumLock;
    case VK_SNAPSHOT:
        return ImGuiKey_PrintScreen;
    case VK_PAUSE:
        return ImGuiKey_Pause;
    case VK_NUMPAD0:
        return ImGuiKey_Keypad0;
    case VK_NUMPAD1:
        return ImGuiKey_Keypad1;
    case VK_NUMPAD2:
        return ImGuiKey_Keypad2;
    case VK_NUMPAD3:
        return ImGuiKey_Keypad3;
    case VK_NUMPAD4:
        return ImGuiKey_Keypad4;
    case VK_NUMPAD5:
        return ImGuiKey_Keypad5;
    case VK_NUMPAD6:
        return ImGuiKey_Keypad6;
    case VK_NUMPAD7:
        return ImGuiKey_Keypad7;
    case VK_NUMPAD8:
        return ImGuiKey_Keypad8;
    case VK_NUMPAD9:
        return ImGuiKey_Keypad9;
    case VK_DECIMAL:
        return ImGuiKey_KeypadDecimal;
    case VK_DIVIDE:
        return ImGuiKey_KeypadDivide;
    case VK_MULTIPLY:
        return ImGuiKey_KeypadMultiply;
    case VK_SUBTRACT:
        return ImGuiKey_KeypadSubtract;
    case VK_ADD:
        return ImGuiKey_KeypadAdd;
    case IM_VK_KEYPAD_ENTER:
        return ImGuiKey_KeypadEnter;
    case VK_LSHIFT:
        return ImGuiKey_LeftShift;
    case VK_LCONTROL:
        return ImGuiKey_LeftCtrl;
    case VK_LMENU:
        return ImGuiKey_LeftAlt;
    case VK_LWIN:
        return ImGuiKey_LeftSuper;
    case VK_RSHIFT:
        return ImGuiKey_RightShift;
    case VK_RCONTROL:
        return ImGuiKey_RightCtrl;
    case VK_RMENU:
        return ImGuiKey_RightAlt;
    case VK_RWIN:
        return ImGuiKey_RightSuper;
    case VK_APPS:
        return ImGuiKey_Menu;
    case '0':
        return ImGuiKey_0;
    case '1':
        return ImGuiKey_1;
    case '2':
        return ImGuiKey_2;
    case '3':
        return ImGuiKey_3;
    case '4':
        return ImGuiKey_4;
    case '5':
        return ImGuiKey_5;
    case '6':
        return ImGuiKey_6;
    case '7':
        return ImGuiKey_7;
    case '8':
        return ImGuiKey_8;
    case '9':
        return ImGuiKey_9;
    case 'A':
        return ImGuiKey_A;
    case 'B':
        return ImGuiKey_B;
    case 'C':
        return ImGuiKey_C;
    case 'D':
        return ImGuiKey_D;
    case 'E':
        return ImGuiKey_E;
    case 'F':
        return ImGuiKey_F;
    case 'G':
        return ImGuiKey_G;
    case 'H':
        return ImGuiKey_H;
    case 'I':
        return ImGuiKey_I;
    case 'J':
        return ImGuiKey_J;
    case 'K':
        return ImGuiKey_K;
    case 'L':
        return ImGuiKey_L;
    case 'M':
        return ImGuiKey_M;
    case 'N':
        return ImGuiKey_N;
    case 'O':
        return ImGuiKey_O;
    case 'P':
        return ImGuiKey_P;
    case 'Q':
        return ImGuiKey_Q;
    case 'R':
        return ImGuiKey_R;
    case 'S':
        return ImGuiKey_S;
    case 'T':
        return ImGuiKey_T;
    case 'U':
        return ImGuiKey_U;
    case 'V':
        return ImGuiKey_V;
    case 'W':
        return ImGuiKey_W;
    case 'X':
        return ImGuiKey_X;
    case 'Y':
        return ImGuiKey_Y;
    case 'Z':
        return ImGuiKey_Z;
    case VK_F1:
        return ImGuiKey_F1;
    case VK_F2:
        return ImGuiKey_F2;
    case VK_F3:
        return ImGuiKey_F3;
    case VK_F4:
        return ImGuiKey_F4;
    case VK_F5:
        return ImGuiKey_F5;
    case VK_F6:
        return ImGuiKey_F6;
    case VK_F7:
        return ImGuiKey_F7;
    case VK_F8:
        return ImGuiKey_F8;
    case VK_F9:
        return ImGuiKey_F9;
    case VK_F10:
        return ImGuiKey_F10;
    case VK_F11:
        return ImGuiKey_F11;
    case VK_F12:
        return ImGuiKey_F12;
    case VK_LBUTTON:
        return ImGuiKey_MouseLeft;
    case VK_RBUTTON:
        return ImGuiKey_MouseRight;
    case VK_MBUTTON:
        return ImGuiKey_MouseMiddle;
    case VK_XBUTTON1:
        return ImGuiKey_MouseX1;
    case VK_XBUTTON2:
        return ImGuiKey_MouseX2;
    case VK_SHIFT:
        return ImGuiKey_ModShift;
    case VK_CONTROL:
        return ImGuiKey_ModCtrl;
    case VK_MENU:
        return ImGuiKey_ModAlt;
    case 0xC1:
        return ImGuiKey_MouseWheelX;
    case 0xC2:
        return ImGuiKey_MouseWheelY;
    default:
        return ImGuiKey_None;
    }
}

static short InputToLegacy(ImGuiKey inputkey) {
    auto &io = ImGui::GetIO();
    auto key = io.KeyMap[inputkey];
    if (key == -1) {
        switch (inputkey) {
        case ImGuiKey_MouseLeft:
            key = VK_LBUTTON;
            break;
        case ImGuiKey_MouseRight:
            key = VK_RBUTTON;
            break;
        case ImGuiKey_MouseMiddle:
            key = VK_MBUTTON;
            break;
        case ImGuiKey_MouseX1:
            key = VK_XBUTTON1;
            break;
        case ImGuiKey_MouseX2:
            key = VK_XBUTTON2;
            break;
        case ImGuiKey_MouseWheelX:
            key = 0xC1;
            break;
        case ImGuiKey_MouseWheelY:
            key = 0xC2;
            break;
        default:
            LOGE("Failed to find legacy input for %d", inputkey);
            break;
        }
    }
    return key;
}

inline const char *GetKeyName(short key) {
    const auto imkey = LegacyToInput(key);
    return ImGui::GetKeyName(imkey);
}

}

template <>
struct nlohmann::adl_serializer<misc::Hotkey> {
    static void to_json(json &j, const misc::Hotkey &htk) {
        htk.to_json(j);
    }

    static void from_json(const json &j, misc::Hotkey &htk) {
        htk.from_json(j);
    }
};
