#include "../../include/money/transaction.h"
#include "../../include/core/exceptions.h"
#include <iomanip>

namespace opep {

// --- Helper functions ---

std::string transactionTypeToString(TransactionType t) {
    return (t == TransactionType::INCOME) ? "INCOME" : "EXPENSE";
}

TransactionType stringToTransactionType(const std::string& s) {
    if (s == "INCOME")  return TransactionType::INCOME;
    if (s == "EXPENSE") return TransactionType::EXPENSE;
    throw InvalidInputException("Loai giao dich khong hop le: " + s);
}

// --- Constructor ---

Transaction::Transaction(int id,
                         const std::string& date,
                         const std::string& title,
                         double amount,
                         TransactionType type,
                         const std::string& category,
                         const std::string& note)
    : Record(id, date, title)
    , amount(amount)
    , type(type)
    , category(category)
    , note(note)
{
    if (amount < 0.0)
        throw InvalidInputException("So tien giao dich phai >= 0");
    if (category.empty())
        throw InvalidInputException("Danh muc khong duoc de trong");
}

// --- Getter ---

double             Transaction::getAmount()   const { return amount; }
TransactionType    Transaction::getType()     const { return type; }
const std::string& Transaction::getCategory() const { return category; }
const std::string& Transaction::getNote()     const { return note; }

// --- Setter ---

void Transaction::setAmount(double a) {
    if (a < 0.0)
        throw InvalidInputException("So tien giao dich phai >= 0");
    amount = a;
}

void Transaction::setCategory(const std::string& cat) {
    if (cat.empty())
        throw InvalidInputException("Danh muc khong duoc de trong");
    category = cat;
}

void Transaction::setNote(const std::string& n) { note = n; }

// --- displayInfo ---

void Transaction::displayInfo() const {
    std::string typeStr = transactionTypeToString(type);
    std::cout << "[" << typeStr << "] #" << id
              << " | " << date
              << " | " << std::left  << std::setw(25) << title
              << " | " << std::right << std::setw(12)
              << std::fixed << std::setprecision(0) << amount << " VND"
              << " | " << category;
    if (!note.empty()) std::cout << " (" << note << ")";
    std::cout << "\n";
}

// --- Serialization operator<< / operator>> cho Database<Transaction> ---
// Format: "<id> <date> <TYPE> <amount>\n<title>\n<category>\n<note>\n"

std::ostream& operator<<(std::ostream& os, const Transaction& t) {
    os << t.id << " " << t.date << " "
       << transactionTypeToString(t.type) << " "
       << std::fixed << std::setprecision(2) << t.amount << "\n"
       << t.title    << "\n"
       << t.category << "\n"
       << t.note     << "\n";
    return os;
}

std::istream& operator>>(std::istream& is, Transaction& t) {
    std::string typeStr;
    is >> t.id >> t.date >> typeStr >> t.amount;
    is.ignore();
    std::getline(is, t.title);
    std::getline(is, t.category);
    std::getline(is, t.note);
    t.type = stringToTransactionType(typeStr);
    return is;
}

} // namespace opep
