# OOP Compliance — Tóm Tắt Vấn Đáp (OPEP)

| # | Khái niệm | File chính | Dòng |
|---|---|---|---|
| 1 | Abstract Class | `include/core/record.h` | 38 |
| 2 | Inheritance | `include/task/task.h` : 22, `include/money/transaction.h` : 21 | 39, 28 |
| 3 | Polymorphism | `main.cpp` | 325 |
| 4 | Encapsulation | `include/money/wallet.h` : 16, `src/money/wallet.cpp` | 20, 55, 76 |
| 5 | Template | `include/data/file_manager.h` | 19 |
| 6 | Smart Pointer | `include/task/task_manager.h` : 14, `include/money/wallet.h` : 17 | 36, 51 |
| 7 | Exception Handling | `include/core/exceptions.h` | 11–60 |
| 8 | Operator Overloading | `src/task/task.cpp`, `src/money/transaction.cpp` | 90–124, 79–116 |

---

## 1. Abstract Class

`Record` là lớp trừu tượng — không thể khởi tạo trực tiếp vì có pure virtual function `displayInfo() const = 0`. Lớp con **bắt buộc** phải cài đặt hàm này; nếu không, compiler từ chối tạo object. Virtual destructor đảm bảo gọi đúng destructor lớp con khi `delete` qua `Record*`.

```cpp
// include/core/record.h : 26, 38
virtual ~Record() = default;            // destructor ảo — tránh memory leak khi xóa qua Record*
virtual void displayInfo() const = 0;  // pure virtual → Record là abstract class
```

---

## 2. Inheritance

`Task` và `Transaction` kế thừa `public` từ `Record` — quan hệ **is-a** (Task là một Record). Constructor lớp con tường minh gọi `Record(id, date, title)` qua member initializer list để khởi tạo trường cha. Từ khóa `override` giúp compiler kiểm tra chữ ký hàm khớp với base class.

```cpp
// include/task/task.h : 22, 49
class Task : public Record {
    void displayInfo() const override;   // ghi đè pure virtual, compiler kiểm tra chữ ký

// src/task/task.cpp : 39
    : Record(id, date, title)            // gọi constructor lớp cha trước khi init trường riêng
```

---

## 3. Polymorphism

Một dòng `r->displayInfo()` với `r` kiểu `Record*` — runtime tra **vtable** để gọi đúng `Task::displayInfo()` hoặc `Transaction::displayInfo()` tùy kiểu thực của đối tượng. Đây là **dynamic dispatch**: quyết định xảy ra lúc chạy, không phải lúc biên dịch. Demo được đóng gói trong `demoPolymorphism()` ở `main.cpp`.

```cpp
// main.cpp : 300, 306, 312, 325
std::vector<const Record*> records;
records.push_back(tasks[i].get());   // unique_ptr<Task> → Record*
records.push_back(txns[i].get());    // shared_ptr<Transaction> → Record*
for (const Record* r : records)
    r->displayInfo();                // 1 dòng → 2 hành vi khác nhau qua vtable
```

---

## 4. Encapsulation

`balance` trong `Wallet` là `private` và **không có setter** — code bên ngoài không thể gán trực tiếp. Số dư chỉ thay đổi qua `addIncome()` hoặc `addExpense()`, mỗi hàm validate trước khi sửa (ví dụ: `addExpense` ném `InsufficientBalanceException` nếu không đủ số dư). Business invariant "số dư không âm" được đảm bảo bởi class, không phụ thuộc caller.

```cpp
// include/money/wallet.h : 16         // src/money/wallet.cpp : 20, 69-70, 76
double balance;                        // PRIVATE — không có setBalance()
double Wallet::getBalance() const { return balance; }   // read-only
if (amount > balance)
    throw InsufficientBalanceException(amount, balance); // validate
balance -= amount;                     // chỉ sửa tại đây, sau khi validate
```

---

## 5. Template

`Database<T>` là class template lưu/nạp danh sách object bất kỳ kiểu `T` ra file (có mã hóa XOR). Compiler sinh ra `Database<Task>` và `Database<Transaction>` — hai class hoàn chỉnh, type-safe — từ một template source. Yêu cầu: `T` phải có default constructor, `operator<<`, `operator>>`.

```cpp
// include/data/file_manager.h : 19
template<typename T>
class Database {
    void save(const std::vector<T>& records) const { for (auto& r : records) os << r; }
    std::vector<T> load() const { T rec; if (is >> rec) results.push_back(rec); }
};
// main.cpp : 50, 56
Database<Transaction>("transactions.db").save(txns);
Database<Task>       ("tasks.db"       ).save(tasks);
```

---

## 6. Smart Pointer

`TaskManager` dùng `unique_ptr<Task>` — sở hữu độc quyền, tự `delete` khi xóa khỏi vector, copy bị `= delete`. `Wallet` dùng `shared_ptr<Transaction>` — chia sẻ ownership với caller nhận về từ `addIncome`/`addExpense`. Không có `new`/`delete` thủ công trong toàn bộ codebase.

```cpp
// include/task/task_manager.h : 14, 25-26, 33
std::vector<std::unique_ptr<Task>> tasks;
TaskManager(const TaskManager&) = delete;          // unique_ptr không thể copy
void addTask(std::unique_ptr<Task> task);           // nhận ownership

// src/money/wallet.cpp : 51, 37
auto txn = std::make_shared<Transaction>(nextId++, ...);   // shared ownership
transactions.push_back(std::make_shared<Transaction>(t));  // load từ file
```

---

## 7. Exception Handling

Hierarchy 2 tầng: `OPEPException : std::exception` làm base, 5 lớp con cho từng tình huống lỗi cụ thể. Message lưu dạng `std::string` — không có dangling pointer khi exception propagate lên stack. Caller có thể bắt tổng quát (`OPEPException`) hoặc cụ thể (`BudgetExceededException`).

```cpp
// include/core/exceptions.h : 11-60
class OPEPException : public std::exception {
    const char* what() const noexcept override { return message.c_str(); }
};
class InvalidInputException    : public OPEPException { /* input sai */ };
class InsufficientBalanceException : public OPEPException { /* hết tiền */ };
class BudgetExceededException  : public OPEPException { /* vượt hạn mức */ };
```

---

## 8. Operator Overloading

`operator==`/`!=` so sánh theo ngữ nghĩa nghiệp vụ (id, amount, category...) thay vì địa chỉ bộ nhớ. `operator<<`/`operator>>` là **cặp serialization đối xứng** — dữ liệu ghi ra bởi `<<` đọc lại đúng bởi `>>` — cho phép `Database<T>` lưu/nạp mọi kiểu với cú pháp stream thống nhất.

```cpp
// src/task/task.cpp : 90-95, 105-111
bool Task::operator==(const Task& o) const {
    return id == o.id && title == o.title && priority == o.priority && completed == o.completed;
}
std::ostream& operator<<(std::ostream& os, const Task& t) {
    os << t.id << " " << t.date << " " << t.completed << "\n" << t.title << "\n";
}
std::istream& operator>>(std::istream& is, Task& t) { is >> t.id >> t.date; /* ... */ }
```
