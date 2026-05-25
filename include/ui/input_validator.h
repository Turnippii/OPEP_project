#ifndef OPEP_INPUT_VALIDATOR_H
#define OPEP_INPUT_VALIDATOR_H

#include <string>

namespace opep {

// Lớp tiện ích validate và đọc input từ người dùng
// Tất cả method là static — không cần khởi tạo đối tượng
// Các hàm tự re-prompt khi input sai, chỉ throw khi stream lỗi (EOF/fail)
class InputValidator {
public:
    // Đọc số nguyên trong đoạn [min, max], re-prompt nếu ngoài khoảng hoặc không phải số
    static int readInt(const std::string& prompt, int min, int max);

    // Đọc số thực trong [min, max], re-prompt nếu không hợp lệ
    // max = 0 nghĩa là không giới hạn trên
    static double readDouble(const std::string& prompt,
                             double min = 0.0, double max = 0.0);

    // Đọc chuỗi không rỗng có độ dài trong [minLen, maxLen]
    static std::string readString(const std::string& prompt,
                                  int minLen = 1, int maxLen = 200);

    // Đọc ngày định dạng YYYY-MM-DD, re-prompt nếu sai format
    // allowEmpty = true: cho phép nhấn Enter để bỏ qua (trả về chuỗi rỗng)
    static std::string readDate(const std::string& prompt, bool allowEmpty = false);

    // Đọc password với ký tự được che bằng '*'
    // Dùng platform-specific API: _getch() trên Windows, termios trên Linux
    static std::string readPassword(const std::string& prompt, int minLen = 6);

    // Hỏi xác nhận y/n, trả về true nếu người dùng nhập 'y' hoặc 'Y'
    static bool confirm(const std::string& prompt);

    // Tạm dừng và chờ người dùng nhấn Enter
    static void pause(const std::string& msg = "Nhan Enter de tiep tuc...");

private:
    // Kiểm tra chuỗi có đúng định dạng YYYY-MM-DD và ngày hợp lệ không
    static bool isValidDate(const std::string& date);
};

} // namespace opep

#endif // OPEP_INPUT_VALIDATOR_H
