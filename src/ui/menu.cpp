#include "../../include/ui/menu.h"
#include "../../include/ui/input_validator.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace opep {

static int utf8Width(const std::string& s) {
    int w = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++w;
    return w;
}

static std::string utf8FitLeft(const std::string& s, int width) {
    if (width <= 0) return "";
    int w = utf8Width(s);
    if (w <= width) return s + std::string(width - w, ' ');

    int target = std::max(0, width - 3);
    int cur = 0;
    size_t i = 0;
    while (i < s.size() && cur < target) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t cl = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
        ++cur;
        i += cl;
    }
    return s.substr(0, i) + "...";
}

static std::string utf8Center(const std::string& s, int width) {
    int w = utf8Width(s);
    if (w >= width) return utf8FitLeft(s, width);
    int left = (width - w) / 2;
    int right = width - w - left;
    return std::string(left, ' ') + s + std::string(right, ' ');
}

static constexpr int MENU_WIDTH = 44;

Menu::Menu(const std::string& t, const std::string& back)
    : title(t), running(false), backLabel(back) {}

void Menu::addItem(const std::string& label, std::function<void()> action) {
    items.push_back({label, std::move(action), false});
}

void Menu::addSeparator() {
    items.push_back({"", nullptr, true});
}

void Menu::stop() { running = false; }

int Menu::countItems() const {
    int n = 0;
    for (const auto& it : items) if (!it.isSeparator) ++n;
    return n;
}

// --- display ---

void Menu::display() const {
    auto hLine = [](char c) {
        std::cout << "+" << std::string(MENU_WIDTH - 2, c) << "+\n";
    };
    auto padRow = [](const std::string& s) {
        int inner = MENU_WIDTH - 2;
        std::cout << "|" << utf8FitLeft(" " + s, inner) << "|\n";
    };

    std::cout << "\n";
    hLine('=');

    // Tiêu đề căn giữa
    int inner = MENU_WIDTH - 2;
    std::cout << "|" << utf8Center(title, inner) << "|\n";

    hLine('=');

    int idx = 1;
    for (const auto& item : items) {
        if (item.isSeparator) {
            hLine('-');
        } else {
            std::string line = "  [" + std::to_string(idx++) + "] " + item.label;
            padRow(line);
        }
    }

    hLine('-');
    padRow("  [0] " + backLabel);
    hLine('=');
    std::cout << "  Chọn: ";
}

// --- run ---

void Menu::run() {
    running = true;
    while (running) {
        display();
        int choice = InputValidator::readInt("", 0, countItems());

        if (choice == 0) { running = false; break; }

        // Tìm item thứ [choice] (bỏ qua separator)
        int idx = 0;
        for (const auto& item : items) {
            if (item.isSeparator) continue;
            if (++idx == choice) {
                try {
                    if (item.action) item.action();
                } catch (const OPEPException& e) {
                    std::cout << "\n  [LỖI] " << e.what() << "\n";
                    InputValidator::pause();
                } catch (const std::exception& e) {
                    std::cout << "\n  [LỖI] " << e.what() << "\n";
                    InputValidator::pause();
                }
                break;
            }
        }
    }
}

} // namespace opep
