#ifndef OPEP_WALLET_H
#define OPEP_WALLET_H

#include "transaction.h"
#include <vector>
#include <memory>
#include <string>
#include <iostream>

namespace opep {

// Ví tiền của một người dùng — quản lý giao dịch và số dư
// balance là PRIVATE, chỉ thay đổi thông qua addIncome/addExpense (Encapsulation)
class Wallet {
private:
    double                                    balance;       // Số dư hiện tại (PRIVATE)
    std::vector<std::shared_ptr<Transaction>> transactions;  // Danh sách giao dịch
    int                                       nextId;        // ID tự tăng cho transaction mới
    std::string                               ownerUsername; // Tài khoản sở hữu ví

public:
    explicit Wallet(const std::string& ownerUsername, double initialBalance = 0.0);

    // --- Getter (chỉ đọc, không cho sửa trực tiếp balance) ---
    double             getBalance()       const;
    const std::string& getOwnerUsername() const;

    // Lấy toàn bộ danh sách giao dịch (read-only)
    const std::vector<std::shared_ptr<Transaction>>& getTransactions() const;

    // Thêm thu nhập: tạo Transaction INCOME, cộng vào balance
    std::shared_ptr<Transaction> addIncome(double amount,
                                           const std::string& category,
                                           const std::string& title,
                                           const std::string& date,
                                           const std::string& note = "");

    // Thêm chi tiêu: tạo Transaction EXPENSE, trừ khỏi balance
    // Ném InsufficientBalanceException nếu balance không đủ
    std::shared_ptr<Transaction> addExpense(double amount,
                                            const std::string& category,
                                            const std::string& title,
                                            const std::string& date,
                                            const std::string& note = "");

    // Nạp một transaction đã lưu từ file vào ví (không validate balance)
    // Dùng khi load data, không dùng cho giao dịch thông thường
    void loadTransaction(const Transaction& t);

    // Tổng thu nhập / chi tiêu tích lũy
    double totalIncome()  const;
    double totalExpense() const;

    // Hiển thị bảng tóm tắt ví ra console
    void displaySummary() const;

    // operator<< in thông tin ví ra ostream
    friend std::ostream& operator<<(std::ostream& os, const Wallet& wallet);
};

} // namespace opep

#endif // OPEP_WALLET_H
