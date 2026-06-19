#include "test_framework.h"
#include "../include/task/task.h"
#include "../include/core/exceptions.h"
#include <sstream>

using namespace opep;

// Kiem tra constructor khoi tao dung gia tri mac dinh
TEST_CASE(test_task_constructor_defaults) {
    Task t(1, "2026-01-01", "Hoc bai");
    ASSERT_EQ(t.getId(), 1);
    ASSERT_EQ(t.getTitle(), std::string("Hoc bai"));
    ASSERT_EQ(t.getDate(), std::string("2026-01-01"));
    ASSERT_EQ(t.getPriority(), Priority::MEDIUM);
    ASSERT_FALSE(t.isCompleted());
    ASSERT_EQ(t.getDeadline(), std::string(""));
}

// Kiem tra constructor voi priority HIGH va deadline cu the
TEST_CASE(test_task_constructor_priority_deadline) {
    Task t(2, "2026-06-01", "Nop bao cao", Priority::HIGH, "2026-06-30");
    ASSERT_EQ(t.getPriority(), Priority::HIGH);
    ASSERT_EQ(t.getDeadline(), std::string("2026-06-30"));
}

// Kiem tra constructor voi priority LOW
TEST_CASE(test_task_constructor_priority_low) {
    Task t(3, "2026-01-01", "Doc sach", Priority::LOW);
    ASSERT_EQ(t.getPriority(), Priority::LOW);
}

// Kiem tra constructor voi tieu de rong phai throw (ke thua tu Record)
TEST_CASE(test_task_empty_title_throws) {
    ASSERT_THROW(Task(1, "2026-01-01", ""), InvalidInputException);
}

// Kiem tra setCompleted thay doi trang thai dung
TEST_CASE(test_task_setCompleted_toggle) {
    Task t(1, "2026-01-01", "Task");
    ASSERT_FALSE(t.isCompleted());
    t.setCompleted(true);
    ASSERT_TRUE(t.isCompleted());
    t.setCompleted(false);
    ASSERT_FALSE(t.isCompleted());
}

// Kiem tra setPriority cap nhat dung gia tri
TEST_CASE(test_task_setPriority) {
    Task t(1, "2026-01-01", "Task");
    t.setPriority(Priority::LOW);
    ASSERT_EQ(t.getPriority(), Priority::LOW);
    t.setPriority(Priority::HIGH);
    ASSERT_EQ(t.getPriority(), Priority::HIGH);
}

// Kiem tra setDeadline cap nhat chuoi deadline
TEST_CASE(test_task_setDeadline) {
    Task t(1, "2026-01-01", "Task");
    t.setDeadline("2026-12-31");
    ASSERT_EQ(t.getDeadline(), std::string("2026-12-31"));
}

// Kiem tra operator== khi tat ca truong giong nhau
TEST_CASE(test_task_operator_eq_identical) {
    Task t1(1, "2026-01-01", "Task", Priority::HIGH, "2026-12-31");
    Task t2(1, "2026-01-01", "Task", Priority::HIGH, "2026-12-31");
    ASSERT_TRUE(t1 == t2);
}

// Kiem tra operator== khi priority khac → khong bang
TEST_CASE(test_task_operator_eq_diff_priority) {
    Task t1(1, "2026-01-01", "Task", Priority::HIGH);
    Task t2(1, "2026-01-01", "Task", Priority::LOW);
    ASSERT_FALSE(t1 == t2);
}

// Kiem tra operator!= khi completed khac nhau
TEST_CASE(test_task_operator_ne_completed) {
    Task t1(1, "2026-01-01", "Task");
    Task t2(1, "2026-01-01", "Task");
    t2.setCompleted(true);
    ASSERT_TRUE(t1 != t2);
}

// Kiem tra priorityToString chuyen dung Priority → string
TEST_CASE(test_task_priorityToString) {
    ASSERT_EQ(priorityToString(Priority::HIGH),   std::string("HIGH"));
    ASSERT_EQ(priorityToString(Priority::MEDIUM), std::string("MEDIUM"));
    ASSERT_EQ(priorityToString(Priority::LOW),    std::string("LOW"));
}

// Kiem tra stringToPriority chuyen dung string → Priority (case-insensitive)
TEST_CASE(test_task_stringToPriority_case_insensitive) {
    ASSERT_EQ(stringToPriority("HIGH"),   Priority::HIGH);
    ASSERT_EQ(stringToPriority("medium"), Priority::MEDIUM);
    ASSERT_EQ(stringToPriority("low"),    Priority::LOW);
    ASSERT_EQ(stringToPriority("High"),   Priority::HIGH);
}

// Kiem tra stringToPriority voi gia tri khong hop le phai throw
TEST_CASE(test_task_stringToPriority_invalid_throws) {
    ASSERT_THROW(stringToPriority("URGENT"),  InvalidInputException);
    ASSERT_THROW(stringToPriority(""),        InvalidInputException);
    ASSERT_THROW(stringToPriority("HIGHEST"), InvalidInputException);
}

// Kiem tra serialization: << roi >> nen phuc hoi dung task goc
TEST_CASE(test_task_serialization_roundtrip) {
    Task original(42, "2026-05-20", "Serialize test", Priority::HIGH, "2026-06-01");
    original.setCompleted(true);

    std::ostringstream oss;
    oss << original;

    Task loaded;
    std::istringstream iss(oss.str());
    iss >> loaded;

    ASSERT_EQ(loaded.getId(), 42);
    ASSERT_EQ(loaded.getTitle(),    std::string("Serialize test"));
    ASSERT_EQ(loaded.getDate(),     std::string("2026-05-20"));
    ASSERT_EQ(loaded.getPriority(), Priority::HIGH);
    ASSERT_TRUE(loaded.isCompleted());
    ASSERT_EQ(loaded.getDeadline(), std::string("2026-06-01"));
}

// Kiem tra serialization voi task chua hoan thanh va deadline rong
TEST_CASE(test_task_serialization_no_deadline) {
    Task original(7, "2026-01-15", "No deadline task", Priority::LOW);

    std::ostringstream oss;
    oss << original;

    Task loaded;
    std::istringstream iss(oss.str());
    iss >> loaded;

    ASSERT_EQ(loaded.getId(), 7);
    ASSERT_FALSE(loaded.isCompleted());
    ASSERT_EQ(loaded.getPriority(), Priority::LOW);
    ASSERT_EQ(loaded.getDeadline(), std::string(""));
}

// --- Ham chay toan bo Task tests ---
void runTaskTests() {
    std::cout << "\n--- Task Tests ---\n";
    RUN_TEST(test_task_constructor_defaults);
    RUN_TEST(test_task_constructor_priority_deadline);
    RUN_TEST(test_task_constructor_priority_low);
    RUN_TEST(test_task_empty_title_throws);
    RUN_TEST(test_task_setCompleted_toggle);
    RUN_TEST(test_task_setPriority);
    RUN_TEST(test_task_setDeadline);
    RUN_TEST(test_task_operator_eq_identical);
    RUN_TEST(test_task_operator_eq_diff_priority);
    RUN_TEST(test_task_operator_ne_completed);
    RUN_TEST(test_task_priorityToString);
    RUN_TEST(test_task_stringToPriority_case_insensitive);
    RUN_TEST(test_task_stringToPriority_invalid_throws);
    RUN_TEST(test_task_serialization_roundtrip);
    RUN_TEST(test_task_serialization_no_deadline);
}
