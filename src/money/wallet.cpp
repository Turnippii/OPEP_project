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
        throw InvalidInputException("So du ban dau khong duoc am");
}

// --- Getter ---
// balance KHÔNG có setter — chỉ thay đổi qua addIncome/addExpense (Encapsulation)

double Wallet::getBalance() const { return balance; }

const std::string& Wallet::getOwnerUsername() const { return ownerUsername; }

const std::vector<std::shared_ptr<Transaction>>& Wallet::getTransactions() const {
    return transactions;
}

// --- addIncome ---

std::shared_ptr<Transaction> Wallet::addIncome(double amount,
                                                const std::string& category,
                                                const std::string& title,
                                                const std::string& date,
                                                const std::string& note)
{
    if (amount <= 0.0)
        throw InvalidInputException("So tien thu nhap phai > 0");

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
        throw InvalidInputException("So tien chi tieu phai > 0");
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
    std::cout << "=== Vi cua: " << ownerUsername << " ===\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "  So du    : " << std::setw(15) << balance        << " VND\n";
    std::cout << "  Thu nhap : " << std::setw(15) << totalIncome()  << " VND\n";
    std::cout << "  Chi tieu : " << std::setw(15) << totalExpense() << " VND\n";
    std::cout << "  Giao dich: " << transactions.size() << " ban ghi\n";
}

// --- operator<< ---

std::ostream& operator<<(std::ostream& os, const Wallet& w) {
    os << "Wallet[" << w.ownerUsername << "] "
       << "balance=" << std::fixed << std::setprecision(0) << w.balance << " VND "
       << "(" << w.transactions.size() << " giao dich)";
    return os;
}

} // namespace opep
