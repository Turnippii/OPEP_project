#include "include/core/exceptions.h"
#include "include/data/auth.h"
#include "include/data/file_manager.h"
#include "include/money/transaction.h"
#include "include/money/wallet.h"
#include "include/money/budget.h"
#include "include/task/task.h"
#include "include/task/task_manager.h"
#include "include/ui/dashboard.h"
#include "include/ui/menu.h"
#include "include/ui/input_validator.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <ctime>
#include <clocale>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace fs = std::filesystem;
using namespace opep;

static void setupConsoleUtf8() {
    std::setlocale(LC_ALL, "");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string today() {
    std::time_t t  = std::time(nullptr);
    std::tm*    tm = std::localtime(&t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return buf;
}

static std::string currentMonth() { return today().substr(0, 7); }

static std::string userDir(const std::string& username) {
    return "data/" + username + "/";
}

// ── Persistence ───────────────────────────────────────────────────────────────

static void saveData(const std::string& username,
                     const Wallet&      wallet,
                     const TaskManager& tm)
{
    fs::create_directories(userDir(username));

    // Ghi transactions
    std::vector<Transaction> txns;
    for (const auto& t : wallet.getTransactions())
        txns.push_back(*t);
    Database<Transaction>(userDir(username) + "transactions.db").save(txns);

    // Ghi tasks
    std::vector<Task> tasks;
    for (const auto& t : tm.getAll())
        tasks.push_back(*t);
    Database<Task>(userDir(username) + "tasks.db").save(tasks);
}

static void loadData(const std::string& username,
                     Wallet&      wallet,
                     TaskManager& tm,
                     int&         nextTaskId)
{
    Database<Transaction> txnDb(userDir(username) + "transactions.db");
    if (txnDb.exists()) {
        try {
            for (auto& t : txnDb.load())
                wallet.loadTransaction(t);
        } catch (const std::exception& e) {
            std::cout << "  [!] Cảnh báo load giao dịch: " << e.what() << "\n";
        }
    }

    Database<Task> taskDb(userDir(username) + "tasks.db");
    if (taskDb.exists()) {
        try {
            for (auto& t : taskDb.load()) {
                if (t.getId() >= nextTaskId) nextTaskId = t.getId() + 1;
                tm.addTask(std::make_unique<Task>(t));
            }
        } catch (const std::exception& e) {
            std::cout << "  [!] Cảnh báo load task: " << e.what() << "\n";
        }
    }
}

// ── Sub-menu: Task ────────────────────────────────────────────────────────────

static void buildTaskMenu(Menu&              taskMenu,
                          TaskManager&       tm,
                          int&               nextTaskId,
                          const std::string& username,
                          const Wallet&      wallet)
{
    taskMenu.addItem("Xem tất cả task", [&]() {
        std::cout << "\n=== DANH SÁCH TASK ===\n";
        if (tm.count() == 0)
            std::cout << "  (Chưa có task nào)\n";
        else
            tm.displayAll();
        std::cout << "  Hoàn thành: " << tm.countCompleted()
                  << "/" << tm.count() << "\n";
        InputValidator::pause();
    });

    taskMenu.addItem("Thêm task mới", [&]() {
        std::cout << "\n=== THÊM TASK MỚI ===\n";
        std::string title    = InputValidator::readString("  Tên task: ");
        std::string deadline = InputValidator::readDate(
            "  Hạn chót YYYY-MM-DD (Enter bỏ qua): ", true);
        std::cout << "  Ưu tiên  [1]LOW  [2]MEDIUM  [3]HIGH: ";
        int p = InputValidator::readInt("", 1, 3);
        Priority pr = (p == 1) ? Priority::LOW
                    : (p == 3) ? Priority::HIGH
                    : Priority::MEDIUM;

        tm.addTask(std::make_unique<Task>(nextTaskId++, today(), title, pr, deadline));
        saveData(username, wallet, tm);
        std::cout << "  [OK] Đã thêm task!\n";
        InputValidator::pause();
    });

    taskMenu.addItem("Đánh dấu hoàn thành", [&]() {
        std::cout << "\n=== ĐÁNH DẤU HOÀN THÀNH ===\n";
        if (tm.count() == 0) { std::cout << "  Chưa có task nào.\n"; InputValidator::pause(); return; }
        tm.displayAll();
        int id = InputValidator::readInt("  Nhập ID task: ", 1, 99999);
        tm.markDone(id);
        saveData(username, wallet, tm);
        std::cout << "  [OK] Đã đánh dấu hoàn thành!\n";
        InputValidator::pause();
    });

    taskMenu.addItem("Xóa task", [&]() {
        std::cout << "\n=== XÓA TASK ===\n";
        if (tm.count() == 0) { std::cout << "  Chưa có task nào.\n"; InputValidator::pause(); return; }
        tm.displayAll();
        int id = InputValidator::readInt("  Nhập ID task cần xóa: ", 1, 99999);
        if (InputValidator::confirm("  Chắc chắn muốn xóa task này?")) {
            tm.removeTask(id);
            saveData(username, wallet, tm);
            std::cout << "  [OK] Đã xóa task!\n";
        }
        InputValidator::pause();
    });

    taskMenu.addItem("Sắp xếp theo ưu tiên", [&]() {
        tm.sortByPriority();
        saveData(username, wallet, tm);
        std::cout << "\n  [OK] Đã sắp xếp: HIGH -> MEDIUM -> LOW\n";
        tm.displayAll();
        InputValidator::pause();
    });
}

// ── Chọn danh mục từ lịch sử hoặc nhập mới ──────────────────────────────────
// Thu thập category đã dùng trong wallet, hiển thị danh sách đánh số,
// cho phép chọn theo số hoặc gõ tên mới.

static std::string pickCategory(const Wallet& wallet) {
    // Thu thập danh mục duy nhất từ lịch sử giao dịch (giữ thứ tự xuất hiện)
    std::vector<std::string> cats;
    for (const auto& t : wallet.getTransactions()) {
        const std::string& c = t->getCategory();
        if (std::find(cats.begin(), cats.end(), c) == cats.end())
            cats.push_back(c);
    }

    if (cats.empty())
        return InputValidator::readString("  Danh mục: ");

    std::cout << "  Danh mục có sẵn:\n";
    for (std::size_t i = 0; i < cats.size(); ++i)
        std::cout << "    [" << (i + 1) << "] " << cats[i] << "\n";
    std::cout << "    [" << (cats.size() + 1) << "] Nhập danh mục mới\n";

    int choice = InputValidator::readInt(
        "  Chọn (1-" + std::to_string(cats.size() + 1) + "): ",
        1, static_cast<int>(cats.size() + 1)
    );

    if (choice == static_cast<int>(cats.size() + 1))
        return InputValidator::readString("  Tên danh mục mới: ");
    return cats[choice - 1];
}

// ── Sub-menu: Money ───────────────────────────────────────────────────────────

static void buildMoneyMenu(Menu&              moneyMenu,
                            Wallet&            wallet,
                            Budget&            budget,
                            const std::string& username,
                            const TaskManager& tm)
{
    moneyMenu.addItem("Xem giao dịch", [&]() {
        std::cout << "\n=== DANH SÁCH GIAO DỊCH ===\n";
        const auto& txns = wallet.getTransactions();
        if (txns.empty())
            std::cout << "  (Chưa có giao dịch nào)\n";
        else
            for (const auto& t : txns) t->displayInfo();
        std::cout << "\n";
        wallet.displaySummary();
        InputValidator::pause();
    });

    moneyMenu.addItem("Thêm thu nhập", [&]() {
        std::cout << "\n=== THÊM THU NHẬP ===\n";
        double      amt   = InputValidator::readDouble("  Số tiền (VND): ", 1.0);
        std::string cat   = InputValidator::readString("  Danh mục (Lương/Thưởng/Khác): ");
        std::string title = InputValidator::readString("  Mô tả: ");
        std::string note  = InputValidator::readString("  Ghi chú (Enter bỏ qua): ", 0);

        wallet.addIncome(amt, cat, title, today(), note);
        saveData(username, wallet, tm);
        std::cout << "  [OK] Đã ghi thu nhập!\n";
        InputValidator::pause();
    });

    moneyMenu.addItem("Thêm chi tiêu", [&]() {
        std::cout << "\n=== THÊM CHI TIÊU ===\n";
        double      amt   = InputValidator::readDouble("  Số tiền (VND): ", 1.0);
        std::string cat   = pickCategory(wallet);
        std::string title = InputValidator::readString("  Mô tả: ");
        std::string note  = InputValidator::readString("  Ghi chú (Enter bỏ qua): ", 0);

        // Cảnh báo ngân sách trước khi ghi; hỏi y/n nếu >= 80% hạn mức
        bool proceed = true;
        try {
            const auto& cb = budget.getCategory(cat);
            if (cb.limit > 0.0) {
                double projected = cb.spent + amt;
                double ratio     = projected / cb.limit;
                if (ratio >= 1.0) {
                    long long over = static_cast<long long>(projected - cb.limit);
                    long long lim  = static_cast<long long>(cb.limit);
                    std::cout << "  [!!] VƯỢT HẠN MỨC: Hạn mức '" << cat
                              << "' là " << lim << " VND, bạn đang chi vượt "
                              << over << " VND\n";
                    proceed = InputValidator::confirm("  Vẫn muốn ghi?");
                } else if (ratio >= WARNING_THRESHOLD) {
                    int pct       = static_cast<int>(ratio * 100);
                    long long proj = static_cast<long long>(projected);
                    long long lim  = static_cast<long long>(cb.limit);
                    std::cout << "  [!] CẢNH BÁO: Chi tiêu này sẽ đạt " << pct
                              << "% hạn mức '" << cat << "' ("
                              << proj << "/" << lim << " VND)\n";
                    proceed = InputValidator::confirm("  Vẫn muốn ghi?");
                }
            }
        } catch (const InvalidInputException&) { /* chưa có ngân sách cho danh mục này */ }

        if (!proceed) { InputValidator::pause(); return; }

        // Cập nhật ngân sách (nếu vượt hạn mức và người dùng đã xác nhận thì bỏ qua exception)
        try {
            budget.recordExpense(cat, amt);
        } catch (const BudgetExceededException&) { /* người dùng đã xác nhận ở trên */ }

        wallet.addExpense(amt, cat, title, today(), note);
        saveData(username, wallet, tm);
        std::cout << "  [OK] Đã ghi chi tiêu!\n";
        InputValidator::pause();
    });

    moneyMenu.addSeparator();

    moneyMenu.addItem("Xem ngân sách tháng này", [&]() {
        budget.syncFromTransactions(wallet.getTransactions());
        std::cout << "\n";
        budget.displayBudget();
        InputValidator::pause();
    });

    moneyMenu.addItem("Đặt hạn mức danh mục", [&]() {
        std::cout << "\n=== ĐẶT HẠN MỨC NGÂN SÁCH ===\n";
        std::string cat = InputValidator::readString("  Tên danh mục: ");
        double      lim = InputValidator::readDouble("  Hạn mức (VND/tháng): ", 0.0);
        budget.setLimit(cat, lim);
        budget.saveLimits(userDir(username) + "budget.cfg");
        std::cout << "  [OK] Hạn mức " << cat << " = " << lim << " VND/tháng\n";
        InputValidator::pause();
    });

    moneyMenu.addItem("Xóa hạn mức danh mục", [&]() {
        std::cout << "\n=== XÓA HẠN MỨC NGÂN SÁCH ===\n";
        budget.syncFromTransactions(wallet.getTransactions());
        budget.displayBudget();

        std::string cat = InputValidator::readString("  Tên danh mục cần xóa hạn mức: ");
        if (!InputValidator::confirm("  Chắc chắn muốn xóa hạn mức của danh mục này?")) {
            InputValidator::pause();
            return;
        }

        bool removed = budget.removeLimit(cat);
        budget.syncFromTransactions(wallet.getTransactions());
        budget.saveLimits(userDir(username) + "budget.cfg");

        if (removed)
            std::cout << "  [OK] Đã xóa hạn mức của danh mục '" << cat << "'.\n";
        else
            std::cout << "  [!] Danh mục '" << cat << "' hiện chưa có hạn mức để xóa.\n";

        InputValidator::pause();
    });
}

// ── Demo đa hình động ─────────────────────────────────────────────────────────
// Minh họa: cùng một dòng r->displayInfo() nhưng runtime gọi đúng phiên bản
// của Task hoặc Transaction nhờ vtable — không cần biết kiểu thực của đối tượng.

static void demoPolymorphism(const TaskManager& taskMgr, const Wallet& wallet) {
    std::cout << "\n";
    std::cout << "===== DEMO ĐA HÌNH ĐỘNG (Dynamic Polymorphism) =====\n";
    std::cout << "  Cùng 1 dòng code  r->displayInfo()  nhưng output khác nhau\n";
    std::cout << "  tùy đối tượng thật là Task hay Transaction.\n";
    std::cout << "  Đây là đa hình động qua vtable (virtual dispatch).\n";
    std::cout << "-----------------------------------------------------\n";

    // Thu thập con trỏ Record* trỏ tới các đối tượng thực trong bộ nhớ.
    // Vector này KHÔNG sở hữu bộ nhớ — lifetime do TaskManager/Wallet quản lý.
    std::vector<const Record*> records;

    // Lấy tối đa 3 Task (kiểu thực: Task, kế thừa Record)
    const auto& tasks = taskMgr.getAll();
    std::size_t takeTasks = std::min(tasks.size(), std::size_t{3});
    for (std::size_t i = 0; i < takeTasks; ++i)
        records.push_back(tasks[i].get());   // unique_ptr<Task> → Task* → Record*

    // Lấy tối đa 3 Transaction (kiểu thực: Transaction, kế thừa Record)
    const auto& txns = wallet.getTransactions();
    std::size_t takeTxns = std::min(txns.size(), std::size_t{3});
    for (std::size_t i = 0; i < takeTxns; ++i)
        records.push_back(txns[i].get());    // shared_ptr<Transaction> → Transaction* → Record*

    if (records.empty()) {
        std::cout << "  (Chưa có dữ liệu. Hãy thêm Task hoặc Giao dịch trước.)\n";
    } else {
        std::cout << "  " << records.size()
                  << " đối tượng (Task + Transaction) qua con trỏ Record*:\n\n";

        for (const Record* r : records) {
            // *** DÒNG THỂ HIỆN ĐA HÌNH ĐỘNG ***
            // Con trỏ 'r' có kiểu Record* nhưng đối tượng thật là Task hoặc Transaction.
            // C++ tra cứu vtable tại runtime để gọi đúng displayInfo() của lớp con —
            // một dòng code, nhiều hành vi khác nhau.
            r->displayInfo();
        }
    }

    std::cout << "\n-----------------------------------------------------\n";
    std::cout << "  Dòng code thể hiện đa hình động (main.cpp):\n";
    std::cout << "    for (const Record* r : records) r->displayInfo();\n";
    std::cout << "  -> r chỉ biết kiểu là Record*, nhưng vtable dispatch\n";
    std::cout << "     đúng phiên bản displayInfo() của kiểu thật lúc runtime.\n";
    std::cout << "=====================================================\n\n";
    InputValidator::pause();
}

// ── Session: sau khi đăng nhập ────────────────────────────────────────────────

static void runSession(std::shared_ptr<User> user, AuthManager& auth) {
    const std::string& uname = user->getUsername();

    Wallet      wallet(uname, 0.0);
    TaskManager tm;
    Budget      budget(currentMonth());
    int         nextTaskId = 1;

    std::cout << "\n  Đang tải dữ liệu...\n";
    loadData(uname, wallet, tm, nextTaskId);
    budget.loadLimits(userDir(uname) + "budget.cfg");
    budget.syncFromTransactions(wallet.getTransactions());

    // Dashboard lần đầu
    Dashboard(wallet, tm, uname).render();

    // ── Menu chính ────────────────────────────────────────────────────────────
    Menu mainMenu("MENU CHÍNH  [" + uname + "]", "Đăng xuất");

    Menu taskMenu("QUẢN LÝ TASK");
    buildTaskMenu(taskMenu, tm, nextTaskId, uname, wallet);
    mainMenu.addItem("Quản lý Task", [&]() { taskMenu.run(); });

    Menu moneyMenu("QUẢN LÝ TÀI CHÍNH");
    buildMoneyMenu(moneyMenu, wallet, budget, uname, tm);
    mainMenu.addItem("Quản lý Tài chính", [&]() { moneyMenu.run(); });

    mainMenu.addSeparator();

    mainMenu.addItem("Làm mới Dashboard", [&]() {
        budget.syncFromTransactions(wallet.getTransactions());
        Dashboard(wallet, tm, uname).render();
        InputValidator::pause();
    });

    mainMenu.addItem("Demo Polymorphism (xem đa hình động)", [&]() {
        demoPolymorphism(tm, wallet);
    });

    mainMenu.addItem("Đổi mật khẩu", [&]() {
        std::cout << "\n=== ĐỔI MẬT KHẨU ===\n";
        std::string oldPw = InputValidator::readPassword("  Mật khẩu hiện tại: ", 1);
        std::string newPw = InputValidator::readPassword("  Mật khẩu mới: ");
        std::string cfmPw = InputValidator::readPassword("  Nhập lại mật khẩu mới: ");
        if (newPw != cfmPw) {
            std::cout << "  [!] Mật khẩu nhập lại không khớp!\n";
        } else {
            auth.changePassword(uname, oldPw, newPw);
            std::cout << "  [OK] Đã đổi mật khẩu thành công!\n";
        }
        InputValidator::pause();
    });

    mainMenu.run();

    // Lưu khi thoát phiên
    std::cout << "\n  Đang lưu dữ liệu...\n";
    saveData(uname, wallet, tm);
    budget.saveLimits(userDir(uname) + "budget.cfg");
    std::cout << "  Tạm biệt, " << uname << "!\n\n";
}

// ── Auth flow ─────────────────────────────────────────────────────────────────

static void showAuthMenu(AuthManager& auth) {
    Menu authMenu("OPEP  -  QUẢN LÝ TÀI CHÍNH & CÔNG VIỆC", "Thoát");

    authMenu.addItem("Đăng nhập", [&]() {
        std::cout << "\n=== ĐĂNG NHẬP ===\n";
        std::string uname = InputValidator::readString("  Username: ", 3, 20);
        std::string pw    = InputValidator::readPassword("  Password: ", 1);
        auto user = auth.login(uname, pw);
        std::cout << "  [OK] Đăng nhập thành công! Xin chào, " << uname << "!\n";
        runSession(user, auth);
    });

    authMenu.addItem("Đăng ký tài khoản", [&]() {
        std::cout << "\n=== ĐĂNG KÝ ===\n";
        std::string uname = InputValidator::readString("  Username (3-20 ký tự, a-z 0-9 _): ", 3, 20);
        std::string pw    = InputValidator::readPassword("  Password (ít nhất 6 ký tự): ");
        std::string cfm   = InputValidator::readPassword("  Nhập lại password: ");
        if (pw != cfm) {
            std::cout << "  [!] Password nhập lại không khớp!\n";
            InputValidator::pause();
            return;
        }
        auth.registerUser(uname, pw);
        std::cout << "  [OK] Đăng ký thành công! Mời bạn đăng nhập.\n";
        InputValidator::pause();
    });

    authMenu.run();
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    setupConsoleUtf8();
    std::cout << "\n";
    try {
        AuthManager auth("data/");
        showAuthMenu(auth);
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
