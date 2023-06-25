#pragma once

#include <algorithm>
#include <set>
#include <unordered_set>

#include "ISerializable.h"

namespace misc {

class Hotkey : public ISerializable {
    inline static std::set<short> s_KeyDowns{};
    inline static std::mutex s_Mutex{};
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
        std::lock_guard _g(s_Mutex);
        s_KeyDowns.insert(k);
        for (const auto htk : s_Hotkeys) {
            htk->notifyKeyDownChanged();
        }
    }

    static void RemoveKeyDown(short k) {
        std::lock_guard _g(s_Mutex);
        s_KeyDowns.erase(k);
    }

    static void ClearPressedKey() {
        std::lock_guard _g(s_Mutex);
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
};

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
