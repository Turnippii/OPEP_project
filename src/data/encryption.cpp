#include "../../include/data/encryption.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace opep {

// --- XOR cipher ---

std::string xorCipher(const std::string& data, const std::string& key) {
    if (key.empty()) return data;
    std::string result = data;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] ^= static_cast<char>(key[i % key.size()]);
    return result;
}

// --- Hex encode / decode ---

std::string hexEncode(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    return oss.str();
}

std::string hexDecode(const std::string& hex) {
    std::string result;
    result.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        unsigned int byte = 0;
        std::istringstream iss(hex.substr(i, 2));
        iss >> std::hex >> byte;
        result.push_back(static_cast<char>(byte));
    }
    return result;
}

// --- Hash password (một chiều) ---
// XOR với DEFAULT_XOR_KEY + Caesar shift +13 + hex encode
// Mục đích: không lưu plain text, đủ cho demo sinh viên

std::string hashPassword(const std::string& password) {
    std::string h = xorCipher(password, DEFAULT_XOR_KEY);
    for (char& c : h)
        c = static_cast<char>((static_cast<unsigned char>(c) + 13) % 256);
    return hexEncode(h);
}

// --- Tiện ích cho Database<T> ---

std::string encryptToFile(const std::string& data, const std::string& key) {
    return hexEncode(xorCipher(data, key));
}

std::string decryptFromFile(const std::string& encoded, const std::string& key) {
    return xorCipher(hexDecode(encoded), key);
}

} // namespace opep
