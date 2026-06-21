#include "../../include/task/task.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <algorithm>

namespace opep {

// --- Helper chuyển đổi Priority <-> string ---

std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::HIGH:   return "HIGH";
        case Priority::MEDIUM: return "MEDIUM";
        case Priority::LOW:    return "LOW";
    }
    return "MEDIUM"; // không thể xảy ra, nhưng tránh compiler warning
}

Priority stringToPriority(const std::string& s) {
    // Chuẩn hóa về chữ hoa để so sánh không phân biệt hoa thường
    std::string upper = s;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper == "HIGH")   return Priority::HIGH;
    if (upper == "MEDIUM") return Priority::MEDIUM;
    if (upper == "LOW")    return Priority::LOW;

    throw InvalidInputException("Mức độ ưu tiên không hợp lệ: '" + s
                                + "'. Chỉ chấp nhận: HIGH, MEDIUM, LOW");
}

// --- Task constructor ---

Task::Task(int id,
           const std::string& date,
           const std::string& title,
           Priority priority,
           const std::string& deadline)
    : Record(id, date, title)   // base class validate title không rỗng
    , completed(false)
    , priority(priority)
    , deadline(deadline)
{}

// --- Getter ---

bool Task::isCompleted() const {
    return completed;
}

Priority Task::getPriority() const {
    return priority;
}

const std::string& Task::getDeadline() const {
    return deadline;
}

// --- Setter ---

void Task::setCompleted(bool value) {
    completed = value;
}

void Task::setPriority(Priority p) {
    priority = p;
}

void Task::setDeadline(const std::string& newDeadline) {
    deadline = newDeadline;
}

// --- displayInfo: hiển thị task dạng bảng ASCII ---

void Task::displayInfo() const {
    const std::string statusMark  = completed ? "[x]" : "[ ]";
    const std::string priorityTag = "[" + priorityToString(priority) + "]";

    // Dùng static_cast để gọi operator<< của Record, không phải của Task
    std::cout << statusMark << " "
              << static_cast<const Record&>(*this)
              << " " << priorityTag;

    if (!deadline.empty()) std::cout << " | Hạn: " << deadline;
    std::cout << "\n";
}

// --- Equality operators ---

bool Task::operator==(const Task& other) const {
    return id        == other.id
        && title     == other.title
        && deadline  == other.deadline
        && priority  == other.priority
        && completed == other.completed;
}

bool Task::operator!=(const Task& other) const {
    return !(*this == other);
}

// --- Serialization operator<< / operator>> cho Database<Task> ---
// Format: "<id> <date> <completed>\n<title>\n<priority>\n<deadline>\n"

std::ostream& operator<<(std::ostream& os, const Task& t) {
    os << t.id << " " << t.date << " " << (t.completed ? 1 : 0) << "\n"
       << t.title    << "\n"
       << priorityToString(t.priority) << "\n"
       << t.deadline << "\n";
    return os;
}

std::istream& operator>>(std::istream& is, Task& t) {
    int completedInt = 0;
    is >> t.id >> t.date >> completedInt;
    is.ignore();
    std::getline(is, t.title);
    std::string priorityStr;
    std::getline(is, priorityStr);
    std::getline(is, t.deadline);
    t.completed = (completedInt != 0);
    t.priority  = stringToPriority(priorityStr);
    return is;
}

} // namespace opep
