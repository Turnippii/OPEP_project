#include "test_framework.h"
#include "../include/core/record.h"
#include "../include/task/task.h"
#include "../include/money/transaction.h"
#include "../include/core/exceptions.h"
#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

using namespace opep;

// RAII suppress stdout de displayInfo khong lam ban output test
struct SuppressStdout {
    std::ostringstream buf;
    std::streambuf*    old;
    SuppressStdout()  { old = std::cout.rdbuf(buf.rdbuf()); }
    ~SuppressStdout() { std::cout.rdbuf(old); }
    std::string captured() const { return buf.str(); }
};

// Kiem tra vector<unique_ptr<Record>> co the chua ca Task lan Transaction
TEST_CASE(test_poly_vector_mixed_types) {
    std::vector<std::unique_ptr<Record>> records;
    records.push_back(std::make_unique<Task>(1, "2026-01-01", "Task 1"));
    records.push_back(std::make_unique<Transaction>(
        2, "2026-01-02", "Luong", 5000000.0, TransactionType::INCOME, "Luong"));

    ASSERT_EQ(records.size(), std::size_t(2));
    ASSERT_EQ(records[0]->getId(), 1);
    ASSERT_EQ(records[1]->getId(), 2);
}

// Kiem tra displayInfo duoc dispatch dung kieu (polymorphism)
TEST_CASE(test_poly_displayInfo_dispatch) {
    std::vector<std::unique_ptr<Record>> records;
    records.push_back(std::make_unique<Task>(1, "2026-01-01", "Task A", Priority::HIGH));
    records.push_back(std::make_unique<Transaction>(
        2, "2026-01-01", "Income", 1000.0, TransactionType::INCOME, "Luong"));

    // Task::displayInfo in "[x]" hoac "[ ]" va priority
    // Transaction::displayInfo in "[INCOME]" hoac "[EXPENSE]" va so tien
    {
        SuppressStdout s;
        records[0]->displayInfo();
        std::string out = s.captured();
        // Task in priority [HIGH]
        ASSERT_TRUE(out.find("HIGH") != std::string::npos);
    }
    {
        SuppressStdout s;
        records[1]->displayInfo();
        std::string out = s.captured();
        // Transaction in loai giao dich
        ASSERT_TRUE(out.find("INCOME") != std::string::npos);
    }
}

// Kiem tra dynamic_cast xac dinh dung kieu runtime
TEST_CASE(test_poly_dynamic_cast) {
    std::unique_ptr<Record> r1 = std::make_unique<Task>(1, "2026-01-01", "My Task");
    std::unique_ptr<Record> r2 = std::make_unique<Transaction>(
        2, "2026-01-01", "My Txn", 100.0, TransactionType::EXPENSE, "Test");

    // r1 la Task
    ASSERT_TRUE(dynamic_cast<Task*>(r1.get())        != nullptr);
    ASSERT_TRUE(dynamic_cast<Transaction*>(r1.get()) == nullptr);

    // r2 la Transaction
    ASSERT_TRUE(dynamic_cast<Transaction*>(r2.get()) != nullptr);
    ASSERT_TRUE(dynamic_cast<Task*>(r2.get())        == nullptr);
}

// Kiem tra operator== tu Record co the so sanh qua con tro base class
TEST_CASE(test_poly_operator_eq_via_base) {
    std::unique_ptr<Record> r1 = std::make_unique<Task>(5, "2026-01-01", "Task");
    std::unique_ptr<Record> r2 = std::make_unique<Transaction>(
        5, "2026-01-01", "Txn", 100.0, TransactionType::INCOME, "C");

    // Hai doi tuong khac loai nhung cung id → operator== (Record) tra true
    ASSERT_TRUE(*r1 == *r2);
}

