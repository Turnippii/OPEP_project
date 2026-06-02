#include "../../include/money/budget.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace opep {

// Chuẩn hóa tên category để so sánh không phân biệt hoa thường / khoảng trắng
// Giữ nguyên dấu tiếng Việt UTF-8 (multi-byte), chỉ lowercase ASCII
static std::string normalizeKey(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    std::string r;
    r.reserve(end - start + 1);
    for (size_t i = start; i <= end; ++i)
        r.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(s[i]))));
    return r;
}

// --- CategoryBudget ---

double CategoryBudget::usageRatio() const {
    if (limit <= 0.0) return 0.0;
    return spent / limit;
}

double CategoryBudget::remaining() const { return limit - spent; }

bool CategoryBudget::isExceeded() const {
    return limit > 0.0 && spent > limit;
}

bool CategoryBudget::isNearLimit() const {
    return limit > 0.0 && usageRatio() >= WARNING_THRESHOLD;
}

// --- Budget ---

Budget::Budget(const std::string& m) : month(m) {}

void Budget::setLimit(const std::string& category, double limit) {
    if (limit < 0.0)
        throw InvalidInputException("Han muc ngan sach phai >= 0");

    // Map key luôn là normalized; CategoryBudget.category giữ tên gốc để hiển thị
    std::string key = normalizeKey(category);
    auto it = categories.find(key);
    if (it != categories.end())
        it->second.limit = limit;
    else
        categories.emplace(key, CategoryBudget(category, limit));
}

bool Budget::recordExpense(const std::string& category, double amount) {
    if (amount < 0.0)
        throw InvalidInputException("So tien chi tieu phai >= 0");

    std::string key = normalizeKey(category);
    if (!categories.count(key))
        categories.emplace(key, CategoryBudget(category, 0.0));

    auto& cb = categories.at(key);
    cb.spent += amount;  // cập nhật trước khi kiểm tra để state luôn nhất quán

    if (cb.isExceeded())
        throw BudgetExceededException(category, cb.limit, cb.spent);

    return cb.isNearLimit();
}

void Budget::syncFromTransactions(const std::vector<std::shared_ptr<Transaction>>& txns) {
    // Reset toàn bộ spent về 0 trước khi tính lại
    for (auto& [key, cb] : categories)
        cb.spent = 0.0;

    for (const auto& t : txns) {
        if (t->getType() != TransactionType::EXPENSE) continue;
        if (t->getDate().substr(0, 7) != month) continue;

        const std::string& cat = t->getCategory();
        std::string key = normalizeKey(cat);
        // Nếu chưa có entry với key này, tạo mới (giữ tên gốc để hiển thị)
        if (!categories.count(key))
            categories.emplace(key, CategoryBudget(cat, 0.0));
        categories.at(key).spent += t->getAmount();
    }
}

const CategoryBudget& Budget::getCategory(const std::string& category) const {
    auto it = categories.find(normalizeKey(category));
    if (it == categories.end())
        throw InvalidInputException("Khong tim thay danh muc: " + category);
    return it->second;
}

double Budget::totalLimit() const {
    double sum = 0.0;
    for (const auto& [key, cb] : categories) sum += cb.limit;
    return sum;
}

double Budget::totalSpent() const {
    double sum = 0.0;
    for (const auto& [key, cb] : categories) sum += cb.spent;
    return sum;
}

const std::string& Budget::getMonth() const { return month; }

// --- displayBudget: bảng ngân sách với progress bar ASCII ---

void Budget::displayBudget() const {
    // Cột: Danh muc(25) | Da chi(15) | Han muc(15) | %(6) | Tien do(20) | Trang thai(20)
    constexpr int W_CAT    = 25;
    constexpr int W_SPENT  = 15;
    constexpr int W_LIMIT  = 15;
    constexpr int W_PCT    = 6;
    constexpr int BAR_FILL = 18;   // ký tự bên trong "[...]" → bar dài đúng 20 char
    constexpr int W_BAR    = 20;   // "[" + BAR_FILL + "]" = 20
    constexpr int W_STATUS = 20;
    constexpr int TOTAL_W  = W_CAT + W_SPENT + W_LIMIT + W_PCT + 2 + W_BAR + W_STATUS;

    std::cout << "\n=== Ngan sach thang " << month << " ===\n";

    // Header
    std::cout << std::left  << std::setw(W_CAT)   << "Danh muc"
              << std::right << std::setw(W_SPENT)  << "Da chi (VND)"
              << std::setw(W_LIMIT)                << "Han muc (VND)"
              << std::setw(W_PCT)                  << "%"
              << "  "
              << std::left  << std::setw(W_BAR)    << "Tien do"
              << std::setw(W_STATUS)               << "Trang thai"
              << "\n";
    std::cout << std::string(TOTAL_W, '-') << "\n";

    if (categories.empty()) {
        std::cout << "  (Chua co danh muc nao. Dat han muc qua option 'Dat han muc'.)\n";
    }

    for (const auto& [key, cb] : categories) {
        // Cắt tên category nếu > W_CAT ký tự
        std::string dispCat = cb.category;
        if (static_cast<int>(dispCat.size()) > W_CAT)
            dispCat = dispCat.substr(0, W_CAT - 3) + "...";

        // Phần trăm và progress bar
        int pct    = (cb.limit > 0.0) ? static_cast<int>(cb.usageRatio() * 100) : 0;
        int filled = (cb.limit > 0.0)
            ? std::min(static_cast<int>(cb.usageRatio() * BAR_FILL), BAR_FILL)
            : 0;
        std::string bar = "[" + std::string(filled, '#')
                        + std::string(BAR_FILL - filled, '.') + "]";

        // Chuỗi % (right-aligned trong W_PCT)
        std::string pctStr = (cb.limit > 0.0) ? (std::to_string(pct) + "%") : "n/a";

        // Trang thái
        std::string status;
        if (cb.limit <= 0.0)       status = "(khong gioi han)";
        else if (cb.isExceeded())  status = "[!!] VUOT HAN MUC!";
        else if (cb.isNearLimit()) status = "[!]  CANH BAO >=80%";
        else                       status = "[ ]  OK";

        std::cout << std::left  << std::setw(W_CAT)   << dispCat
                  << std::right << std::fixed << std::setprecision(0)
                  << std::setw(W_SPENT) << cb.spent
                  << std::setw(W_LIMIT) << (cb.limit > 0.0 ? cb.limit : 0.0)
                  << std::setw(W_PCT)   << pctStr
                  << "  "
                  << std::left  << std::setw(W_BAR)    << bar
                  << std::setw(W_STATUS) << status
                  << "\n";
    }

    std::cout << std::string(TOTAL_W, '-') << "\n";
    std::cout << std::left  << std::setw(W_CAT)   << "TONG CONG"
              << std::right << std::fixed << std::setprecision(0)
              << std::setw(W_SPENT) << totalSpent()
              << std::setw(W_LIMIT) << totalLimit()
              << "\n\n";
}

// --- operator<< ---

std::ostream& operator<<(std::ostream& os, const Budget& b) {
    os << "Budget[" << b.month << "] "
       << std::fixed << std::setprecision(0)
       << b.totalSpent() << "/" << b.totalLimit() << " VND";
    return os;
}

} // namespace opep
