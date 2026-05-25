#ifndef OPEP_EXCEPTIONS_H
#define OPEP_EXCEPTIONS_H

#include <exception>
#include <string>

namespace opep {

// --- Lớp base cho tất cả exception trong ứng dụng ---
// Lưu message dạng std::string thay vì const char* để tránh dangling pointer
class OPEPException : public std::exception {
protected:
    std::string message;

public:
    explicit OPEPException(const std::string& msg) : message(msg) {}

    // override what() trả về C-string theo chuẩn std::exception
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Ném khi user nhập dữ liệu không hợp lệ (số âm, chuỗi rỗng, định dạng sai...)
class InvalidInputException : public OPEPException {
public:
    explicit InvalidInputException(const std::string& msg)
        : OPEPException("Input khong hop le: " + msg) {}
};

// Ném khi không tìm thấy file dữ liệu cần đọc/ghi
class FileNotFoundException : public OPEPException {
public:
    explicit FileNotFoundException(const std::string& filename)
        : OPEPException("Khong tim thay file: " + filename) {}
};

// Ném khi thông tin đăng nhập sai (sai mật khẩu, tài khoản không tồn tại...)
class AuthenticationException : public OPEPException {
public:
    explicit AuthenticationException(const std::string& msg)
        : OPEPException("Xac thuc that bai: " + msg) {}
};

// Ném khi thực hiện giao dịch vượt quá số dư hiện có
class InsufficientBalanceException : public OPEPException {
public:
    explicit InsufficientBalanceException(double required, double available)
        : OPEPException("So du khong du: can " + std::to_string(required)
                        + ", hien co " + std::to_string(available)) {}
};

} // namespace opep

#endif // OPEP_EXCEPTIONS_H
