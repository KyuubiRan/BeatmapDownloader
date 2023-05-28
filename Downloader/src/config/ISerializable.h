#pragma once
#include <json.hpp>

namespace config {

class ISerializable {
public:
    virtual ~ISerializable() = default;

    virtual void to_json(nlohmann::json &j) {
    }

    virtual void from_json(nlohmann::json &j) {
    }
};

}

template <>
struct nlohmann::adl_serializer<config::ISerializable> {
    static void to_json(json &j, config::ISerializable &obj) {
        obj.to_json(j);
    }

    static void from_json(json &j, config::ISerializable &obj) {
        obj.from_json(j);
    }
};