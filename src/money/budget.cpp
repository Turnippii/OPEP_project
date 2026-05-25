#include "../../include/money/budget.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace opep {

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

    auto it = categories.find(category);
    if (it != categories.end())
        it->second.limit = limit;
    else
        categories.emplace(category, CategoryBudget(category, limit));
}

bool Budget::recordExpense(const std::string& category, double amount) {
    if (amount < 0.0)
        throw InvalidInputException("So tien chi tieu phai >= 0");

    // Tự động tạo danh mục không giới hạn nếu chưa có
    if (!categories.count(category))
        categories.emplace(category, CategoryBudget(category, 0.0));

    auto& cb = categories.at(category);
    cb.spent += amount;

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
        // Chỉ tính giao dịch trong tháng khớp với ngân sách này
        if (t->getDate().substr(0, 7) != month) continue;

        const std::string& cat = t->getCategory();
        if (!categories.count(cat))
            categories.emplace(cat, CategoryBudget(cat, 0.0));
        categories.at(cat).spent += t->getAmount();
    }
}

const CategoryBudget& Budget::getCategory(const std::string& category) const {
    auto it = categories.find(category);
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
    constexpr int BAR_WIDTH = 20;
    std::cout << "=== Ngan sach thang " << month << " ===\n";
    std::cout << std::left  << std::setw(16) << "Danh muc"
              << std::right << std::setw(12) << "Da chi"
              << std::setw(12) << "Han muc"
              << "  Tien do\n";
    std::cout << std::string(65, '-') << "\n";

    for (const auto& [key, cb] : categories) {
        int filled = (cb.limit > 0.0)
            ? static_cast<int>(cb.usageRatio() * BAR_WIDTH)
            : 0;
        filled = std::min(filled, BAR_WIDTH);

        std::string bar = "[" + std::string(filled, '#')
                        + std::string(BAR_WIDTH - filled, '.') + "]";

        std::cout << std::left  << std::setw(16) << cb.category
                  << std::right << std::fixed << std::setprecision(0)
                  << std::setw(12) << cb.spent
                  << std::setw(12) << cb.limit
                  << "  " << bar;

        if (cb.isExceeded())       std::cout << " !! VUOT HAN MUC";
        else if (cb.isNearLimit()) std::cout << " >> CANH BAO";
        std::cout << "\n";
    }

    std::cout << std::string(65, '-') << "\n";
    std::cout << std::left  << std::setw(16) << "TONG CONG"
              << std::right << std::fixed << std::setprecision(0)
              << std::setw(12) << totalSpent()
              << std::setw(12) << totalLimit() << "\n";
}

// --- operator<< ---

std::ostream& operator<<(std::ostream& os, const Budget& b) {
    os << "Budget[" << b.month << "] "
       << std::fixed << std::setprecision(0)
       << b.totalSpent() << "/" << b.totalLimit() << " VND";
    return os;
}

} // namespace opep
