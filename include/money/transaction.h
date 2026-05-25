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

    // operator>> đọc dữ liệu transaction từ istream (dùng khi load file)
    friend std::istream& operator>>(std::istream& is, Transaction& t);
};

} // namespace opep

#endif // OPEP_TRANSACTION_H
