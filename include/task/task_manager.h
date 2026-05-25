#ifndef OPEP_TASK_MANAGER_H
#define OPEP_TASK_MANAGER_H

#include "task.h"
#include <vector>
#include <memory>
#include <cstddef>

namespace opep {

// Quản lý tập hợp Task, sở hữu toàn bộ đối tượng qua unique_ptr
class TaskManager {
private:
    std::vector<std::unique_ptr<Task>> tasks;

    // Tìm iterator tới task theo id, trả về end() nếu không có
    std::vector<std::unique_ptr<Task>>::iterator       findIterById(int id);
    std::vector<std::unique_ptr<Task>>::const_iterator findIterById(int id) const;

public:
    TaskManager()  = default;
    ~TaskManager() = default;

    // Không cho copy vì unique_ptr không thể copy
    TaskManager(const TaskManager&)            = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    // Cho phép move
    TaskManager(TaskManager&&)            = default;
    TaskManager& operator=(TaskManager&&) = default;

    // Thêm task mới, nhận ownership hoàn toàn
    void addTask(std::unique_ptr<Task> task);

    // Xóa task theo id, ném InvalidInputException nếu không tìm thấy
    void removeTask(int id);

    // Đánh dấu task hoàn thành, ném InvalidInputException nếu không tìm thấy
    void markDone(int id);

    // Sắp xếp danh sách: HIGH -> MEDIUM -> LOW, cùng priority giữ thứ tự ban đầu
    void sortByPriority();

    // Lấy con trỏ const đến task theo id, trả về nullptr nếu không có
    const Task* getById(int id) const;

    // Truy cập toàn bộ danh sách (read-only)
    const std::vector<std::unique_ptr<Task>>& getAll() const;

    // Số lượng task hiện có
    std::size_t count() const;

    // Số task đã hoàn thành
    std::size_t countCompleted() const;

    // In toàn bộ danh sách ra console
    void displayAll() const;
};

} // namespace opep

#endif // OPEP_TASK_MANAGER_H
