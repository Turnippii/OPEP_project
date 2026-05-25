# Personal Budget & Task Manager (OPEP)

## Mô tả dự án
Đồ án cuối kỳ môn Lập trình Hướng đối tượng C++.
Ứng dụng console quản lý chi tiêu và công việc cá nhân.
Nhóm 5 thành viên.

## Ngôn ngữ & Công cụ
- C++17, chỉ dùng STL chuẩn
- Build bằng Makefile + g++
- OS: Windows + Linux
- Không dùng thư viện ngoài

## Yêu cầu OOP bắt buộc thể hiện trong code
- Abstract class: Record là pure virtual
- Inheritance: Task, Transaction kế thừa Record
- Polymorphism: virtual displayInfo()
- Encapsulation: balance là private
- Template: lớp Database<T>
- Smart pointer: dùng unique_ptr, shared_ptr (KHÔNG dùng raw pointer)
- Exception handling: tạo custom exception class
- Operator overloading: nạp chồng <<, >>, ==

## Quy ước code
- Tên class: PascalCase (Task, Transaction, Record)
- Tên hàm và biến: camelCase (addTask, totalAmount)
- Tên file: snake_case (task_manager.cpp, record.h)
- Mọi class nằm trong namespace `opep`
- Comment giải thích bằng tiếng Việt
- Tên hàm, biến viết tiếng Anh

## Cấu trúc thư mục
- include/  : header (.h) chia 5 module
- src/      : implementation (.cpp) chia 5 module
- main.cpp  : entry point
- data/     : file lưu trữ runtime (sẽ tạo khi chạy)
- docs/     : UML, báo cáo
- tests/    : unit test
- build/    : output biên dịch

## 5 module
1. core/   - Record (abstract), exceptions, common types
2. task/   - Task, TaskManager, Subtask, Priority
3. money/  - Transaction, Budget, Goal, Wallet
4. data/   - FileManager, User, Auth, Encryption, CSV
5. ui/     - Menu, Dashboard ASCII, InputValidator

## Yêu cầu khác
- KHÔNG dùng GUI, chỉ console
- File lưu trữ mã hóa đơn giản (XOR hoặc Caesar)
- Mọi input từ user phải validate, sai thì throw exception
- Đa người dùng (mỗi user 1 file dữ liệu riêng)
- Có dashboard ASCII hiển thị tổng quan khi đăng nhập