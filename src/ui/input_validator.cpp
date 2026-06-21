#include "../../include/ui/input_validator.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <ctime>

// Import platform-specific header để mask password
#ifdef _WIN32
#  include <conio.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

namespace opep {

// --- readInt ---

int InputValidator::readInt(const std::string& prompt, int min, int max) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line))
            throw InvalidInputException("Lỗi đọc input (stream fail)");

        try {
            size_t pos;
            int val = std::stoi(line, &pos);
            // Đảm bảo không có ký tự thừa sau số
            if (pos != line.size()) throw std::invalid_argument("");
            if (val < min || val > max) {
                std::cout << "  [!] Vui lòng nhập số từ " << min
                          << " đến " << max << "\n";
                continue;
            }
            return val;
        } catch (const std::exception&) {
            std::cout << "  [!] Vui lòng nhập số nguyên hợp lệ\n";
        }
    }
}

// --- readDouble ---

double InputValidator::readDouble(const std::string& prompt, double min, double max) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line))
            throw InvalidInputException("Lỗi đọc input (stream fail)");

        try {
            size_t pos;
            double val = std::stod(line, &pos);
            if (pos != line.size()) throw std::invalid_argument("");
            if (val < min) {
                std::cout << "  [!] Giá trị phải >= " << min << "\n";
                continue;
            }
            if (max > 0.0 && val > max) {
                std::cout << "  [!] Giá trị phải <= " << max << "\n";
                continue;
            }
            return val;
        } catch (const std::exception&) {
            std::cout << "  [!] Vui lòng nhập số thực hợp lệ\n";
        }
    }
}

// --- readString ---

std::string InputValidator::readString(const std::string& prompt, int minLen, int maxLen) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line))
            throw InvalidInputException("Lỗi đọc input (stream fail)");

        int len = static_cast<int>(line.size());
        if (len < minLen) {
            std::cout << "  [!] Chuỗi phải có ít nhất " << minLen << " ký tự\n";
            continue;
        }
        if (len > maxLen) {
            std::cout << "  [!] Chuỗi không được quá " << maxLen << " ký tự\n";
            continue;
        }
        return line;
    }
}

// --- readDate ---

bool InputValidator::isValidDate(const std::string& d) {
    if (d.size() != 10) return false;
    if (d[4] != '-' || d[7] != '-') return false;

    for (int i : {0,1,2,3,5,6,8,9})
        if (!std::isdigit(static_cast<unsigned char>(d[i]))) return false;

    int y = std::stoi(d.substr(0, 4));
    int m = std::stoi(d.substr(5, 2));
    int day = std::stoi(d.substr(8, 2));

    if (y < 2000 || y > 2100) return false;
    if (m < 1 || m > 12) return false;

    // Số ngày tối đa trong từng tháng (xử lý năm nhuận)
    int maxDay[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (leap) maxDay[1] = 29;

    return day >= 1 && day <= maxDay[m - 1];
}

std::string InputValidator::readDate(const std::string& prompt, bool allowEmpty) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line))
            throw InvalidInputException("Lỗi đọc input (stream fail)");

        if (allowEmpty && line.empty()) return "";

        if (!isValidDate(line)) {
            std::cout << "  [!] Định dạng ngày phải là YYYY-MM-DD (vd: 2026-05-25)\n";
            continue;
        }
        return line;
    }
}

// --- readPassword ---

std::string InputValidator::readPassword(const std::string& prompt, int minLen) {
    while (true) {
        std::cout << prompt;
        std::string pw;

#ifdef _WIN32
        // Windows: dùng _getch() để đọc từng ký tự không hiển thị lên màn hình
        int c;
        while ((c = _getch()) != '\r') {
            if (c == '\b') {
                if (!pw.empty()) {
                    pw.pop_back();
                    std::cout << "\b \b" << std::flush;
                }
            } else if (c >= 32 && c < 127) {
                pw.push_back(static_cast<char>(c));
                std::cout << '*' << std::flush;
            }
        }
        std::cout << '\n';
#else
        // Linux/macOS: tắt echo qua termios
        termios old{}, now{};
        tcgetattr(STDIN_FILENO, &old);
        now = old;
        now.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &now);
        std::getline(std::cin, pw);
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        std::cout << '\n';
#endif

        if (static_cast<int>(pw.size()) < minLen) {
            std::cout << "  [!] Mật khẩu phải có ít nhất " << minLen << " ký tự\n";
            continue;
        }
        return pw;
    }
}

// --- confirm ---

bool InputValidator::confirm(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " (y/n): ";
        std::string line;
        if (!std::getline(std::cin, line)) return false;
        if (line == "y" || line == "Y") return true;
        if (line == "n" || line == "N") return false;
        std::cout << "  [!] Vui lòng nhập 'y' hoặc 'n'\n";
    }
}

// --- pause ---

void InputValidator::pause(const std::string& msg) {
    std::cout << msg;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace opep
