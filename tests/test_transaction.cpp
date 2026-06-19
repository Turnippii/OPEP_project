#include "test_framework.h"
#include "../include/money/transaction.h"
#include "../include/core/exceptions.h"
#include <sstream>

using namespace opep;

// Kiem tra constructor INCOME khoi tao dung tat ca truong
TEST_CASE(test_txn_constructor_income) {
    Transaction t(1, "2026-01-01", "Luong thang 1",
                  5000000.0, TransactionType::INCOME, "Luong");
    ASSERT_EQ(t.getId(), 1);
    ASSERT_EQ(t.getDate(), std::string("2026-01-01"));
    ASSERT_EQ(t.getTitle(), std::string("Luong thang 1"));
    ASSERT_EQ(t.getAmount(), 5000000.0);
    ASSERT_EQ(t.getType(), TransactionType::INCOME);
    ASSERT_EQ(t.getCategory(), std::string("Luong"));
    ASSERT_EQ(t.getNote(), std::string("")); // Note mac dinh rong
}

// Kiem tra constructor EXPENSE voi note khac rong
TEST_CASE(test_txn_constructor_expense_with_note) {
    Transaction t(2, "2026-01-15", "An trua",
                  50000.0, TransactionType::EXPENSE, "An uong", "Pho bo");
    ASSERT_EQ(t.getType(), TransactionType::EXPENSE);
    ASSERT_EQ(t.getCategory(), std::string("An uong"));
    ASSERT_EQ(t.getNote(), std::string("Pho bo"));
}

// Kiem tra amount = 0 hop le (giao dich 0 dong duoc chap nhan)
TEST_CASE(test_txn_constructor_zero_amount_ok) {
    ASSERT_NO_THROW(
        Transaction(1, "2026-01-01", "T", 0.0, TransactionType::INCOME, "C")
    );
}

// Kiem tra amount am phai throw InvalidInputException
TEST_CASE(test_txn_constructor_negative_amount_throws) {
    ASSERT_THROW(
        Transaction(1, "2026-01-01", "T", -100.0, TransactionType::INCOME, "C"),
        InvalidInputException
    );
}

// Kiem tra category rong phai throw
TEST_CASE(test_txn_constructor_empty_category_throws) {
    ASSERT_THROW(
        Transaction(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, ""),
        InvalidInputException
    );
}

// Kiem tra title rong phai throw (ke thua tu Record)
TEST_CASE(test_txn_constructor_empty_title_throws) {
    ASSERT_THROW(
        Transaction(1, "2026-01-01", "", 100.0, TransactionType::INCOME, "C"),
        InvalidInputException
    );
}

// Kiem tra setAmount cap nhat dung gia tri hop le
TEST_CASE(test_txn_setAmount_valid) {
    Transaction t(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, "C");
    t.setAmount(999.0);
    ASSERT_EQ(t.getAmount(), 999.0);
}

// Kiem tra setAmount voi gia tri am phai throw
TEST_CASE(test_txn_setAmount_negative_throws) {
    Transaction t(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, "C");
    ASSERT_THROW(t.setAmount(-1.0), InvalidInputException);
}

// Kiem tra setCategory voi chuoi rong phai throw
TEST_CASE(test_txn_setCategory_empty_throws) {
    Transaction t(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, "Cat");
    ASSERT_THROW(t.setCategory(""), InvalidInputException);
}

// Kiem tra operator== khi hai giao dich giong nhau hoan toan
TEST_CASE(test_txn_operator_eq_equal) {
    Transaction t1(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, "C");
    Transaction t2(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, "C");
    ASSERT_TRUE(t1 == t2);
}

// Kiem tra operator== khi type khac nhau → khong bang
TEST_CASE(test_txn_operator_eq_diff_type) {
    Transaction t1(1, "2026-01-01", "T", 100.0, TransactionType::INCOME,  "C");
    Transaction t2(1, "2026-01-01", "T", 100.0, TransactionType::EXPENSE, "C");
    ASSERT_FALSE(t1 == t2);
}

