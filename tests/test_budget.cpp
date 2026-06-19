#include "test_framework.h"
#include "../include/money/budget.h"
#include "../include/core/exceptions.h"
#include <memory>
#include <vector>

using namespace opep;

// Kiem tra constructor khoi tao thang dung, chua co category
TEST_CASE(test_budget_constructor) {
    Budget b("2026-06");
    ASSERT_EQ(b.getMonth(), std::string("2026-06"));
    ASSERT_EQ(b.totalLimit(), 0.0);
    ASSERT_EQ(b.totalSpent(), 0.0);
}

// Kiem tra setLimit va getCategory lay dung thong tin
TEST_CASE(test_budget_setLimit_and_get) {
    Budget b("2026-06");
    b.setLimit("An uong", 2000000.0);
    const CategoryBudget& cb = b.getCategory("An uong");
    ASSERT_EQ(cb.limit, 2000000.0);
    ASSERT_EQ(cb.spent, 0.0);
}

// Kiem tra setLimit ghi de khi goi lai voi cung category
TEST_CASE(test_budget_setLimit_overwrite) {
    Budget b("2026-06");
    b.setLimit("Di lai", 1000000.0);
    b.setLimit("Di lai", 1500000.0); // Ghi de
    ASSERT_EQ(b.getCategory("Di lai").limit, 1500000.0);
}

// Kiem tra setLimit voi gia tri am phai throw
TEST_CASE(test_budget_setLimit_negative_throws) {
    Budget b("2026-06");
    ASSERT_THROW(b.setLimit("An uong", -500.0), InvalidInputException);
}

// Kiem tra getCategory voi danh muc khong ton tai phai throw
TEST_CASE(test_budget_getCategory_not_found_throws) {
    Budget b("2026-06");
    ASSERT_THROW(b.getCategory("Khong ton tai"), InvalidInputException);
}

// Kiem tra recordExpense cap nhat spent dung
TEST_CASE(test_budget_recordExpense_updates_spent) {
    Budget b("2026-06");
    b.setLimit("An uong", 2000000.0);
    b.recordExpense("An uong", 500000.0);
    ASSERT_EQ(b.getCategory("An uong").spent, 500000.0);
}

// Kiem tra recordExpense vuot han muc phai throw BudgetExceededException
TEST_CASE(test_budget_recordExpense_exceeded_throws) {
    Budget b("2026-06");
    b.setLimit("An uong", 500000.0);
    ASSERT_THROW(b.recordExpense("An uong", 600000.0), BudgetExceededException);
}

// Kiem tra recordExpense cho category chua co limit (limit = 0, khong throw)
TEST_CASE(test_budget_recordExpense_no_limit_ok) {
    Budget b("2026-06");
    // Khong goi setLimit — category tu dong tao voi limit = 0
    ASSERT_NO_THROW(b.recordExpense("Khac", 999999.0));
}

// Kiem tra CategoryBudget::isExceeded chi true khi spent > limit > 0
TEST_CASE(test_category_isExceeded) {
    CategoryBudget cb("Test", 1000.0);
    cb.spent = 1001.0;
    ASSERT_TRUE(cb.isExceeded());
    cb.spent = 1000.0; // Dung bang limit → chua vuot
    ASSERT_FALSE(cb.isExceeded());
    cb.spent = 999.0;
    ASSERT_FALSE(cb.isExceeded());
    // Limit = 0 → khong bao gio vuot
    CategoryBudget cb2("Test2", 0.0);
    cb2.spent = 9999.0;
    ASSERT_FALSE(cb2.isExceeded());
}

// Kiem tra CategoryBudget::isNearLimit (>= 80% han muc)
TEST_CASE(test_category_isNearLimit) {
    CategoryBudget cb("Test", 1000000.0);
    cb.spent = 800000.0; // 80% → canh bao
    ASSERT_TRUE(cb.isNearLimit());
    cb.spent = 799999.0; // < 80% → khong canh bao
    ASSERT_FALSE(cb.isNearLimit());
    cb.spent = 1100000.0; // vuot 110% → canh bao (da vuot thi cung isNear)
    ASSERT_TRUE(cb.isNearLimit());
}

