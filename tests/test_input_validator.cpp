#include "test_framework.h"
#include "../include/ui/input_validator.h"
#include "../include/core/exceptions.h"
#include <sstream>
#include <iostream>

using namespace opep;

// RAII helper: doi cin/cout thanh stream gia lap, tu dong khoi phuc khi ra khoi scope
struct IORedirect {
    std::istringstream cinBuf;
    std::ostringstream coutBuf;
    std::streambuf*    oldCin;
    std::streambuf*    oldCout;

    explicit IORedirect(const std::string& cinInput) : cinBuf(cinInput) {
        oldCin  = std::cin.rdbuf(cinBuf.rdbuf());
        oldCout = std::cout.rdbuf(coutBuf.rdbuf());
    }

    ~IORedirect() {
        std::cin.rdbuf(oldCin);
        std::cin.clear(); // Reset eofbit/failbit sau khi stream gia lap can
        std::cout.rdbuf(oldCout);
    }
};

// --- readDate ---

// Kiem tra readDate chap nhan ngay hop le va tra ve dung
TEST_CASE(test_validator_readDate_valid) {
    IORedirect io("2026-05-25\n");
    std::string result = InputValidator::readDate("Date: ");
    ASSERT_EQ(result, std::string("2026-05-25"));
}

// Kiem tra readDate voi ngay dau thang/cuoi thang hop le
TEST_CASE(test_validator_readDate_boundary_days) {
    {
        IORedirect io("2026-01-31\n");
        ASSERT_EQ(InputValidator::readDate(""), std::string("2026-01-31"));
    }
    {
        IORedirect io("2026-04-30\n");
        ASSERT_EQ(InputValidator::readDate(""), std::string("2026-04-30"));
    }
}

// Kiem tra readDate voi allowEmpty=true va input rong tra ve ""
TEST_CASE(test_validator_readDate_allow_empty) {
    IORedirect io("\n"); // Nguoi dung nhan Enter, khong nhap gi
    std::string result = InputValidator::readDate("Date: ", true);
    ASSERT_EQ(result, std::string(""));
}

// Kiem tra readDate tu choi ngay sai dinh dang: chuoi bat ky → loop → EOF → throw
TEST_CASE(test_validator_readDate_invalid_format_throws) {
    IORedirect io("not-a-date\n"); // Sai format, tiep theo la EOF
    ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
}

// Kiem tra readDate tu choi thang khong hop le (13)
TEST_CASE(test_validator_readDate_invalid_month_throws) {
    IORedirect io("2026-13-01\n");
    ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
}

// Kiem tra readDate tu choi thang 0
TEST_CASE(test_validator_readDate_month_zero_throws) {
    IORedirect io("2026-00-01\n");
    ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
}

// Kiem tra readDate tu choi ngay 0
TEST_CASE(test_validator_readDate_day_zero_throws) {
    IORedirect io("2026-01-00\n");
    ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
}

// Kiem tra readDate tu choi ngay 32 trong thang 1
TEST_CASE(test_validator_readDate_day_32_throws) {
    IORedirect io("2026-01-32\n");
    ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
}

// Kiem tra readDate chap nhan 29/02 nam nhuan 2024
TEST_CASE(test_validator_readDate_leap_year_valid) {
    IORedirect io("2024-02-29\n");
    std::string result = InputValidator::readDate("Date: ");
    ASSERT_EQ(result, std::string("2024-02-29"));
}

// Kiem tra readDate tu choi 29/02 nam khong nhuan 2026
TEST_CASE(test_validator_readDate_non_leap_year_throws) {
    IORedirect io("2026-02-29\n");
    ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
}

// Kiem tra readDate tu choi nam ngoai khoang [2000, 2100]
TEST_CASE(test_validator_readDate_year_out_of_range_throws) {
    {
        IORedirect io("1999-12-31\n"); // Nam < 2000
        ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
    }
    {
        IORedirect io("2101-01-01\n"); // Nam > 2100
        ASSERT_THROW(InputValidator::readDate("Date: "), InvalidInputException);
    }
}

// Kiem tra readDate chap nhan nam bien 2000 va 2100
TEST_CASE(test_validator_readDate_boundary_years) {
    {
        IORedirect io("2000-01-01\n");
        ASSERT_EQ(InputValidator::readDate(""), std::string("2000-01-01"));
    }
    {
        IORedirect io("2100-12-31\n");
        ASSERT_EQ(InputValidator::readDate(""), std::string("2100-12-31"));
    }
}

// --- readString ---

// Kiem tra readString tra ve chuoi hop le
TEST_CASE(test_validator_readString_valid) {
    IORedirect io("Hello World\n");
    std::string result = InputValidator::readString("Input: ", 1, 200);
    ASSERT_EQ(result, std::string("Hello World"));
}

// Kiem tra readString voi input qua ngan → loop → input hop le → tra ve
TEST_CASE(test_validator_readString_too_short_then_valid) {
    // "ab" < minLen=3 → bao loi, vong lai; "hello" hop le → tra ve
    IORedirect io("ab\nhello\n");
    std::string result = InputValidator::readString("Input: ", 3, 20);
    ASSERT_EQ(result, std::string("hello"));
}

// Kiem tra readString voi EOF va chuoi qua ngan → throw
TEST_CASE(test_validator_readString_too_short_eof_throws) {
    IORedirect io("ab\n"); // < minLen=5, tiep theo EOF
    ASSERT_THROW(InputValidator::readString("Input: ", 5, 200), InvalidInputException);
}

// --- Ham chay toan bo InputValidator tests ---
void runInputValidatorTests() {
    std::cout << "\n--- InputValidator Tests ---\n";
    RUN_TEST(test_validator_readDate_valid);
    RUN_TEST(test_validator_readDate_boundary_days);
    RUN_TEST(test_validator_readDate_allow_empty);
    RUN_TEST(test_validator_readDate_invalid_format_throws);
    RUN_TEST(test_validator_readDate_invalid_month_throws);
    RUN_TEST(test_validator_readDate_month_zero_throws);
    RUN_TEST(test_validator_readDate_day_zero_throws);
    RUN_TEST(test_validator_readDate_day_32_throws);
    RUN_TEST(test_validator_readDate_leap_year_valid);
    RUN_TEST(test_validator_readDate_non_leap_year_throws);
    RUN_TEST(test_validator_readDate_year_out_of_range_throws);
    RUN_TEST(test_validator_readDate_boundary_years);
    RUN_TEST(test_validator_readString_valid);
    RUN_TEST(test_validator_readString_too_short_then_valid);
    RUN_TEST(test_validator_readString_too_short_eof_throws);
}
