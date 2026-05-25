#ifndef OPEP_FILE_MANAGER_H
#define OPEP_FILE_MANAGER_H

#include "../core/exceptions.h"
#include "encryption.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>

namespace opep {

// Template class lưu trữ danh sách bản ghi kiểu T ra file
// Yêu cầu T phải có:
//   - Default constructor
//   - friend ostream& operator<<(ostream&, const T&)
//   - friend istream& operator>>(istream&, T&)
template<typename T>
class Database {
private:
    std::string filename;  // Đường dẫn đến file lưu trữ
    bool        encrypted; // true = mã hóa XOR + hex encode trước khi ghi
    std::string encKey;    // Khóa mã hóa

    // Chuỗi phân tách giữa các record trong file
    static constexpr const char* RECORD_SEP = "---OPEP---\n";

public:
    explicit Database(const std::string& filename,
                      bool encrypted       = true,
                      const std::string&   key = DEFAULT_XOR_KEY)
        : filename(filename), encrypted(encrypted), encKey(key) {}

    // Kiểm tra file tồn tại
    bool exists() const {
        std::ifstream f(filename, std::ios::binary);
        return f.good();
    }

    const std::string& getFilename() const { return filename; }

    // Ghi toàn bộ danh sách records vào file (ghi đè nội dung cũ)
    // Ném FileNotFoundException nếu không mở được file
    void save(const std::vector<T>& records) const {
        std::ostringstream oss;
        for (const auto& rec : records)
            oss << rec << RECORD_SEP;

        std::string data = oss.str();
        if (encrypted) data = encryptToFile(data, encKey);

        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open())
            throw FileNotFoundException(filename);
        file << data;
    }

    // Đọc toàn bộ records từ file
    // Ném FileNotFoundException nếu file không tồn tại
    std::vector<T> load() const {
        if (!exists())
            throw FileNotFoundException(filename);

        // Đọc toàn bộ nội dung file dưới dạng binary
        std::ifstream file(filename, std::ios::binary);
        std::string raw((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

        if (raw.empty()) return {};

        std::string plaintext = encrypted ? decryptFromFile(raw, encKey) : raw;

        // Tách từng record theo RECORD_SEP rồi parse bằng operator>>
        std::vector<T> records;
        std::string sep(RECORD_SEP);
        size_t pos = 0;

        while (pos < plaintext.size()) {
            size_t end = plaintext.find(sep, pos);
            if (end == std::string::npos) break;

            std::string chunk = plaintext.substr(pos, end - pos);
            pos = end + sep.size();

            if (chunk.empty()) continue;

            std::istringstream iss(chunk);
            T record;
            if (iss >> record)
                records.push_back(std::move(record));
        }

        return records;
    }

    // Thêm một record vào database (load → append → save)
    void append(const T& record) const {
        std::vector<T> records;
        if (exists()) records = load();
        records.push_back(record);
        save(records);
    }

    // Xóa toàn bộ nội dung file
    void clear() const {
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
            throw FileNotFoundException(filename);
    }
};

} // namespace opep

#endif // OPEP_FILE_MANAGER_H
