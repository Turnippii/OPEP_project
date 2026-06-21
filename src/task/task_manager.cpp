#include "../../include/task/task_manager.h"
#include "../../include/core/exceptions.h"
#include <algorithm>
#include <iostream>

namespace opep {

// --- findIterById: tìm vị trí task trong vector theo id ---

std::vector<std::unique_ptr<Task>>::iterator
TaskManager::findIterById(int id) {
    return std::find_if(tasks.begin(), tasks.end(),
        [id](const std::unique_ptr<Task>& t) {
            return t->getId() == id;
        });
}

std::vector<std::unique_ptr<Task>>::const_iterator
TaskManager::findIterById(int id) const {
    return std::find_if(tasks.cbegin(), tasks.cend(),
        [id](const std::unique_ptr<Task>& t) {
            return t->getId() == id;
        });
}

// --- addTask: nhận ownership task, kiểm tra id trùng trước khi thêm ---

void TaskManager::addTask(std::unique_ptr<Task> task) {
    if (!task) {
        throw InvalidInputException("Không thể thêm task null");
    }
    if (findIterById(task->getId()) != tasks.end()) {
        throw InvalidInputException(
            "Đã tồn tại task với ID: " + std::to_string(task->getId()));
    }
    tasks.push_back(std::move(task));
}

// --- removeTask: xóa và giải phóng bộ nhớ task ---

void TaskManager::removeTask(int id) {
    auto it = findIterById(id);
    if (it == tasks.end()) {
        throw InvalidInputException(
            "Không tìm thấy task với ID: " + std::to_string(id));
    }
    tasks.erase(it); // unique_ptr tự giải phóng Task khi bị erase
}

// --- markDone: đánh dấu task hoàn thành ---

void TaskManager::markDone(int id) {
    auto it = findIterById(id);
    if (it == tasks.end()) {
        throw InvalidInputException(
            "Không tìm thấy task với ID: " + std::to_string(id));
    }
    (*it)->setCompleted(true);
}

// --- sortByPriority: stable_sort giữ nguyên thứ tự các task cùng mức ---
// HIGH(2) -> MEDIUM(1) -> LOW(0): so sánh ngược (lớn hơn lên trước)

void TaskManager::sortByPriority() {
    std::stable_sort(tasks.begin(), tasks.end(),
        [](const std::unique_ptr<Task>& a, const std::unique_ptr<Task>& b) {
            return static_cast<int>(a->getPriority())
                 > static_cast<int>(b->getPriority());
        });
}

// --- getById: trả về còn trỏ const, không transfer ownership ---

const Task* TaskManager::getById(int id) const {
    auto it = findIterById(id);
    if (it == tasks.cend()) {
        return nullptr;
    }
    return it->get();
}

// --- getAll: truy cập danh sách read-only ---

const std::vector<std::unique_ptr<Task>>& TaskManager::getAll() const {
    return tasks;
}

// --- count / countCompleted ---

std::size_t TaskManager::count() const {
    return tasks.size();
}

std::size_t TaskManager::countCompleted() const {
    return static_cast<std::size_t>(
        std::count_if(tasks.cbegin(), tasks.cend(),
            [](const std::unique_ptr<Task>& t) {
                return t->isCompleted();
            }));
}

// --- displayAll: in bảng task ra console ---

void TaskManager::displayAll() const {
    if (tasks.empty()) {
        std::cout << "Chưa có task nào.\n";
        return;
    }

    std::cout << "+-----------------------------------------+\n";
    std::cout << "| DANH SÁCH CÔNG VIỆC ("
              << countCompleted() << "/" << count() << " hoàn thành) |\n";
    std::cout << "+-----------------------------------------+\n";

    for (const auto& task : tasks) {
        task->displayInfo();
    }

    std::cout << "+-----------------------------------------+\n";
}

} // namespace opep
