#ifndef OPEP_BUDGET_H
#define OPEP_BUDGET_H

#include "transaction.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>

namespace opep {

// Khi đã dùng >= 80% hạn mức thì hiện cảnh báo
constexpr double WARNING_THRESHOLD = 0.80;

// Thông tin hạn mức và chi tiêu của một danh mục trong tháng
struct CategoryBudget {
    std::string category; // Tên danh mục
    double      limit;    // Hạn mức tháng (0 = không giới hạn)
    double      spent;    // Đã chi trong tháng

    CategoryBudget(const std::string& cat, double lim)
        : category(cat), limit(lim), spent(0.0) {}

    // Tỷ lệ đã sử dụng trong khoảng [0, 1+]
    double usageRatio() const;

    // Số tiền còn được chi
    double remaining() const;

    // Đã vượt hạn mức chưa (spent > limit, chỉ khi limit > 0)
    bool isExceeded() const;

    // Sắp đến ngưỡng cảnh báo (>= WARNING_THRESHOLD)
    bool isNearLimit() const;
};

// Quản lý ngân sách theo danh mục trong một tháng cụ thể
class Budget {
private:
    std::string                           month;      // Tháng ngân sách, dạng YYYY-MM
    std::map<std::string, CategoryBudget> categories; // Hạn mức từng danh mục

public:
    explicit Budget(const std::string& month);

    // Thiết lập hạn mức cho một danh mục (ghi đè nếu đã tồn tại)
    void setLimit(const std::string& category, double limit);

    // Xóa hạn mức của một danh mục.
    // Nếu danh mục đã có chi tiêu trong tháng thì giữ lại dòng danh mục với limit = 0.
    // Trả về true nếu có hạn mức để xóa, false nếu danh mục đã không giới hạn.
    bool removeLimit(const std::string& category);

    // Ghi nhận khoản chi vào danh mục
    // Ném BudgetExceededException nếu vượt hạn mức
    // Trả về true nếu đang trong vùng cảnh báo (>= WARNING_THRESHOLD)
    bool recordExpense(const std::string& category, double amount);

    // Tính lại toàn bộ spent từ danh sách transaction (dùng khi load file)
    void syncFromTransactions(const std::vector<std::shared_ptr<Transaction>>& txns);

    // Trả về thông tin danh mục, ném InvalidInputException nếu không tồn tại
    const CategoryBudget& getCategory(const std::string& category) const;

    // Tổng hạn mức và tổng chi tiêu toàn tháng
    double totalLimit() const;
    double totalSpent() const;

    const std::string& getMonth() const;

    // Trả về toàn bộ danh mục ngân sách để UI/GUI có thể đọc dữ liệu hiển thị
    const std::map<std::string, CategoryBudget>& getCategories() const;

    // Hiển thị bảng ngân sách với progress bar ASCII
    void displayBudget() const;

    // Lưu giới hạn ngân sách ra file (chỉ lưu limit, spent tính lại từ transactions)
    void saveLimits(const std::string& filename) const;

    // Nạp giới hạn ngân sách từ file; bỏ qua nếu file không tồn tại hoặc sai tháng
    void loadLimits(const std::string& filename);

    // operator<< in tóm tắt ngân sách ra ostream
    friend std::ostream& operator<<(std::ostream& os, const Budget& budget);
};

} // namespace opep

#endif // OPEP_BUDGET_H
