# Personal Budget & Task Manager (OPEP)

## Mô tả dự án
Đồ án cuối kỳ môn Lập trình Hướng đối tượng C++.
Ứng dụng quản lý chi tiêu và công việc cá nhân, có 2 phiên bản: Console (cũ) và GUI Qt (mới).
Nhóm 5 thành viên.

## Ngôn ngữ & Công cụ
- C++17, chỉ dùng STL chuẩn cho backend
- Build console bằng Makefile + g++
- Build GUI bằng CMake + Qt 6.11 (MSYS2 UCRT64)
- OS: Windows + Linux
- Không dùng thư viện ngoài (backend); GUI dùng Qt Widgets

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
- include/        : header (.h) chia 5 module
- src/            : implementation (.cpp) chia 5 module
- main.cpp        : entry point (console)
- gui/            : Qt widgets (phiên bản GUI)
- CMakeLists.txt  : build GUI
- README_GUI.md   : hướng dẫn build/chạy GUI
- data/           : file lưu trữ runtime (sẽ tạo khi chạy)
- docs/           : UML, báo cáo
- tests/          : unit test
- build/          : output biên dịch

## 5 module
1. core/   - Record (abstract), exceptions, common types
2. task/   - Task, TaskManager, Subtask, Priority
3. money/  - Transaction, Budget, Goal, Wallet
4. data/   - FileManager, User, Auth, Encryption, CSV
5. ui/     - Menu, Dashboard ASCII, InputValidator (console)

## Phiên bản GUI Qt
- 4 module backend (core/task/money/data) dùng chung với console, không sửa logic nghiệp vụ
- gui/ gồm 7 file:
  - appservice.h/.cpp - Service Layer, cầu nối giữa backend và UI
  - loginwindow.h/.cpp - màn hình đăng nhập
  - mainwindow.h/.cpp - màn hình chính
  - main_gui.cpp - entry point GUI
- Giao diện chính có 4 tab: Tổng quan, Giao dịch, Task, Tài khoản
- Tính năng: bộ lọc giao dịch, xóa hạn mức ngân sách, ô nhập tiền tự format
- Build qua CMake, yêu cầu Qt 6.11 trên MSYS2 UCRT64

## Build & chạy
- `make`        : build console
- `make test`   : chạy unit test (135 test case)
- `make gui`     : build GUI qua CMake
- `make run-gui` : chạy GUI

## Yêu cầu khác
- File lưu trữ mã hóa đơn giản (XOR hoặc Caesar)
- Mọi input từ user phải validate, sai thì throw exception
- Đa người dùng (mỗi user 1 file dữ liệu riêng)
- Có dashboard ASCII hiển thị tổng quan khi đăng nhập (console)

## Ghi chú quan trọng (memory notes)
- Service Layer pattern: mọi tương tác GUI ↔ backend đi qua class `AppService`, UI không gọi trực tiếp các class backend
- main.cpp (console) đã thêm `setupConsoleUtf8()` để hiển thị tiếng Việt có dấu đúng trên terminal Windows
- Cảnh báo vượt hạn mức ngân sách (80%/100%) hoạt động ở cả console và GUI, dùng chung logic từ module money/
- Bộ test mutation testing đã verify, vẫn còn nguyên không bị phá vỡ khi thêm GUI