#include "../../include/core/record.h"
#include "../../include/core/exceptions.h"

namespace opep {

// Khởi tạo Record với id, ngày và tiêu đề
// Validate tiêu đề không rỗng ngay tại constructor
Record::Record(int id, const std::string& date, const std::string& title)
    : id(id), date(date), title(title)
{
    if (title.empty()) {
        throw InvalidInputException("Tieu de khong duoc de trong");
    }
}

// --- Getter ---

int Record::getId() const {
    return id;
}

const std::string& Record::getDate() const {
    return date;
}

const std::string& Record::getTitle() const {
    return title;
}

// --- Setter ---

void Record::setDate(const std::string& newDate) {
    if (newDate.empty()) {
        throw InvalidInputException("Ngay khong duoc de trong");
    }
    date = newDate;
}

void Record::setTitle(const std::string& newTitle) {
    if (newTitle.empty()) {
        throw InvalidInputException("Tieu de khong duoc de trong");
    }
    title = newTitle;
}

// So sánh bằng dựa trên id — hai bản ghi cùng id là một bản ghi
bool Record::operator==(const Record& other) const {
    return id == other.id;
}

// In thông tin cơ bản; lớp con có thể mở rộng bằng displayInfo()
std::ostream& operator<<(std::ostream& os, const Record& record) {
    os << "[ID:" << record.id << "] "
       << record.date << " | "
       << record.title;
    return os;
}

} // namespace opep
