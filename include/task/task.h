#ifndef OPEP_TASK_H
#define OPEP_TASK_H

#include "../core/record.h"
#include <string>
#include <iostream>

namespace opep {

// Mức độ ưu tiên của task, dùng enum class để tránh xung đột tên
enum class Priority {
    LOW    = 0,
    MEDIUM = 1,
    HIGH   = 2
};

// Chuyển Priority <-> string, dùng trong displayInfo và file I/O
std::string  priorityToString(Priority p);
Priority     stringToPriority(const std::string& s);

// Lớp Task kế thừa Record, đại diện cho một công việc cần làm
class Task : public Record {
private:
    bool        completed;  // Trạng thái hoàn thành
    Priority    priority;   // Mức độ ưu tiên
    std::string deadline;   // Hạn chót, định dạng YYYY-MM-DD (rỗng = không giới hạn)

public:
    Task(int id,
         const std::string& date,
         const std::string& title,
         Priority priority          = Priority::MEDIUM,
         const std::string& deadline = "");

    // --- Getter ---
    bool               isCompleted()  const;
    Priority           getPriority()  const;
    const std::string& getDeadline()  const;

    // --- Setter ---
    void setCompleted(bool value);
    void setPriority(Priority p);
    void setDeadline(const std::string& deadline);

    // Override pure virtual từ Record
    void displayInfo() const override;

    // operator>> đọc dữ liệu task từ stream (dùng khi load file hoặc nhập liệu)
    friend std::istream& operator>>(std::istream& is, Task& task);
};

} // namespace opep

#endif // OPEP_TASK_H
