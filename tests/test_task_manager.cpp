#include "test_framework.h"
#include "../include/task/task_manager.h"
#include "../include/core/exceptions.h"
#include <memory>

using namespace opep;

// Helper tao task nhanh, tranh lap code
static std::unique_ptr<Task> makeTask(int id,
                                      const std::string& title,
                                      Priority p = Priority::MEDIUM)
{
    return std::make_unique<Task>(id, "2026-01-01", title, p);
}

// Kiem tra addTask va count tang dung so luong
TEST_CASE(test_tm_add_and_count) {
    TaskManager tm;
    ASSERT_EQ(tm.count(), std::size_t(0));
    tm.addTask(makeTask(1, "Task 1"));
    tm.addTask(makeTask(2, "Task 2"));
    ASSERT_EQ(tm.count(), std::size_t(2));
}

// Kiem tra addTask voi unique_ptr null phai throw
TEST_CASE(test_tm_add_null_throws) {
    TaskManager tm;
    ASSERT_THROW(tm.addTask(nullptr), InvalidInputException);
}

// Kiem tra addTask voi id trung phai throw
TEST_CASE(test_tm_add_duplicate_id_throws) {
    TaskManager tm;
    tm.addTask(makeTask(1, "Task A"));
    ASSERT_THROW(tm.addTask(makeTask(1, "Task B")), InvalidInputException);
}

// Kiem tra getById tra ve dung task khi co
TEST_CASE(test_tm_getById_found) {
    TaskManager tm;
    tm.addTask(makeTask(10, "Find Me", Priority::HIGH));
    const Task* t = tm.getById(10);
    ASSERT_TRUE(t != nullptr);
    ASSERT_EQ(t->getId(), 10);
    ASSERT_EQ(t->getTitle(), std::string("Find Me"));
    ASSERT_EQ(t->getPriority(), Priority::HIGH);
}

// Kiem tra getById tra ve nullptr khi khong co
TEST_CASE(test_tm_getById_not_found) {
    TaskManager tm;
    ASSERT_TRUE(tm.getById(999) == nullptr);
}

// Kiem tra getById sau khi them nhieu task
TEST_CASE(test_tm_getById_among_multiple) {
    TaskManager tm;
    tm.addTask(makeTask(1, "T1"));
    tm.addTask(makeTask(2, "T2"));
    tm.addTask(makeTask(3, "T3"));
    ASSERT_EQ(tm.getById(2)->getTitle(), std::string("T2"));
}

// Kiem tra removeTask xoa phan tu dung
TEST_CASE(test_tm_removeTask_correct_element) {
    TaskManager tm;
    tm.addTask(makeTask(1, "Task 1"));
    tm.addTask(makeTask(2, "Task 2"));
    tm.removeTask(1);
    ASSERT_EQ(tm.count(), std::size_t(1));
    ASSERT_TRUE(tm.getById(1) == nullptr);
    ASSERT_TRUE(tm.getById(2) != nullptr);
}

// Kiem tra removeTask voi id khong ton tai phai throw
TEST_CASE(test_tm_removeTask_not_found_throws) {
    TaskManager tm;
    ASSERT_THROW(tm.removeTask(99), InvalidInputException);
}

// Kiem tra markDone doi trang thai completed → true
TEST_CASE(test_tm_markDone_sets_completed) {
    TaskManager tm;
    tm.addTask(makeTask(1, "Task"));
    ASSERT_FALSE(tm.getById(1)->isCompleted());
    tm.markDone(1);
    ASSERT_TRUE(tm.getById(1)->isCompleted());
}

// Kiem tra markDone voi id khong ton tai phai throw
TEST_CASE(test_tm_markDone_not_found_throws) {
    TaskManager tm;
    ASSERT_THROW(tm.markDone(55), InvalidInputException);
}

// Kiem tra sortByPriority sap xep dung: HIGH -> MEDIUM -> LOW
TEST_CASE(test_tm_sortByPriority_order) {
    TaskManager tm;
    tm.addTask(makeTask(1, "Low",    Priority::LOW));
    tm.addTask(makeTask(2, "High",   Priority::HIGH));
    tm.addTask(makeTask(3, "Medium", Priority::MEDIUM));
    tm.sortByPriority();

    const auto& all = tm.getAll();
    ASSERT_EQ(all[0]->getPriority(), Priority::HIGH);
    ASSERT_EQ(all[1]->getPriority(), Priority::MEDIUM);
    ASSERT_EQ(all[2]->getPriority(), Priority::LOW);
}

// Kiem tra sortByPriority la stable sort (giu thu tu ban dau voi cung priority)
TEST_CASE(test_tm_sortByPriority_stable) {
    TaskManager tm;
    tm.addTask(makeTask(1, "First HIGH",  Priority::HIGH));
    tm.addTask(makeTask(2, "Second HIGH", Priority::HIGH));
    tm.addTask(makeTask(3, "Third HIGH",  Priority::HIGH));
    tm.sortByPriority();

    const auto& all = tm.getAll();
    // Stable sort: thu tu ban dau duoc giu nguyen
    ASSERT_EQ(all[0]->getId(), 1);
    ASSERT_EQ(all[1]->getId(), 2);
    ASSERT_EQ(all[2]->getId(), 3);
}

// Kiem tra countCompleted dem dung so task da hoan thanh
TEST_CASE(test_tm_countCompleted) {
    TaskManager tm;
    tm.addTask(makeTask(1, "T1"));
    tm.addTask(makeTask(2, "T2"));
    tm.addTask(makeTask(3, "T3"));
    ASSERT_EQ(tm.countCompleted(), std::size_t(0));
    tm.markDone(1);
    tm.markDone(3);
    ASSERT_EQ(tm.countCompleted(), std::size_t(2));
    ASSERT_EQ(tm.count(),          std::size_t(3));
}

// Kiem tra getAll tra ve tham chieu read-only toi danh sach
TEST_CASE(test_tm_getAll_size_matches_count) {
    TaskManager tm;
    tm.addTask(makeTask(1, "A"));
    tm.addTask(makeTask(2, "B"));
    ASSERT_EQ(tm.getAll().size(), tm.count());
}

// --- Ham chay toan bo TaskManager tests ---
void runTaskManagerTests() {
    std::cout << "\n--- TaskManager Tests ---\n";
    RUN_TEST(test_tm_add_and_count);
    RUN_TEST(test_tm_add_null_throws);
    RUN_TEST(test_tm_add_duplicate_id_throws);
    RUN_TEST(test_tm_getById_found);
    RUN_TEST(test_tm_getById_not_found);
    RUN_TEST(test_tm_getById_among_multiple);
    RUN_TEST(test_tm_removeTask_correct_element);
    RUN_TEST(test_tm_removeTask_not_found_throws);
    RUN_TEST(test_tm_markDone_sets_completed);
    RUN_TEST(test_tm_markDone_not_found_throws);
    RUN_TEST(test_tm_sortByPriority_order);
    RUN_TEST(test_tm_sortByPriority_stable);
    RUN_TEST(test_tm_countCompleted);
    RUN_TEST(test_tm_getAll_size_matches_count);
}
