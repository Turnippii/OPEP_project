#ifndef OPEP_ENCRYPTION_H
#define OPEP_ENCRYPTION_H

#include <string>

namespace opep {

// Khóa XOR mặc định dùng cho toàn bộ ứng dụng
constexpr const char* DEFAULT_XOR_KEY = "OPEP2024";

// Mã hóa/giải mã XOR — symmetric: xorCipher(xorCipher(data)) == data
std::string xorCipher(const std::string& data, const std::string& key = DEFAULT_XOR_KEY);

// Chuyển binary <-> chuỗi hex printable để lưu file an toàn trên Windows/Linux
std::string hexEncode(const std::string& data);
std::string hexDecode(const std::string& hex);

// Hash một chiều: XOR + Caesar + hex — dùng để lưu password, không thể giải ngược
std::string hashPassword(const std::string& password);

// Tiện ích cho Database<T>:
// encryptToFile: XOR encrypt rồi hex encode → chuỗi ASCII an toàn ghi file
// decryptFromFile: hex decode rồi XOR decrypt → plain text gốc
std::string encryptToFile(const std::string& data, const std::string& key = DEFAULT_XOR_KEY);
std::string decryptFromFile(const std::string& encoded, const std::string& key = DEFAULT_XOR_KEY);

} // namespace opep

#endif // OPEP_ENCRYPTION_H
