#pragma once

#include "ISerializable.h"

namespace config {

template <typename T>
class Field : ISerializable {
    std::string_view name;
    T value;
public:

    Field(std::string_view name) : name(name), value(T{}) {
    }
    Field(std::string_view name, T value) : name(name), value(value) {
    }

    std::string_view getName() const {
        return name;
    }

    T &getValue() const {
        return value;
    }

    void setValue(T _value) {
        this->value = _value;
    }

    operator T &() const {
        return value;
    }

    Field &operator=(T _value) {
        setValue(_value);
        return *this;
    }

    T *getPtr() const {
        return &value;
    }
};

}