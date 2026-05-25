#ifndef OPEP_RECORD_H
#define OPEP_RECORD_H

#include <string>
#include <iostream>

namespace opep {

// Lớp abstract cơ sở cho tất cả bản ghi (Task, Transaction)
// Không thể khởi tạo trực tiếp vì có pure virtual function
class Record {
protected:
    int         id;     // Mã định danh duy nhất
    std::string date;   // Ngày tạo, định dạng YYYY-MM-DD
    std::string title;  // Tiêu đề / tên bản ghi

protected:
    // Default constructor chỉ dùng cho Database<T> serialization (T cần default-constructible)
    Record() : id(0) {}

public:
    // Constructor khởi tạo đầy đủ thuộc tính
    Record(int id, const std::string& date, const std::string& title);

    // Virtual destructor đảm bảo giải phóng đúng khi dùng con trỏ base class
    virtual ~Record() = default;

    // --- Getter ---
    int                getId()    const;
    const std::string& getDate()  const;
    const std::string& getTitle() const;

    // --- Setter ---
    void setDate(const std::string& date);
    void setTitle(const std::string& title);

    // Pure virtual: mỗi lớp con tự quyết cách hiển thị thông tin
    virtual void displayInfo() const = 0;

    // Operator== so sánh hai bản ghi dựa trên id
    bool operator==(const Record& other) const;

    // Friend operator<< cho phép ghi Record ra ostream (cout, file...)
    friend std::ostream& operator<<(std::ostream& os, const Record& record);
};

} // namespace opep

#endif // OPEP_RECORD_H
