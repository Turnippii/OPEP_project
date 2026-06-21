#include "../../include/data/auth.h"
#include "../../include/data/encryption.h"
#include "../../include/core/exceptions.h"
#include <filesystem>
#include <algorithm>
#include <ctime>

namespace opep {

namespace fs = std::filesystem;

// --- Private static helpers ---

void AuthManager::validateUsername(const std::string& username) {
    if (username.size() < 3 || username.size() > 20)
        throw InvalidInputException("Username phải từ 3 đến 20 ký tự");

    bool valid = std::all_of(username.begin(), username.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    });
    if (!valid)
        throw InvalidInputException("Username chỉ được dùng a-z, A-Z, 0-9, _");
}

void AuthManager::validatePassword(const std::string& password) {
    if (password.size() < 6)
        throw InvalidInputException("Password phải có ít nhất 6 ký tự");
}

std::string AuthManager::currentDate() {
    std::time_t t = std::time(nullptr);
    std::tm* tm   = std::localtime(&t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return buf;
}

// --- Constructor ---

AuthManager::AuthManager(const std::string& dir)
    : dataDir(dir)
    , userDb(dir + "users.db")
{
    // Tạo thư mục data/ nếu chưa tồn tại
    if (!fs::exists(dataDir))
        fs::create_directories(dataDir);
}

// --- registerUser ---

std::shared_ptr<User> AuthManager::registerUser(const std::string& username,
                                                  const std::string& password)
{
    validateUsername(username);
    validatePassword(password);

    if (userExists(username))
        throw AuthenticationException("Username '" + username + "' đã tồn tại");

    std::string hash = hashPassword(password);
    User newUser(username, hash, currentDate());
    userDb.append(newUser);

    return std::make_shared<User>(newUser);
}

// --- login ---

std::shared_ptr<User> AuthManager::login(const std::string& username,
                                          const std::string& password)
{
    if (username.empty() || password.empty())
        throw InvalidInputException("Username và password không được để trống");

    // Không phân biệt "sai username" hay "sai password" để tránh user enumeration
    auto throwFail = []() {
        throw AuthenticationException("Tên đăng nhập hoặc mật khẩu không đúng");
    };

    if (!userDb.exists()) throwFail();

    std::vector<User> users = userDb.load();
    std::string hash = hashPassword(password);

    for (const auto& u : users) {
        if (u.getUsername() == username) {
            if (u.getPasswordHash() != hash) throwFail();
            return std::make_shared<User>(u);
        }
    }

    throwFail();
    return nullptr; // unreachable — tránh cảnh báo compiler
}

// --- changePassword ---

void AuthManager::changePassword(const std::string& username,
                                  const std::string& oldPassword,
                                  const std::string& newPassword)
{
    validatePassword(newPassword);

    if (!userDb.exists())
        throw AuthenticationException("Không tìm thấy tài khoản: " + username);

    std::vector<User> users = userDb.load();
    std::string oldHash = hashPassword(oldPassword);
    std::string newHash = hashPassword(newPassword);

    bool found = false;
    for (auto& u : users) {
        if (u.getUsername() == username) {
            if (u.getPasswordHash() != oldHash)
                throw AuthenticationException("Mật khẩu cũ không đúng");
            u.setPasswordHash(newHash);
            found = true;
            break;
        }
    }

    if (!found)
        throw AuthenticationException("Không tìm thấy tài khoản: " + username);

    userDb.save(users);
}

// --- userExists ---

bool AuthManager::userExists(const std::string& username) const {
    if (!userDb.exists()) return false;
    std::vector<User> users = userDb.load();
    return std::any_of(users.begin(), users.end(), [&](const User& u) {
        return u.getUsername() == username;
    });
}

} // namespace opep
