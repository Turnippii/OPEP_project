#ifndef OPEP_DASHBOARD_H
#define OPEP_DASHBOARD_H

#include "../money/wallet.h"
#include "../task/task_manager.h"
#include <string>
#include <vector>

namespace opep {

// Dashboard ASCII hiển thị tổng quan khi đăng nhập
// Bao gồm: số dư ví, thu/chi tháng, task sắp đến hạn, chi tiêu hôm nay
class Dashboard {
private:
    const Wallet&      wallet;
    const TaskManager& taskManager;
    std::string        username;
    std::string        today; // YYYY-MM-DD lấy lúc khởi tạo

    static constexpr int WIDTH = 64; // Chiều rộng dashboard tính cả border

    // --- Lấy ngày hôm nay ---
    static std::string getCurrentDate();

    // Cộng n ngày vào chuỗi ngày YYYY-MM-DD, trả về chuỗi mới
    static std::string addDays(const std::string& date, int n);

    // Trả về tháng hiện tại dạng YYYY-MM
    static std::string getCurrentMonth(const std::string& date);

    // --- Tính toán dữ liệu ---

    // Tasks chưa hoàn thành có deadline trong vòng [days] ngày tới
    std::vector<const Task*> getUpcomingTasks(int days = 3) const;

    // Tổng chi tiêu hôm nay
    double getTodayExpense() const;

    // Tổng thu nhập / chi tiêu trong tháng hiện tại
    double getMonthIncome()  const;
    double getMonthExpense() const;

    // --- Tiện ích vẽ khung ---

    // In dòng ngang: +----...----+
    static void hLine(char corner, char fill, int w);

    // In một hàng với nội dung căn trái: | text           |
    static void row(const std::string& content, int w);

    // In một hàng với nội dung căn giữa
    static void rowCenter(const std::string& content, int w);

    // Định dạng số tiền: 1500000 -> "1,500,000 VND"
    static std::string fmtMoney(double amount);

    // Định dạng dương/âm: +1,500,000 VND
    static std::string fmtSigned(double amount);

public:
    Dashboard(const Wallet& wallet,
              const TaskManager& taskManager,
              const std::string& username);

    // Vẽ toàn bộ dashboard ra stdout
    void render() const;
};

} // namespace opep

#endif // OPEP_DASHBOARD_H
