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

    throw InvalidInputException("Muc do uu tien khong hop le: '" + s
                                + "'. Chi chap nhan: HIGH, MEDIUM, LOW");
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
    // Ký hiệu trạng thái trực quan
    const std::string statusMark = completed ? "[x]" : "[ ]";
    const std::string priorityTag = "[" + priorityToString(priority) + "]";

    std::cout << statusMark << " "
              << *this           // gọi operator<< từ Record: [ID:x] date | title
              << " " << priorityTag;

    if (!deadline.empty()) {
        std::cout << " | Han: " << deadline;
    }
    std::cout << "\n";
}

// --- operator>> đọc thuộc tính task từ stream (không in prompt) ---
// Định dạng stream: title\nHIGH|MEDIUM|LOW\ndeadline\n
// Module UI chịu trách nhiệm in prompt trước khi gọi operator>>

std::istream& operator>>(std::istream& is, Task& task) {
    std::string newTitle, priorityStr, newDeadline;

    std::getline(is, newTitle);
    std::getline(is, priorityStr);
    std::getline(is, newDeadline);

    task.setTitle(newTitle);
    task.setPriority(stringToPriority(priorityStr));
    task.setDeadline(newDeadline);

    return is;
}

} // namespace opep
