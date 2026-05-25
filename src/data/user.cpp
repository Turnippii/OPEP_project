#include "../../include/data/user.h"

namespace opep {

User::User(const std::string& uname,
           const std::string& hash,
           const std::string& date)
    : username(uname), passwordHash(hash), createdDate(date) {}

// --- Getter ---
const std::string& User::getUsername()     const { return username; }
const std::string& User::getPasswordHash() const { return passwordHash; }
const std::string& User::getCreatedDate()  const { return createdDate; }

void User::setPasswordHash(const std::string& hash) { passwordHash = hash; }

// Hai user trùng nhau khi có cùng username
bool User::operator==(const User& other) const {
    return username == other.username;
}

// Mỗi trường trên một dòng riêng — phải khớp với format operator>>
std::ostream& operator<<(std::ostream& os, const User& u) {
    os << u.username     << "\n"
       << u.passwordHash << "\n"
       << u.createdDate  << "\n";
    return os;
}

// Đọc 3 dòng theo thứ tự: username, passwordHash, createdDate
std::istream& operator>>(std::istream& is, User& u) {
    std::getline(is, u.username);
    std::getline(is, u.passwordHash);
    std::getline(is, u.createdDate);
    return is;
}

} // namespace opep
