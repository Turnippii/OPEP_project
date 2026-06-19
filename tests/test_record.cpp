#include "test_framework.h"
#include "../include/core/record.h"
#include "../include/core/exceptions.h"
#include <sstream>

using namespace opep;

// Lop con cu the de test Record (vi Record la abstract)
class ConcreteRecord : public Record {
public:
    ConcreteRecord(int id, const std::string& date, const std::string& title)
        : Record(id, date, title) {}
    void displayInfo() const override {} // Khong lam gi trong test
};

// Kiem tra constructor khoi tao dung id, date, title
TEST_CASE(test_record_constructor_basic) {
    ConcreteRecord r(1, "2026-01-01", "Test Record");
    ASSERT_EQ(r.getId(), 1);
    ASSERT_EQ(r.getDate(), std::string("2026-01-01"));
    ASSERT_EQ(r.getTitle(), std::string("Test Record"));
}

// Kiem tra constructor voi id am (hop le — Record khong gioi han id)
TEST_CASE(test_record_constructor_negative_id) {
    ASSERT_NO_THROW(ConcreteRecord(-1, "2026-01-01", "Negative ID"));
}

// Kiem tra constructor voi tieu de rong phai throw InvalidInputException
TEST_CASE(test_record_empty_title_throws) {
    ASSERT_THROW(ConcreteRecord(1, "2026-01-01", ""), InvalidInputException);
}

// Kiem tra operator== so sanh chi dua tren id
TEST_CASE(test_record_operator_eq_same_id_diff_content) {
    ConcreteRecord r1(5, "2026-01-01", "Task A");
    ConcreteRecord r2(5, "2026-12-31", "Task B"); // Khac date/title, cung id
    ASSERT_TRUE(r1 == r2);
}

// Kiem tra operator== voi id khac nhau → khong bang
TEST_CASE(test_record_operator_eq_diff_id) {
    ConcreteRecord r1(1, "2026-01-01", "Same Title");
    ConcreteRecord r2(2, "2026-01-01", "Same Title");
    ASSERT_FALSE(r1 == r2);
}

// Kiem tra setTitle voi gia tri hop le
TEST_CASE(test_record_setTitle_valid) {
    ConcreteRecord r(1, "2026-01-01", "Old");
    ASSERT_NO_THROW(r.setTitle("New Title"));
    ASSERT_EQ(r.getTitle(), std::string("New Title"));
}

// Kiem tra setTitle voi chuoi rong phai throw
TEST_CASE(test_record_setTitle_empty_throws) {
    ConcreteRecord r(1, "2026-01-01", "Title");
    ASSERT_THROW(r.setTitle(""), InvalidInputException);
}

// Kiem tra setDate voi gia tri hop le
TEST_CASE(test_record_setDate_valid) {
    ConcreteRecord r(1, "2026-01-01", "Title");
    r.setDate("2026-12-31");
    ASSERT_EQ(r.getDate(), std::string("2026-12-31"));
}

// Kiem tra setDate voi chuoi rong phai throw
TEST_CASE(test_record_setDate_empty_throws) {
    ConcreteRecord r(1, "2026-01-01", "Title");
    ASSERT_THROW(r.setDate(""), InvalidInputException);
}

// Kiem tra operator<< in ra dung dinh dang (co id, date, title)
TEST_CASE(test_record_ostream_operator) {
    ConcreteRecord r(3, "2026-05-15", "Hello World");
    std::ostringstream oss;
    oss << static_cast<const Record&>(r);
    std::string out = oss.str();
    ASSERT_TRUE(out.find("3")          != std::string::npos);
    ASSERT_TRUE(out.find("2026-05-15") != std::string::npos);
    ASSERT_TRUE(out.find("Hello World") != std::string::npos);
}

// --- Ham chay toan bo Record tests ---
void runRecordTests() {
    std::cout << "\n--- Record Tests ---\n";
    RUN_TEST(test_record_constructor_basic);
    RUN_TEST(test_record_constructor_negative_id);
    RUN_TEST(test_record_empty_title_throws);
    RUN_TEST(test_record_operator_eq_same_id_diff_content);
    RUN_TEST(test_record_operator_eq_diff_id);
    RUN_TEST(test_record_setTitle_valid);
    RUN_TEST(test_record_setTitle_empty_throws);
    RUN_TEST(test_record_setDate_valid);
    RUN_TEST(test_record_setDate_empty_throws);
    RUN_TEST(test_record_ostream_operator);
}
