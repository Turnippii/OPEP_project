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
    // Default constructor cho Database<Task>::load()
    Task() : Record(), completed(false), priority(Priority::MEDIUM) {}

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

    // So sánh đầy đủ: id, title, deadline, priority, completed
    bool operator==(const Task& other) const;
    bool operator!=(const Task& other) const;

    // operator<< / operator>> cho serialization vào Database<Task>
    // Format: "<id> <date> <completed>\n<title>\n<priority>\n<deadline>\n"
    friend std::ostream& operator<<(std::ostream& os, const Task& task);
    friend std::istream& operator>>(std::istream& is, Task& task);
};

} // namespace opep

#endif // OPEP_TASK_H