// Kiem tra normalizeKey: hoa/thuong, khoang trang dau cuoi, collapse spaces
TEST_CASE(test_budget_category_normalize) {
    Budget b("2026-06");
    b.setLimit("An Uong", 1000000.0);      // key: "an uong"
    b.recordExpense("  an uong  ", 200000.0); // khop voi "an uong"
    ASSERT_EQ(b.getCategory("AN UONG").spent, 200000.0);
}

// Kiem tra totalLimit va totalSpent tong hop nhieu category
TEST_CASE(test_budget_totals) {
    Budget b("2026-06");
    b.setLimit("An uong", 2000000.0);
    b.setLimit("Di lai",  1000000.0);
    b.recordExpense("An uong", 500000.0);
    b.recordExpense("Di lai",  300000.0);

    ASSERT_EQ(b.totalLimit(), 3000000.0);
    ASSERT_EQ(b.totalSpent(),  800000.0);
}

// Kiem tra syncFromTransactions tinh lai spent tu danh sach giao dich
TEST_CASE(test_budget_syncFromTransactions) {
    Budget b("2026-06");
    b.setLimit("An uong", 2000000.0);

    std::vector<std::shared_ptr<Transaction>> txns;
    // Duoc tinh: EXPENSE, dung thang 2026-06
    txns.push_back(std::make_shared<Transaction>(
        1, "2026-06-01", "Com trua", 50000.0, TransactionType::EXPENSE, "An uong"));
    txns.push_back(std::make_shared<Transaction>(
        2, "2026-06-02", "Pho bo",   80000.0, TransactionType::EXPENSE, "An uong"));
    // Khong tinh: sai thang
    txns.push_back(std::make_shared<Transaction>(
        3, "2026-05-30", "Thang truoc", 100000.0, TransactionType::EXPENSE, "An uong"));
    // Khong tinh: la INCOME
    txns.push_back(std::make_shared<Transaction>(
        4, "2026-06-10", "Luong", 5000000.0, TransactionType::INCOME, "Luong"));

    b.syncFromTransactions(txns);
    ASSERT_EQ(b.getCategory("An uong").spent, 130000.0);
}

// Kiem tra syncFromTransactions dat lai spent tu 0 truoc khi tinh
TEST_CASE(test_budget_syncFromTransactions_resets_spent) {
    Budget b("2026-06");
    b.setLimit("An uong", 2000000.0);
    b.recordExpense("An uong", 500000.0); // spent = 500000 truoc khi sync

    std::vector<std::shared_ptr<Transaction>> txns;
    txns.push_back(std::make_shared<Transaction>(
        1, "2026-06-01", "Com", 100000.0, TransactionType::EXPENSE, "An uong"));

    b.syncFromTransactions(txns);
    // Sau sync, spent duoc tinh lai tu txns (khong cong don)
    ASSERT_EQ(b.getCategory("An uong").spent, 100000.0);
}

// --- Ham chay toan bo Budget tests ---
void runBudgetTests() {
    std::cout << "\n--- Budget Tests ---\n";
    RUN_TEST(test_budget_constructor);
    RUN_TEST(test_budget_setLimit_and_get);
    RUN_TEST(test_budget_setLimit_overwrite);
    RUN_TEST(test_budget_setLimit_negative_throws);
    RUN_TEST(test_budget_getCategory_not_found_throws);
    RUN_TEST(test_budget_recordExpense_updates_spent);
    RUN_TEST(test_budget_recordExpense_exceeded_throws);
    RUN_TEST(test_budget_recordExpense_no_limit_ok);
    RUN_TEST(test_category_isExceeded);
    RUN_TEST(test_category_isNearLimit);
    RUN_TEST(test_budget_category_normalize);
    RUN_TEST(test_budget_totals);
    RUN_TEST(test_budget_syncFromTransactions);
    RUN_TEST(test_budget_syncFromTransactions_resets_spent);
}
