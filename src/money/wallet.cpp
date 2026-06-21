#include "../../include/money/wallet.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <iomanip>

namespace opep {

Wallet::Wallet(const std::string& owner, double initialBalance)
    : balance(initialBalance)
    , nextId(1)
    , ownerUsername(owner)
{
    if (initialBalance < 0.0)
        throw InvalidInputException("Số dư ban đầu không được âm");
}

// --- Getter ---
// balance KHÔNG có setter — chỉ thay đổi qua addIncome/addExpense (Encapsulation)

double Wallet::getBalance() const { return balance; }

const std::string& Wallet::getOwnerUsername() const { return ownerUsername; }

const std::vector<std::shared_ptr<Transaction>>& Wallet::getTransactions() const {
    return transactions;
}

// --- loadTransaction (chỉ dùng khi khôi phục từ file) ---

void Wallet::loadTransaction(const Transaction& t) {
    if (t.getType() == TransactionType::INCOME)
        balance += t.getAmount();
    else
        balance -= t.getAmount();
    // Cập nhật nextId để tránh trùng ID khi thêm mới
    if (t.getId() >= nextId) nextId = t.getId() + 1;
    transactions.push_back(std::make_shared<Transaction>(t));
}

// --- addIncome ---

std::shared_ptr<Transaction> Wallet::addIncome(double amount,
                                                const std::string& category,
                                                const std::string& title,
                                                const std::string& date,
                                                const std::string& note)
{
    if (amount <= 0.0)
        throw InvalidInputException("Số tiền thu nhập phải > 0");

    auto txn = std::make_shared<Transaction>(
        nextId++, date, title, amount, TransactionType::INCOME, category, note
    );
    transactions.push_back(txn);
    balance += amount; // balance chỉ được sửa tại đây
    return txn;
}

// --- addExpense ---

std::shared_ptr<Transaction> Wallet::addExpense(double amount,
                                                 const std::string& category,
                                                 const std::string& title,
                                                 const std::string& date,
                                                 const std::string& note)
{
    if (amount <= 0.0)
        throw InvalidInputException("Số tiền chi tiêu phải > 0");
    if (amount > balance)
        throw InsufficientBalanceException(amount, balance);

    auto txn = std::make_shared<Transaction>(
        nextId++, date, title, amount, TransactionType::EXPENSE, category, note
    );
    transactions.push_back(txn);
    balance -= amount; // balance chỉ được sửa tại đây
    return txn;
}

// --- Tổng thu / chi ---

double Wallet::totalIncome() const {
    double sum = 0.0;
    for (const auto& t : transactions)
        if (t->getType() == TransactionType::INCOME) sum += t->getAmount();
    return sum;
}

double Wallet::totalExpense() const {
    double sum = 0.0;
    for (const auto& t : transactions)
        if (t->getType() == TransactionType::EXPENSE) sum += t->getAmount();
    return sum;
}

// --- displaySummary ---

void Wallet::displaySummary() const {
    std::cout << "=== Ví của: " << ownerUsername << " ===\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "  Số dư    : " << std::setw(15) << balance        << " VND\n";
    std::cout << "  Thu nhập : " << std::setw(15) << totalIncome()  << " VND\n";
    std::cout << "  Chi tiêu : " << std::setw(15) << totalExpense() << " VND\n";
    std::cout << "  Giao dịch: " << transactions.size() << " bản ghi\n";
}

// --- operator<< ---

std::ostream& operator<<(std::ostream& os, const Wallet& w) {
    os << "Wallet[" << w.ownerUsername << "] "
       << "balance=" << std::fixed << std::setprecision(0) << w.balance << " VND "
       << "(" << w.transactions.size() << " giao dịch)";
    return os;
}

} // namespace opep
