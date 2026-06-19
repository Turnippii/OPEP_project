#include "test_framework.h"
#include "../include/money/wallet.h"
#include "../include/core/exceptions.h"
#include <sstream>

using namespace opep;

// Kiem tra constructor khoi tao vi voi so du ban dau
TEST_CASE(test_wallet_constructor_basic) {
    Wallet w("alice", 1000000.0);
    ASSERT_EQ(w.getOwnerUsername(), std::string("alice"));
    ASSERT_EQ(w.getBalance(), 1000000.0);
    ASSERT_EQ(w.getTransactions().size(), std::size_t(0));
}

// Kiem tra constructor voi so du mac dinh = 0
TEST_CASE(test_wallet_constructor_zero_balance) {
    Wallet w("bob");
    ASSERT_EQ(w.getBalance(), 0.0);
}

// Kiem tra constructor voi so du am phai throw
TEST_CASE(test_wallet_negative_initial_balance_throws) {
    ASSERT_THROW(Wallet("charlie", -1.0), InvalidInputException);
}

// Kiem tra addIncome tang so du va tao transaction dung
TEST_CASE(test_wallet_addIncome_basic) {
    Wallet w("alice", 0.0);
    auto txn = w.addIncome(1000000.0, "Luong", "Luong thang 6", "2026-06-01");
    ASSERT_EQ(w.getBalance(), 1000000.0);
    ASSERT_EQ(w.getTransactions().size(), std::size_t(1));
    ASSERT_TRUE(txn != nullptr);
    ASSERT_EQ(txn->getType(), TransactionType::INCOME);
    ASSERT_EQ(txn->getAmount(), 1000000.0);
}

// Kiem tra addExpense giam so du dung
TEST_CASE(test_wallet_addExpense_basic) {
    Wallet w("alice", 1000000.0);
    w.addExpense(200000.0, "An uong", "An trua", "2026-06-01");
    ASSERT_EQ(w.getBalance(), 800000.0);
    ASSERT_EQ(w.getTransactions().size(), std::size_t(1));
}

// Kiem tra addExpense vuot qua so du phai throw InsufficientBalanceException
TEST_CASE(test_wallet_addExpense_insufficient_balance_throws) {
    Wallet w("alice", 100.0);
    ASSERT_THROW(
        w.addExpense(101.0, "An uong", "An trua", "2026-06-01"),
        InsufficientBalanceException
    );
    // So du khong doi khi throw
    ASSERT_EQ(w.getBalance(), 100.0);
}

// Kiem tra addIncome voi amount <= 0 phai throw
TEST_CASE(test_wallet_addIncome_nonpositive_throws) {
    Wallet w("alice", 0.0);
    ASSERT_THROW(w.addIncome(0.0,  "Luong", "Test", "2026-01-01"), InvalidInputException);
    ASSERT_THROW(w.addIncome(-1.0, "Luong", "Test", "2026-01-01"), InvalidInputException);
}

// Kiem tra addExpense voi amount <= 0 phai throw
TEST_CASE(test_wallet_addExpense_nonpositive_throws) {
    Wallet w("alice", 1000.0);
    ASSERT_THROW(w.addExpense(0.0,  "An uong", "Test", "2026-01-01"), InvalidInputException);
    ASSERT_THROW(w.addExpense(-1.0, "An uong", "Test", "2026-01-01"), InvalidInputException);
}

// Kiem tra totalIncome va totalExpense tinh dung
TEST_CASE(test_wallet_totals_correct) {
    Wallet w("alice", 0.0);
    w.addIncome( 5000000.0, "Luong",   "Luong",   "2026-06-01");
    w.addIncome( 1000000.0, "Bonus",   "Thuong",  "2026-06-05");
    w.addExpense( 200000.0, "An uong", "An trua", "2026-06-10");
    w.addExpense( 300000.0, "Di lai",  "Xang xe", "2026-06-15");

    ASSERT_EQ(w.totalIncome(),  6000000.0);
    ASSERT_EQ(w.totalExpense(),  500000.0);
    ASSERT_EQ(w.getBalance(),   5500000.0);
}

// Kiem tra loadTransaction (khoi phuc tu file, khong validate balance)
TEST_CASE(test_wallet_loadTransaction_income) {
    Wallet w("alice", 0.0);
    Transaction t(1, "2026-01-01", "Luong", 3000000.0, TransactionType::INCOME, "Luong");
    w.loadTransaction(t);
    ASSERT_EQ(w.getBalance(), 3000000.0);
    ASSERT_EQ(w.getTransactions().size(), std::size_t(1));
}

// Kiem tra loadTransaction voi EXPENSE giam so du (ke ca khi so du am)
TEST_CASE(test_wallet_loadTransaction_expense) {
    Wallet w("alice", 0.0);
    Transaction t(1, "2026-01-01", "Mua hang", 500000.0, TransactionType::EXPENSE, "Mua sam");
    w.loadTransaction(t); // Khong throw du so du = 0 (load khong validate)
    ASSERT_EQ(w.getBalance(), -500000.0);
}

// Kiem tra operator<< in thong tin vi ra ostream
TEST_CASE(test_wallet_ostream_operator) {
    Wallet w("testuser", 500000.0);
    std::ostringstream oss;
    oss << w;
    std::string out = oss.str();
    ASSERT_TRUE(out.find("testuser")  != std::string::npos);
    ASSERT_TRUE(out.find("500000")    != std::string::npos);
}

// Kiem tra nhieu giao dich lien tiep, balance chinh xac
TEST_CASE(test_wallet_multiple_transactions_balance) {
    Wallet w("user", 2000000.0);
    w.addIncome(  500000.0, "Bonus",   "Bonus", "2026-06-01");
    w.addExpense( 100000.0, "Di lai",  "Taxi",  "2026-06-02");
    w.addExpense(  50000.0, "An uong", "Cafe",  "2026-06-03");

    ASSERT_EQ(w.getTransactions().size(), std::size_t(3));
    ASSERT_EQ(w.getBalance(), 2000000.0 + 500000.0 - 100000.0 - 50000.0);
}

// --- Ham chay toan bo Wallet tests ---
void runWalletTests() {
    std::cout << "\n--- Wallet Tests ---\n";
    RUN_TEST(test_wallet_constructor_basic);
    RUN_TEST(test_wallet_constructor_zero_balance);
    RUN_TEST(test_wallet_negative_initial_balance_throws);
    RUN_TEST(test_wallet_addIncome_basic);
    RUN_TEST(test_wallet_addExpense_basic);
    RUN_TEST(test_wallet_addExpense_insufficient_balance_throws);
    RUN_TEST(test_wallet_addIncome_nonpositive_throws);
    RUN_TEST(test_wallet_addExpense_nonpositive_throws);
    RUN_TEST(test_wallet_totals_correct);
    RUN_TEST(test_wallet_loadTransaction_income);
    RUN_TEST(test_wallet_loadTransaction_expense);
    RUN_TEST(test_wallet_ostream_operator);
    RUN_TEST(test_wallet_multiple_transactions_balance);
}
