#include "test_framework.h"
#include "../include/data/auth.h"
#include "../include/core/exceptions.h"
#include <filesystem>
#include <string>

using namespace opep;
namespace fs = std::filesystem;

// Thu muc tam rieng biet cho test, tranh dong den data/ chinh
static const std::string TEST_AUTH_DIR = "build/tests/temp_auth/";

// Xoa toan bo thu muc truoc/sau moi test de dam bao doc lap
static void cleanAuthDir() {
    if (fs::exists(TEST_AUTH_DIR))
        fs::remove_all(TEST_AUTH_DIR);
}

// Kiem tra registerUser tao user thanh cong va tra ve shared_ptr hop le
TEST_CASE(test_auth_register_success) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auto user = auth.registerUser("alice123", "password123");
    ASSERT_TRUE(user != nullptr);
    ASSERT_EQ(user->getUsername(), std::string("alice123"));
    ASSERT_FALSE(user->getPasswordHash().empty());
    cleanAuthDir();
}

// Kiem tra register voi username da ton tai phai throw AuthenticationException
TEST_CASE(test_auth_register_duplicate_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auth.registerUser("alice123", "password123");
    ASSERT_THROW(auth.registerUser("alice123", "other123"), AuthenticationException);
    cleanAuthDir();
}

// Kiem tra register voi username qua ngan (< 3 ky tu) phai throw
TEST_CASE(test_auth_register_short_username_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    ASSERT_THROW(auth.registerUser("ab", "password123"), InvalidInputException);
    cleanAuthDir();
}

// Kiem tra register voi username qua dai (> 20 ky tu) phai throw
TEST_CASE(test_auth_register_long_username_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    ASSERT_THROW(
        auth.registerUser("this_username_is_too_long123", "password123"),
        InvalidInputException
    );
    cleanAuthDir();
}

// Kiem tra register voi username co ky tu dac biet phai throw
TEST_CASE(test_auth_register_invalid_chars_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    ASSERT_THROW(auth.registerUser("alice@123", "password123"), InvalidInputException);
    ASSERT_THROW(auth.registerUser("alice-user", "password123"), InvalidInputException);
    ASSERT_THROW(auth.registerUser("alice user", "password123"), InvalidInputException);
    cleanAuthDir();
}

// Kiem tra register voi password qua ngan (< 6 ky tu) phai throw
TEST_CASE(test_auth_register_short_password_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    ASSERT_THROW(auth.registerUser("alice123", "12345"), InvalidInputException);
    cleanAuthDir();
}

// Kiem tra login dung thong tin tra ve user chinh xac
TEST_CASE(test_auth_login_success) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auth.registerUser("bob_user", "securePass1");
    auto user = auth.login("bob_user", "securePass1");
    ASSERT_TRUE(user != nullptr);
    ASSERT_EQ(user->getUsername(), std::string("bob_user"));
    cleanAuthDir();
}

// Kiem tra login voi mat khau sai phai throw AuthenticationException
TEST_CASE(test_auth_login_wrong_password_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auth.registerUser("bob_user", "correctPass");
    ASSERT_THROW(auth.login("bob_user", "wrongPass"), AuthenticationException);
    cleanAuthDir();
}

// Kiem tra login voi username khong ton tai phai throw
TEST_CASE(test_auth_login_unknown_user_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    // Chua dang ky ai ca
    ASSERT_THROW(auth.login("nobody", "anyPassword"), AuthenticationException);
    cleanAuthDir();
}

// Kiem tra login voi input rong phai throw
TEST_CASE(test_auth_login_empty_input_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    ASSERT_THROW(auth.login("", "password"), InvalidInputException);
    ASSERT_THROW(auth.login("user", ""),     InvalidInputException);
    cleanAuthDir();
}

// Kiem tra changePassword thanh cong: mat khau moi co hieu luc
TEST_CASE(test_auth_changePassword_success) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auth.registerUser("carol_01", "oldPass1");
    ASSERT_NO_THROW(auth.changePassword("carol_01", "oldPass1", "newPass1"));
    // Dang nhap bang mat khau moi → thanh cong
    auto user = auth.login("carol_01", "newPass1");
    ASSERT_TRUE(user != nullptr);
    // Dang nhap bang mat khau cu → that bai
    ASSERT_THROW(auth.login("carol_01", "oldPass1"), AuthenticationException);
    cleanAuthDir();
}

// Kiem tra changePassword voi mat khau cu sai phai throw
TEST_CASE(test_auth_changePassword_wrong_old_throws) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auth.registerUser("dave_02", "correctOld");
    ASSERT_THROW(
        auth.changePassword("dave_02", "wrongOld", "newPass1"),
        AuthenticationException
    );
    cleanAuthDir();
}

// Kiem tra userExists tra ve dung trang thai ton tai / chua ton tai
TEST_CASE(test_auth_userExists) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    ASSERT_FALSE(auth.userExists("nobody"));
    auth.registerUser("eve_user", "password1");
    ASSERT_TRUE(auth.userExists("eve_user"));
    ASSERT_FALSE(auth.userExists("bob"));
    cleanAuthDir();
}

// Kiem tra nhieu user co the dang ky va dang nhap doc lap
TEST_CASE(test_auth_multiple_users) {
    cleanAuthDir();
    AuthManager auth(TEST_AUTH_DIR);
    auth.registerUser("user1", "pass1234");
    auth.registerUser("user2", "pass5678");
    auth.registerUser("user3", "pass9012");

    ASSERT_TRUE(auth.userExists("user1"));
    ASSERT_TRUE(auth.userExists("user2"));
    ASSERT_TRUE(auth.userExists("user3"));

    // Moi user dang nhap bang mat khau cua chinh ho
    ASSERT_NO_THROW(auth.login("user1", "pass1234"));
    ASSERT_NO_THROW(auth.login("user2", "pass5678"));
    // Dang nhap cheo mat khau phai that bai
    ASSERT_THROW(auth.login("user1", "pass5678"), AuthenticationException);
    cleanAuthDir();
}

// --- Ham chay toan bo Auth tests ---
void runAuthTests() {
    std::cout << "\n--- Auth Tests ---\n";
    RUN_TEST(test_auth_register_success);
    RUN_TEST(test_auth_register_duplicate_throws);
    RUN_TEST(test_auth_register_short_username_throws);
    RUN_TEST(test_auth_register_long_username_throws);
    RUN_TEST(test_auth_register_invalid_chars_throws);
    RUN_TEST(test_auth_register_short_password_throws);
    RUN_TEST(test_auth_login_success);
    RUN_TEST(test_auth_login_wrong_password_throws);
    RUN_TEST(test_auth_login_unknown_user_throws);
    RUN_TEST(test_auth_login_empty_input_throws);
    RUN_TEST(test_auth_changePassword_success);
    RUN_TEST(test_auth_changePassword_wrong_old_throws);
    RUN_TEST(test_auth_userExists);
    RUN_TEST(test_auth_multiple_users);
}
