#ifndef OPEP_TRANSACTION_H
#define OPEP_TRANSACTION_H

#include "../core/record.h"
#include <string>
#include <iostream>

namespace opep {

// Loại giao dịch: thu nhập hoặc chi tiêu
enum class TransactionType {
    INCOME  = 0,
    EXPENSE = 1
};

// Chuyển TransactionType <-> string, dùng trong displayInfo và file I/O
std::string     transactionTypeToString(TransactionType t);
TransactionType stringToTransactionType(const std::string& s);

// Lớp Transaction kế thừa Record, đại diện cho một giao dịch tài chính
class Transaction : public Record {
private:
    double          amount;    // Số tiền giao dịch (luôn >= 0)
    TransactionType type;      // INCOME hoặc EXPENSE
    std::string     category;  // Danh mục (An uong, Di lai, Luong...)
    std::string     note;      // Ghi chú thêm (có thể rỗng)

public:
    // Default constructor cho Database<Transaction>::load()
    Transaction() : Record(), amount(0.0), type(TransactionType::INCOME), category("?") {}

    Transaction(int id,
                const std::string& date,
                const std::string& title,
                double amount,
                TransactionType type,
                const std::string& category,
                const std::string& note = "");

    // --- Getter ---
    double             getAmount()   const;
    TransactionType    getType()     const;
    const std::string& getCategory() const;
    const std::string& getNote()     const;

    // --- Setter ---
    void setAmount(double amount);
    void setCategory(const std::string& category);
    void setNote(const std::string& note);

    // Override pure virtual từ Record
    void displayInfo() const override;

    // So sánh đầy đủ: id, amount, type, category, date
    bool operator==(const Transaction& other) const;
    bool operator!=(const Transaction& other) const;

    // operator<< / operator>> cho serialization vào Database<Transaction>
    // Format: "<id> <date> <TYPE> <amount>\n<title>\n<category>\n<note>\n"
    friend std::ostream& operator<<(std::ostream& os, const Transaction& t);
    friend std::istream& operator>>(std::istream& is, Transaction& t);
};

} // namespace opep

#endif // OPEP_TRANSACTION_H
