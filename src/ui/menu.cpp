#include "../../include/ui/menu.h"
#include "../../include/ui/input_validator.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <iomanip>

namespace opep {

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
        std::cout << "| " << std::left << std::setw(inner - 1) << s << "|\n";
    };

    std::cout << "\n";
    hLine('=');

    // Tiêu đề căn giữa
    int inner = MENU_WIDTH - 2;
    int pad   = (inner - static_cast<int>(title.size())) / 2;
    pad       = std::max(0, pad);
    std::string centered = std::string(pad, ' ') + title;
    std::cout << "| " << std::left << std::setw(inner - 1) << centered << "|\n";

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
    std::cout << "  Chon: ";
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
                    std::cout << "\n  [LOI] " << e.what() << "\n";
                    InputValidator::pause();
                } catch (const std::exception& e) {
                    std::cout << "\n  [LOI] " << e.what() << "\n";
                    InputValidator::pause();
                }
                break;
            }
        }
    }
}

} // namespace opep
