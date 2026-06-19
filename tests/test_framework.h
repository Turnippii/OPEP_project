#ifndef OPEP_TEST_FRAMEWORK_H
#define OPEP_TEST_FRAMEWORK_H

#include <iostream>
#include <sstream>
#include <string>
#include <exception>

// --- Counters toan cuc, dung ham static local de tranh ODR violation ---
inline int& testTotal()  { static int n = 0; return n; }
inline int& testPassed() { static int n = 0; return n; }
inline int& testFailed() { static int n = 0; return n; }

// --- Exception noi bo cua framework ---
// Duoc nem khi ASSERT_* that bai, bi bat tai RUN_TEST
class AssertionFailure : public std::exception {
    std::string msg;
public:
    explicit AssertionFailure(const std::string& m) : msg(m) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

// --- Khai bao test case ---
// TEST_CASE(ten) ~ void ten()
#define TEST_CASE(name) void name()

// --- Chay mot test va cap nhat counter ---
#define RUN_TEST(name) \
    do { \
        ++testTotal(); \
        try { \
            name(); \
            ++testPassed(); \
            std::cout << "  [PASS] " #name "\n"; \
        } catch (const AssertionFailure& _af) { \
            ++testFailed(); \
            std::cout << "  [FAIL] " #name ": " << _af.what() << "\n"; \
        } catch (const std::exception& _ex) { \
            ++testFailed(); \
            std::cout << "  [FAIL] " #name " (exception bat ngo): " \
                      << _ex.what() << "\n"; \
        } catch (...) { \
            ++testFailed(); \
            std::cout << "  [FAIL] " #name " (exception khong xac dinh)\n"; \
        } \
    } while(0)

// --- ASSERT_EQ: kiem tra a == b ---
#define ASSERT_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            std::ostringstream _oss; \
            _oss << #a " != " #b " (dong " << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } \
    } while(0)

// --- ASSERT_NE: kiem tra a != b ---
#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::ostringstream _oss; \
            _oss << #a " == " #b " nhung mong khac nhau (dong " \
                 << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } \
    } while(0)

// --- ASSERT_TRUE: kiem tra bieu thuc la true ---
#define ASSERT_TRUE(x) \
    do { \
        if (!(x)) { \
            std::ostringstream _oss; \
            _oss << "ASSERT_TRUE(" #x ") that bai (dong " << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } \
    } while(0)

// --- ASSERT_FALSE: kiem tra bieu thuc la false ---
#define ASSERT_FALSE(x) \
    do { \
        if (x) { \
            std::ostringstream _oss; \
            _oss << "ASSERT_FALSE(" #x ") that bai (dong " << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } \
    } while(0)

// --- ASSERT_THROW: dam bao bieu thuc nem dung loai exception ---
#define ASSERT_THROW(expr, ExcType) \
    do { \
        bool _threw = false; \
        try { \
            (void)(expr); \
        } catch (const ExcType&) { \
            _threw = true; \
        } catch (...) {} \
        if (!_threw) { \
            std::ostringstream _oss; \
            _oss << #expr " khong nem " #ExcType " (dong " << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } \
    } while(0)

// --- ASSERT_NO_THROW: dam bao bieu thuc khong nem exception ---
#define ASSERT_NO_THROW(expr) \
    do { \
        try { \
            (void)(expr); \
        } catch (const std::exception& _ex) { \
            std::ostringstream _oss; \
            _oss << #expr " nem exception bat ngo: " << _ex.what() \
                 << " (dong " << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } catch (...) { \
            std::ostringstream _oss; \
            _oss << #expr " nem exception khong xac dinh (dong " \
                 << __LINE__ << ")"; \
            throw AssertionFailure(_oss.str()); \
        } \
    } while(0)

// --- In tong ket sau khi chay het test ---
inline void printTestSummary() {
    int total  = testTotal();
    int passed = testPassed();
    int failed = testFailed();
    int pct    = (total > 0) ? (100 * passed / total) : 0;

    std::cout << "\n=========================================\n";
    std::cout << "  TONG KET: " << passed << "/" << total
              << " test PASS (" << pct << "%)\n";
    if (failed > 0)
        std::cout << "  THAT BAI:  " << failed << " test\n";
    std::cout << "=========================================\n";
}

#endif // OPEP_TEST_FRAMEWORK_H
