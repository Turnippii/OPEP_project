# 💰 OPEP — Personal Budget & Task Manager

<div align="center">

```
  ██████╗  ██████╗ ██████╗ 
 ██╔═══██╗██╔═══██╗██╔══██╗
 ██║   ██║██║   ██║██████╔╝
██║   ██║██║   ██║██╔═══╝
╚██████╔╝╚██████╔╝██║   
 ╚═════╝ ╚═════╝ ╚═╝     
```

**Ứng dụng console quản lý chi tiêu và công việc cá nhân, viết bằng C++17 thuần**

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/Status-Complete-brightgreen.svg?style=flat-square)]()
[![License](https://img.shields.io/badge/License-Academic-orange.svg?style=flat-square)]()
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg?style=flat-square)]()
[![Build](https://img.shields.io/badge/Build-GNU%20Make-yellow.svg?style=flat-square)]()

</div>

---

## 📋 Mục Lục

1. [Giới Thiệu](#-giới-thiệu)
2. [Tính Năng Đầy Đủ](#-tính-năng-đầy-đủ)
3. [Yêu Cầu Hệ Thống](#-yêu-cầu-hệ-thống)
4. [Hướng Dẫn Cài Đặt](#-hướng-dẫn-cài-đặt)
5. [Hướng Dẫn Sử Dụng](#-hướng-dẫn-sử-dụng)
6. [Cấu Trúc Thư Mục](#-cấu-trúc-thư-mục)
7. [Kiến Trúc OOP](#-kiến-trúc-oop)
8. [Sơ Đồ UML](#-sơ-đồ-uml)
9. [Công Nghệ Sử Dụng](#-công-nghệ-sử-dụng)
10. [Thành Viên Nhóm](#-thành-viên-nhóm)
11. [Roadmap](#-roadmap)
12. [Đóng Góp](#-đóng-góp)
13. [License](#-license)
14. [Liên Hệ](#-liên-hệ)

---

## 📚 Giới Thiệu

### Lý do chọn đề tài

Quản lý chi tiêu và công việc cá nhân là nhu cầu thực tế của mọi sinh viên và người đi làm. Thay vì dùng spreadsheet hay app di động, **OPEP** cung cấp một công cụ nhẹ, nhanh, chạy hoàn toàn trên terminal — không cần cài thư viện ngoài, không cần kết nối mạng.

Dự án được thiết kế để vừa **giải quyết bài toán thực tế**, vừa minh họa đầy đủ các khái niệm **Lập trình Hướng đối tượng C++** theo yêu cầu đồ án.

### Đối tượng người dùng

- Sinh viên muốn theo dõi ngân sách học kỳ và deadline bài tập
- Người đi làm cần nhật ký thu/chi cá nhân đơn giản
- Người yêu thích công cụ terminal, tối giản, không phụ thuộc cloud

### ✨ Tính năng nổi bật

- 🔐 **Đa người dùng** — mỗi tài khoản có file dữ liệu riêng, mật khẩu được hash một chiều
- 📊 **Dashboard ASCII** — tổng quan số dư, thu/chi tháng, task sắp đến hạn ngay khi đăng nhập
- ⚠️ **Cảnh báo ngân sách thông minh** — tự động cảnh báo khi chi tiêu đạt 80% hoặc vượt hạn mức
- 🗂️ **Quản lý Task đầy đủ** — priority (LOW/MEDIUM/HIGH), deadline, đánh dấu hoàn thành
- 🔒 **Mã hóa dữ liệu** — file lưu trữ được mã hóa XOR + hex encode, an toàn trên mọi OS

---

## 🚀 Tính Năng Đầy Đủ

### 🔐 Module Xác Thực (`data/`)

| Tính năng | Chi tiết |
|---|---|
| Đăng ký | Username 3–20 ký tự (a-z, A-Z, 0-9, _), password ≥ 6 ký tự |
| Đăng nhập | Không phân biệt "sai username" / "sai password" — tránh user enumeration |
| Đổi mật khẩu | Xác minh mật khẩu cũ trước khi cập nhật |
| Mã hóa password | Hash một chiều: XOR + Caesar shift + hex encode — không thể giải ngược |
| Lưu trữ | File `data/users.db` mã hóa XOR, mỗi user trên 3 dòng |

### ✅ Module Quản Lý Task (`task/`)

| Tính năng | Chi tiết |
|---|---|
| Thêm task | Tên, ngày tạo, mức ưu tiên (LOW/MEDIUM/HIGH), hạn chót tuỳ chọn |
| Xem tất cả | Danh sách có trạng thái `[x]`/`[ ]`, priority tag, deadline |
| Đánh dấu hoàn thành | Theo ID, có xác nhận |
| Xóa task | Theo ID, có xác nhận |
| Sắp xếp | HIGH → MEDIUM → LOW, giữ thứ tự tương đối (stable sort) |
| Thống kê | Tổng task / hoàn thành hiển thị ở cuối danh sách |

### 💳 Module Quản Lý Tài Chính (`money/`)

| Tính năng | Chi tiết |
|---|---|
| Thêm thu nhập | Số tiền, danh mục (Lương/Thưởng/Khác), mô tả, ghi chú |
| Thêm chi tiêu | Chọn danh mục từ lịch sử hoặc nhập mới |
| Xem giao dịch | Toàn bộ lịch sử với định dạng bảng; tổng kết số dư |
| Đặt hạn mức | Hạn mức theo danh mục (VND/tháng), tự động persist qua các phiên |
| Cảnh báo 80% | `[!] CANH BAO: Chi tieu nay se dat XX% han muc ...` + hỏi y/n |
| Cảnh báo 100% | `[!!] VUOT HAN MUC: Han muc la Y VND, ban dang chi vuot Z VND` + hỏi y/n |
| Xem ngân sách | Bảng ASCII với progress bar, % sử dụng, trạng thái từng danh mục |

### 📊 Module Dashboard (`ui/`)

Hiển thị tự động sau đăng nhập và khi làm mới:

```
+--------------------------------------------------------------+
|                    OPEP Dashboard - user                     |
+--------------------------------------------------------------+
| So du hien tai:                          1,250,000 VND       |
| Thu nhap thang nay:                      5,000,000 VND       |
| Chi tieu thang nay:                      3,750,000 VND       |
| Chi tieu hom nay:                          150,000 VND       |
+--------------------------------------+
| Task sap den han (3 ngay toi):                               |
|   [ ] Nop bai tap lon       | Han: 2026-06-04  [HIGH]        |
|   [ ] Kiem tra giua ky      | Han: 2026-06-05  [HIGH]        |
+--------------------------------------------------------------+
```

### 💾 Module Lưu Trữ (`data/`)

- Mỗi user có thư mục `data/<username>/` riêng biệt
- `transactions.db` — giao dịch, mã hóa XOR + hex
- `tasks.db` — công việc, mã hóa XOR + hex
- `budget.cfg` — hạn mức ngân sách, plain text (tháng + `category|limit`)
- Auto-save sau mỗi thao tác; auto-load khi đăng nhập

---

## 💻 Yêu Cầu Hệ Thống

| Thành phần | Yêu cầu tối thiểu |
|---|---|
| **Compiler** | g++ 7.0+ (hỗ trợ C++17) hoặc MSVC 2019+ |
| **Build tool** | GNU Make 3.81+ |
| **OS** | Windows 10+, Ubuntu 20.04+, macOS 10.15+ |
| **RAM** | ~50 MB |
| **Disk** | ~10 MB (chưa tính dữ liệu user) |
| **Terminal** | Hỗ trợ UTF-8 (Windows: chạy `chcp 65001` nếu cần) |

> **Lưu ý Windows:** Dùng MinGW-w64 hoặc MSYS2. MSVC cần điều chỉnh Makefile.

---

## 🔧 Hướng Dẫn Cài Đặt

### 6A — Clone repository

```bash
git clone https://github.com/<username>/OPEP_project.git
cd OPEP_project
```

### 6B — Build trên Windows (MinGW / MSYS2)

```bash
make
# Output: build/opep.exe
```

> Nếu gặp lỗi temp file: đặt biến môi trường `TEMP` về thư mục user trước khi chạy make.

### 6C — Build trên Linux / macOS

```bash
make
# Output: build/opep
```

> GCC < 9 trên Linux cần thêm `-lstdc++fs` — bỏ comment dòng `LDFLAGS` trong Makefile.

### 6D — Chạy ứng dụng

```bash
make run           # build (nếu cần) rồi chạy ngay

./build/opep       # Linux / macOS
./build/opep.exe   # Windows
```

### 6E — Dọn dẹp

```bash
make clean         # xóa thư mục build/
```

---

## 📖 Hướng Dẫn Sử Dụng

### Lần đầu sử dụng

1. Chạy ứng dụng → chọn **[2] Đăng ký tài khoản**
2. Nhập username (3–20 ký tự, chỉ `a-z A-Z 0-9 _`)
3. Nhập password (tối thiểu 6 ký tự) → nhập lại để xác nhận
4. Chọn **[1] Đăng nhập** → nhập thông tin vừa tạo

### Sau khi đăng nhập

```
+==========================================+
|           MENU CHINH  [username]         |
+==========================================+
|  [1] Quan ly Task                        |
|  [2] Quan ly Tai chinh                   |
+------------------------------------------+
|  [3] Lam moi Dashboard                   |
|  [4] Demo Polymorphism (xem da hinh dong)|
|  [5] Doi mat khau                        |
+==========================================+
|  [0] Dang xuat                           |
+==========================================+
```

### Gợi ý thứ tự sử dụng lần đầu

1. **Thêm thu nhập** — nhập số dư ban đầu / lương tháng
2. **Đặt hạn mức danh mục** — ví dụ: "An uong" 2,000,000 VND/tháng
3. **Thêm chi tiêu** — hệ thống tự cảnh báo khi gần đến hạn mức
4. **Thêm task** — ghi lại deadline quan trọng
5. **Xem Dashboard** — tổng quan nhanh mỗi khi mở app

### Lưu ý quan trọng

- 📁 Dữ liệu lưu trong `data/<username>/` — **không** xóa thư mục này
- 🔄 Hạn mức ngân sách được lưu theo tháng (`YYYY-MM`) — sang tháng mới cần đặt lại
- 💬 Mọi input sai sẽ được yêu cầu nhập lại tự động (không crash)

---

## 📁 Cấu Trúc Thư Mục

```
OPEP_project/
│
├── 📄 main.cpp                     # Entry point — auth flow + session management
├── 📄 Makefile                     # Build script, auto-detect Windows/Linux/macOS
├── 📄 README.md                    # File này
│
├── 📂 include/                     # Header files (.h) — khai báo class
│   ├── core/
│   │   ├── record.h                # Abstract class Record (pure virtual displayInfo)
│   │   └── exceptions.h            # OPEPException + 5 custom exception class
│   ├── task/
│   │   ├── task.h                  # Task : public Record, enum Priority
│   │   └── task_manager.h          # TaskManager (unique_ptr<Task>)
│   ├── money/
│   │   ├── transaction.h           # Transaction : public Record, enum TransactionType
│   │   ├── wallet.h                # Wallet — balance PRIVATE, shared_ptr<Transaction>
│   │   └── budget.h                # Budget, CategoryBudget struct
│   ├── data/
│   │   ├── file_manager.h          # Database<T> template class
│   │   ├── user.h                  # User class
│   │   ├── auth.h                  # AuthManager
│   │   └── encryption.h            # XOR cipher, hex encode/decode, hashPassword
│   └── ui/
│       ├── menu.h                  # Menu console nhiều cấp
│       ├── dashboard.h             # Dashboard ASCII
│       └── input_validator.h       # InputValidator — static utility methods
│
├── 📂 src/                         # Implementation files (.cpp)
│   ├── core/record.cpp
│   ├── task/task.cpp
│   ├── task/task_manager.cpp
│   ├── money/transaction.cpp
│   ├── money/wallet.cpp
│   ├── money/budget.cpp
│   ├── data/encryption.cpp
│   ├── data/user.cpp
│   ├── data/auth.cpp
│   ├── ui/input_validator.cpp
│   ├── ui/menu.cpp
│   └── ui/dashboard.cpp
│
├── 📂 data/                        # Runtime data — tạo tự động khi chạy
│   ├── users.db                    # Danh sách tài khoản (mã hóa XOR)
│   └── <username>/
│       ├── transactions.db         # Giao dịch của user (mã hóa XOR)
│       ├── tasks.db                # Task của user (mã hóa XOR)
│       └── budget.cfg              # Hạn mức ngân sách (plain text, theo tháng)
│
├── 📂 docs/                        # Tài liệu kỹ thuật
│   ├── oop_compliance_report.md    # Báo cáo chi tiết 8 yêu cầu OOP (667 dòng)
│   ├── oop_compliance_summary.md   # Tóm tắt vấn đáp nhanh (137 dòng)
│   ├── class_diagram.puml          # Class Diagram PlantUML (23 class, 21 quan hệ)
│   ├── sequence_login.puml         # Sequence Diagram: luồng đăng nhập
│   └── sequence_add_expense.puml   # Sequence Diagram: luồng thêm chi tiêu
│
├── 📂 tests/                       # Unit tests (WIP)
└── 📂 build/                       # Output biên dịch — tạo bởi make
    └── opep(.exe)
```

---

## 🏗️ Kiến Trúc OOP

Dự án thể hiện đầy đủ **8 yêu cầu OOP** của môn học:

| # | Khái niệm | Thể hiện trong code | File : Dòng |
|---|---|---|---|
| 1 | **Abstract Class** | `virtual void displayInfo() const = 0` | `record.h` : 38 |
| 2 | **Inheritance** | `Task : public Record`, `Transaction : public Record` | `task.h` : 22, `transaction.h` : 21 |
| 3 | **Polymorphism** | `for (Record* r : records) r->displayInfo()` — vtable dispatch | `main.cpp` : 325 |
| 4 | **Encapsulation** | `balance` private, không có setter, chỉ sửa qua `addIncome/addExpense` | `wallet.h` : 16 |
| 5 | **Template** | `template<typename T> class Database` — dùng với Task, Transaction, User | `file_manager.h` : 19 |
| 6 | **Smart Pointer** | `unique_ptr<Task>` (TaskManager), `shared_ptr<Transaction>` (Wallet) | `task_manager.h` : 14 |
| 7 | **Exception Handling** | `OPEPException` + 5 lớp con: Invalid, FileNotFound, Auth, Budget, Balance | `exceptions.h` : 11 |
| 8 | **Operator Overloading** | `operator<<` / `>>` (serialization), `operator==` / `!=` (so sánh ngữ nghĩa) | `task.cpp` : 90–124 |

📄 Xem báo cáo đầy đủ với code snippet và giải thích: [`docs/oop_compliance_report.md`](docs/oop_compliance_report.md)

📋 Tóm tắt nhanh cho vấn đáp: [`docs/oop_compliance_summary.md`](docs/oop_compliance_summary.md)

---

## 📐 Sơ Đồ UML

### Class Diagram

Bao gồm **23 class/enum** trong **5 package**, **21 quan hệ** (inheritance, composition, aggregation, dependency):

📊 [`docs/class_diagram.puml`](docs/class_diagram.puml)

```
core (xanh)          task (xanh lá)       money (vàng)
├── Record            ├── Priority          ├── TransactionType
├── OPEPException     ├── Task              ├── Transaction
└── 5 Exceptions      └── TaskManager       ├── Wallet
                                            ├── CategoryBudget
data (hồng)          ui (tím)              └── Budget
├── User              ├── MenuItem
├── Database<T>       ├── Menu
├── AuthManager       ├── Dashboard
└── Encryption        └── InputValidator
```

> **Render diagram:** Dùng extension [PlantUML](https://marketplace.visualstudio.com/items?itemName=jebbs.plantuml) trong VS Code, hoặc paste nội dung vào [plantuml.com/plantuml](https://www.plantuml.com/plantuml/uml/).

### Sequence Diagrams

| Diagram | File | Mô tả |
|---|---|---|
| Luồng đăng nhập | [`docs/sequence_login.puml`](docs/sequence_login.puml) | `showAuthMenu` → `AuthManager::login` → decrypt/hash → `runSession` → `Dashboard::render` |
| Luồng thêm chi tiêu | [`docs/sequence_add_expense.puml`](docs/sequence_add_expense.puml) | InputValidator → Budget pre-check → `Wallet::addExpense` → encrypt & save |

---

## 🛠️ Công Nghệ Sử Dụng

| Thành phần | Công nghệ | Lý do chọn |
|---|---|---|
| **Ngôn ngữ** | C++17 | Yêu cầu đồ án; `std::filesystem`, structured bindings, `if constexpr` |
| **Thư viện** | STL thuần | `vector`, `map`, `memory` (smart ptr), `fstream`, `functional`, `algorithm` |
| **Build** | GNU Make | Đơn giản, portable, auto-detect Windows/Linux/macOS |
| **Mã hóa** | XOR + Caesar + hex | Đủ cho dữ liệu học thuật; không cần thư viện ngoài |
| **Editor** | VS Code + Claude Code | AI-assisted development, PlantUML preview |
| **VCS** | Git + GitHub | Quản lý lịch sử, làm việc nhóm |
| **Docs** | PlantUML + Markdown | Diagram dạng text, version-controllable |

---

## 👥 Thành Viên Nhóm

| STT | Họ và Tên | MSSV | Vai Trò | Đóng Góp Chính |
|:---:|---|---|---|---|
| 1 | [Tên TV1] | [MSSV1] | 🏗️ Architect / Lead Dev | Thiết kế kiến trúc, code 5 module, tích hợp `main.cpp` |
| 2 | [Tên TV2] | [MSSV2] | 🧪 Tester | Unit test, phát hiện và fix bug, kiểm thử đa nền tảng |
| 3 | [Tên TV3] | [MSSV3] | 📐 Designer | Vẽ UML (class/sequence diagram), review kiến trúc |
| 4 | [Tên TV4] | [MSSV4] | ✍️ Technical Writer | Báo cáo OOP, README, tài liệu kỹ thuật |
| 5 | [Tên TV5] | [MSSV5] | 🎤 Presenter | Slide thuyết trình, video demo, chuẩn bị vấn đáp |

> 📝 **Hướng dẫn cập nhật:** Thay `[Tên TVx]` và `[MSSVx]` bằng thông tin thật của nhóm.

---

## 🗺️ Roadmap

Các tính năng có thể mở rộng trong tương lai:

### Phiên bản tiếp theo (v2.0)

- [ ] **Export báo cáo CSV/PDF** — xuất thống kê thu/chi ra file
- [ ] **Biểu đồ ASCII nâng cao** — bar chart thu/chi theo tháng trong terminal
- [ ] **Nhắc nhở task** — thông báo khi mở app nếu có deadline hôm nay
- [ ] **Tìm kiếm và lọc** — lọc giao dịch theo danh mục, khoảng thời gian, số tiền

### Phiên bản dài hạn (v3.0+)

- [ ] **GUI với Qt** — giao diện đồ hoạ giữ nguyên backend C++
- [ ] **Đồng bộ cloud** — backup dữ liệu lên server qua REST API
- [ ] **Mobile companion** — app xem nhanh trên Android/iOS
- [ ] **Multi-currency** — hỗ trợ USD, EUR với tỷ giá tự động

---

## 🤝 Đóng Góp

### Quy trình tạo Pull Request

```bash
# 1. Fork repo và clone về máy
git clone https://github.com/<your-username>/OPEP_project.git

# 2. Tạo branch mới từ main
git checkout -b feat/ten-tinh-nang

# 3. Commit theo quy ước (xem bên dưới)
git commit -m "feat: thêm tính năng export CSV"

# 4. Push và tạo PR
git push origin feat/ten-tinh-nang
# → Mở GitHub → Compare & pull request
```

### Quy ước commit message

| Prefix | Ý nghĩa | Ví dụ |
|---|---|---|
| `feat:` | Tính năng mới | `feat: thêm export CSV cho giao dịch` |
| `fix:` | Sửa bug | `fix: budget không lưu khi thoát phiên` |
| `docs:` | Cập nhật tài liệu | `docs: bổ sung sequence diagram login` |
| `refactor:` | Cải thiện code (không đổi hành vi) | `refactor: tách pickCategory ra helper riêng` |
| `test:` | Thêm / sửa test | `test: unit test cho Budget::recordExpense` |
| `chore:` | Việc vặt (build, CI, gitignore...) | `chore: cập nhật Makefile cho macOS ARM` |

### Quy ước code

```
Tên class, struct, enum  →  PascalCase    (Task, TransactionType, CategoryBudget)
Tên hàm, biến           →  camelCase     (addExpense, totalIncome, nextTaskId)
Tên file                →  snake_case    (task_manager.cpp, file_manager.h)
Namespace               →  opep
Comment giải thích      →  Tiếng Việt
Tên hàm, biến           →  Tiếng Anh
```

---

## 📜 License

Dự án này là **đồ án học thuật** thuộc môn **Lập trình Hướng đối tượng C++**.

- ✅ Được phép tham khảo, học tập, và trích dẫn (kèm nguồn)
- ❌ Không được dùng cho mục đích thương mại
- ❌ Không được nộp nguyên như đồ án của người khác (academic integrity)

---

## 📬 Liên Hệ

| | |
|---|---|
| 📧 **Email nhóm** | `` |
| 🐙 **GitHub** | `https://github.com/Turnippii/OPEP_project` |
| 🏫 **Trường** | Đại học Công nghệ kỹ thuật TP HCM |
| 📚 **Môn học** | Lập trình Hướng đối tượng và kỹ thuật phần mềm|
|

---

<div align="center">

Made with ❤️ by **Nhóm OPEP** · C++17 · GNU Make · PlantUML

*"Viết code sạch — không dùng raw pointer, không dùng magic number, không dùng thư viện ngoài."*

</div>
