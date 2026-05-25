#ifndef OPEP_AUTH_H
#define OPEP_AUTH_H

#include "user.h"
#include "file_manager.h"
#include <string>
#include <memory>

namespace opep {

// Quản lý đăng ký và đăng nhập người dùng
// Mỗi lần đăng nhập thành công trả về shared_ptr<User> để các module khác dùng
class AuthManager {
private:
    std::string    dataDir; // Thư mục gốc chứa dữ liệu, mặc định "data/"
    Database<User> userDb;  // Lưu toàn bộ tài khoản vào users.db

    // Kiểm tra định dạng username hợp lệ: 3-20 ký tự, chỉ a-z A-Z 0-9 _
    static void validateUsername(const std::string& username);

    // Kiểm tra password đủ mạnh: tối thiểu 6 ký tự
    static void validatePassword(const std::string& password);

    // Lấy ngày hiện tại theo định dạng YYYY-MM-DD
    static std::string currentDate();

public:
    explicit AuthManager(const std::string& dataDir = "data/");

    // Đăng ký tài khoản mới
    // Ném InvalidInputException nếu username/password không hợp lệ
    // Ném AuthenticationException nếu username đã tồn tại
    std::shared_ptr<User> registerUser(const std::string& username,
                                       const std::string& password);

    // Đăng nhập
    // Ném AuthenticationException nếu thông tin sai hoặc không tồn tại
    std::shared_ptr<User> login(const std::string& username,
                                const std::string& password);

    // Đổi mật khẩu — xác minh oldPassword trước khi ghi
    // Ném AuthenticationException nếu oldPassword sai
    void changePassword(const std::string& username,
                        const std::string& oldPassword,
                        const std::string& newPassword);

    // Kiểm tra username có tồn tại trong database không
    bool userExists(const std::string& username) const;
};

} // namespace opep

#endif // OPEP_AUTH_H
