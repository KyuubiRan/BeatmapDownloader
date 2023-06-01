#pragma once
#include <json.hpp>

class ISerializable {
public:
    virtual ~ISerializable() = default;

    virtual void to_json(nlohmann::json &j) const {
    }

    virtual void from_json(const nlohmann::json &j) {
    }
};

template <>
struct nlohmann::adl_serializer<ISerializable> {
    static void to_json(json &j, ISerializable &obj) {
        obj.to_json(j);
    }

    static void from_json(json &j, ISerializable &obj) {
        obj.from_json(j);
    }
};

template<>
struct nlohmann::adl_serializer<std::vector<ISerializable>> {
    static void to_json(json &j, std::vector<ISerializable> &obj) {
        for (auto &i : obj) {
            i.to_json(j);
        }
    }

    static void from_json(json &j, std::vector<ISerializable> &obj) {
        for (auto &i : obj) {
            i.from_json(j);
        }
    }
};
