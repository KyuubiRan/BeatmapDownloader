#include "pch.h"
#include "Account.h"

#include <regex>

#include "utils/MD5.hpp"

void osu::Password::to_json(nlohmann::json &j) const {
    j["PasswordHash"] = md5();
}

void osu::Password::from_json(const nlohmann::json &j) {
    j["PasswordHash"].get_to(m_Password);
}

std::string osu::Password::md5() const {
    if (m_Password.empty())
        return "";

    static const std::string md5Regex = "^[a-f0-9]{32}$";
    if (std::regex_match(m_Password, std::regex(md5Regex))) {
        return m_Password;
    }
    MD5 md5{};
    md5.update(m_Password);
    return md5.str();
}

void osu::Account::to_json(nlohmann::json &j) const {
    j["PasswordHash"] = m_Password.md5();
    j["Username"] = m_Username;
}

void osu::Account::from_json(const nlohmann::json &j) {
    j["PasswordHash"].get_to(m_Password.m_Password);
    j["Username"].get_to(m_Username);
}
