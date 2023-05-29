#pragma once

#include "ISerializable.h"

namespace config {

void Register(ISerializable *cfg);
void Load(ISerializable *cfg);
void Init();
void Save();

template <typename T>
class Field : ISerializable {
    std::string_view name;
    T value;
    T defaultValue;

public:

    Field(std::string_view name, T defaultValue = T{}) : name(name), defaultValue(defaultValue) {
        Register(this);
        Load(this);
    }

    std::string_view getName() const {
        return name;
    }

    T &getValue() {
        return value;
    }

    void setValue(T _value) {
        this->value = _value;
    }

    operator T &() {
        return value;
    }

    Field &operator=(T _value) {
        setValue(_value);
        return *this;
    }

    T *getPtr() {
        return &value;
    }

    void reset() {
        value = defaultValue;
    }
    
    void to_json(nlohmann::json &j) override {
        j[name] = value;
    }

    void from_json(nlohmann::json &j) override {
        if (j.contains(name)) {
            value = j[name].get<T>();
        } else {
            value = defaultValue;
        }
    }
};

}