// Kiem tra goi displayInfo tren toan bo vector (khong crash, khong rong)
TEST_CASE(test_poly_displayInfo_all_records) {
    std::vector<std::unique_ptr<Record>> records;
    records.push_back(std::make_unique<Task>(1, "2026-01-01", "Task 1", Priority::LOW));
    records.push_back(std::make_unique<Task>(2, "2026-01-02", "Task 2", Priority::HIGH));
    records.push_back(std::make_unique<Transaction>(
        3, "2026-01-03", "Income 1", 5000.0, TransactionType::INCOME, "Luong"));
    records.push_back(std::make_unique<Transaction>(
        4, "2026-01-04", "Expense 1", 1000.0, TransactionType::EXPENSE, "An uong"));

    SuppressStdout s;
    // Goi displayInfo tren toan bo vector; neu throw thi RUN_TEST se bat va fail
    for (const auto& r : records) r->displayInfo();
    std::string out = s.captured();
    ASSERT_FALSE(out.empty()); // phai co output
}

// Kiem tra shared_ptr<Record> cung ho tro polymorphism
TEST_CASE(test_poly_shared_ptr_base) {
    std::vector<std::shared_ptr<Record>> records;
    records.push_back(std::make_shared<Task>(10, "2026-01-01", "Shared Task"));
    records.push_back(std::make_shared<Transaction>(
        20, "2026-01-01", "Shared Txn", 500.0, TransactionType::INCOME, "Bonus"));

    ASSERT_EQ(records[0]->getTitle(), std::string("Shared Task"));
    ASSERT_EQ(records[1]->getTitle(), std::string("Shared Txn"));

    // Cast ve kieu cu the de truy cap truong rieng
    auto task = std::dynamic_pointer_cast<Task>(records[0]);
    ASSERT_TRUE(task != nullptr);
    ASSERT_EQ(task->getPriority(), Priority::MEDIUM); // default

    auto txn = std::dynamic_pointer_cast<Transaction>(records[1]);
    ASSERT_TRUE(txn != nullptr);
    ASSERT_EQ(txn->getAmount(), 500.0);
}

// Kiem tra virtual destructor giai phong dung (khong leak) khi xoa qua base pointer
TEST_CASE(test_poly_virtual_destructor) {
    // Kiem tra gian tiep: tao + huy qua unique_ptr<Record> khong crash
    ASSERT_NO_THROW({
        std::unique_ptr<Record> p = std::make_unique<Task>(
            99, "2026-01-01", "Destructor test", Priority::HIGH, "2026-12-31");
        // unique_ptr tu dong huy khi het scope — goi ~Task() qua ~Record()
    });
    ASSERT_NO_THROW({
        std::unique_ptr<Record> p = std::make_unique<Transaction>(
            98, "2026-01-01", "Txn dest", 100.0, TransactionType::EXPENSE, "Cat");
    });
}

// Kiem tra getTitle() va getDate() truy cap qua con tro base class
TEST_CASE(test_poly_base_getters) {
    Record* r = new Task(7, "2026-06-19", "Graduation Day", Priority::HIGH);
    ASSERT_EQ(r->getId(),    7);
    ASSERT_EQ(r->getDate(),  std::string("2026-06-19"));
    ASSERT_EQ(r->getTitle(), std::string("Graduation Day"));
    delete r; // Virtual destructor dam bao goi ~Task()
}

// --- Ham chay toan bo Polymorphism tests ---
void runPolymorphismTests() {
    std::cout << "\n--- Polymorphism Tests ---\n";
    RUN_TEST(test_poly_vector_mixed_types);
    RUN_TEST(test_poly_displayInfo_dispatch);
    RUN_TEST(test_poly_dynamic_cast);
    RUN_TEST(test_poly_operator_eq_via_base);
    RUN_TEST(test_poly_displayInfo_all_records);
    RUN_TEST(test_poly_shared_ptr_base);
    RUN_TEST(test_poly_virtual_destructor);
    RUN_TEST(test_poly_base_getters);
}
