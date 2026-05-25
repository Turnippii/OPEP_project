#ifndef OPEP_USER_H
#define OPEP_USER_H

#include <string>
#include <iostream>

namespace opep {

// Đại diện cho một tài khoản người dùng trong hệ thống
class User {
private:
    std::string username;     // Tên đăng nhập (duy nhất, 3-20 ký tự)
    std::string passwordHash; // Hash của password — KHÔNG lưu plain text
    std::string createdDate;  // Ngày tạo tài khoản, định dạng YYYY-MM-DD

public:
    // Constructor mặc định cần cho Database<User>::load()
    User() = default;
    User(const std::string& username,
         const std::string& passwordHash,
         const std::string& createdDate);

    // --- Getter ---
    const std::string& getUsername()     const;
    const std::string& getPasswordHash() const;
    const std::string& getCreatedDate()  const;

    // Chỉ AuthManager được phép cập nhật hash (đổi mật khẩu)
    void setPasswordHash(const std::string& hash);

    // Operator== so sánh theo username (case-sensitive)
    bool operator==(const User& other) const;

    // operator<< ghi mỗi trường trên một dòng (dùng cho Database<User>)
    friend std::ostream& operator<<(std::ostream& os, const User& user);

    // operator>> đọc 3 dòng: username, passwordHash, createdDate
    friend std::istream& operator>>(std::istream& is, User& user);
};

} // namespace opep

#endif // OPEP_USER_H
