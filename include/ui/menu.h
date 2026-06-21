#ifndef OPEP_MENU_H
#define OPEP_MENU_H

#include <string>
#include <vector>
#include <functional>

namespace opep {

// Một mục trong menu: nhãn hiển thị và hành động khi chọn
struct MenuItem {
    std::string           label;
    std::function<void()> action;
    bool                  isSeparator = false; // Dòng kẻ ngang, không có số thứ tự
};

// Menu console nhiều cấp
// Cách dùng nhiều cấp: action của một item gọi menu.run() của submenu
// Exception từ action được bắt và hiển thị lỗi thay vì crash
class Menu {
private:
    std::string           title;
    std::vector<MenuItem> items;
    bool                  running;
    std::string           backLabel; // Nhãn mục [0]: "Quay lại" hoặc "Thoát"

    // In khung menu ra stdout
    void display() const;

    // Đếm số item thực sự (bỏ qua separator)
    int countItems() const;

public:
    // backLabel = "Thoát" cho menu gốc, "Quay lại" cho submenu
    explicit Menu(const std::string& title,
                  const std::string& backLabel = "Quay lại");

    // Thêm một mục có hành động
    void addItem(const std::string& label, std::function<void()> action);

    // Thêm dòng kẻ ngang giữa các nhóm mục
    void addSeparator();

    // Chạy vòng lặp menu cho đến khi user chọn [0]
    // Exception từ action được bắt, in lỗi, rồi tiếp tục vòng lặp
    void run();

    // Dừng vòng lặp từ bên trong một action (dùng cho "Đăng xuất")
    void stop();
};

} // namespace opep

#endif // OPEP_MENU_H
