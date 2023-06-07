#pragma once
#include <string>

#include "misc/ISerializable.h"

namespace osu {
class Password : public ISerializable {
    std::string m_Password;

    friend class Account;

public:
    Password() = default;

    explicit Password(std::string pw) :
        m_Password(std::move(pw)) {
    }

    void to_json(nlohmann::json &j) const override;
    void from_json(const nlohmann::json &j) override;

    std::string md5() const;
};

class Account : public ISerializable {
    std::string m_Username;
    Password m_Password;

public:
    Account() = default;

    Account(std::string username, Password pw) :
        m_Username(std::move(username)), m_Password(std::move(pw)) {
    }

    [[nodiscard]] const std::string &username() const { return m_Username; }
    [[nodiscard]] const Password &password() const { return m_Password; }


    void setUsername(const std::string &un) {
        m_Username = un;
    }

    void setPassword(const std::string &pw) {
        m_Password.m_Password = pw;
    }

    void to_json(nlohmann::json &j) const override;
    void from_json(const nlohmann::json &j) override;
};
}

template <>
struct nlohmann::adl_serializer<osu::Account> {
    static void to_json(json &j, const osu::Account &acc) {
        acc.to_json(j);
    }

    static void from_json(const json &j, osu::Account &acc) {
        acc.from_json(j);
    }
};
