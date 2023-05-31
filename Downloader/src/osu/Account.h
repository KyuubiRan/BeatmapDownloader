#pragma once
#include <string>

namespace osu {
class Password {
    inline const static char *s_Entropy = "cu24180ncjeiu0ci1nwui";
    std::string m_Password;
    
public:
    explicit Password(std::string pw);
};


class Account {
    std::string m_Username;
    Password m_Password;
public:
    Account(std::string username, Password pw);
};
}