// Kiem tra operator!= khi amount khac nhau
TEST_CASE(test_txn_operator_ne_diff_amount) {
    Transaction t1(1, "2026-01-01", "T", 100.0, TransactionType::INCOME, "C");
    Transaction t2(1, "2026-01-01", "T", 200.0, TransactionType::INCOME, "C");
    ASSERT_TRUE(t1 != t2);
}

// Kiem tra helper transactionTypeToString
TEST_CASE(test_txn_typeToString) {
    ASSERT_EQ(transactionTypeToString(TransactionType::INCOME),  std::string("INCOME"));
    ASSERT_EQ(transactionTypeToString(TransactionType::EXPENSE), std::string("EXPENSE"));
}

// Kiem tra helper stringToTransactionType
TEST_CASE(test_txn_stringToType_valid) {
    ASSERT_EQ(stringToTransactionType("INCOME"),  TransactionType::INCOME);
    ASSERT_EQ(stringToTransactionType("EXPENSE"), TransactionType::EXPENSE);
}

// Kiem tra stringToTransactionType voi gia tri sai phai throw
TEST_CASE(test_txn_stringToType_invalid_throws) {
    ASSERT_THROW(stringToTransactionType("UNKNOWN"),  InvalidInputException);
    ASSERT_THROW(stringToTransactionType("income"),   InvalidInputException); // Case-sensitive
    ASSERT_THROW(stringToTransactionType(""),         InvalidInputException);
}

// Kiem tra serialization: << roi >> phuc hoi dung gia tri (note rong dung sentinel ~)
TEST_CASE(test_txn_serialization_empty_note) {
    Transaction orig(10, "2026-03-15", "Tien dien",
                     200000.0, TransactionType::EXPENSE, "Tien ich");
    std::ostringstream oss;
    oss << orig;
    Transaction loaded;
    std::istringstream iss(oss.str());
    iss >> loaded;

    ASSERT_EQ(loaded.getId(), 10);
    ASSERT_EQ(loaded.getDate(), std::string("2026-03-15"));
    ASSERT_EQ(loaded.getAmount(), 200000.0);
    ASSERT_EQ(loaded.getType(), TransactionType::EXPENSE);
    ASSERT_EQ(loaded.getCategory(), std::string("Tien ich"));
    ASSERT_EQ(loaded.getNote(), std::string("")); // "~" duoc khoi phuc thanh ""
}

// Kiem tra serialization voi note co noi dung
TEST_CASE(test_txn_serialization_with_note) {
    Transaction orig(5, "2026-06-19", "An sang",
                     35000.0, TransactionType::EXPENSE, "An uong", "Banh mi thit");
    std::ostringstream oss;
    oss << orig;
    Transaction loaded;
    std::istringstream iss(oss.str());
    iss >> loaded;

    ASSERT_EQ(loaded.getNote(), std::string("Banh mi thit"));
    ASSERT_EQ(loaded.getType(), TransactionType::EXPENSE);
}

// --- Ham chay toan bo Transaction tests ---
void runTransactionTests() {
    std::cout << "\n--- Transaction Tests ---\n";
    RUN_TEST(test_txn_constructor_income);
    RUN_TEST(test_txn_constructor_expense_with_note);
    RUN_TEST(test_txn_constructor_zero_amount_ok);
    RUN_TEST(test_txn_constructor_negative_amount_throws);
    RUN_TEST(test_txn_constructor_empty_category_throws);
    RUN_TEST(test_txn_constructor_empty_title_throws);
    RUN_TEST(test_txn_setAmount_valid);
    RUN_TEST(test_txn_setAmount_negative_throws);
    RUN_TEST(test_txn_setCategory_empty_throws);
    RUN_TEST(test_txn_operator_eq_equal);
    RUN_TEST(test_txn_operator_eq_diff_type);
    RUN_TEST(test_txn_operator_ne_diff_amount);
    RUN_TEST(test_txn_typeToString);
    RUN_TEST(test_txn_stringToType_valid);
    RUN_TEST(test_txn_stringToType_invalid_throws);
    RUN_TEST(test_txn_serialization_empty_note);
    RUN_TEST(test_txn_serialization_with_note);
}
