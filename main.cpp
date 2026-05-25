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
#include <ctime>

namespace fs = std::filesystem;
using namespace opep;

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
            std::cout << "  [!] Canh bao load giao dich: " << e.what() << "\n";
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
            std::cout << "  [!] Canh bao load task: " << e.what() << "\n";
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
    taskMenu.addItem("Xem tat ca task", [&]() {
        std::cout << "\n=== DANH SACH TASK ===\n";
        if (tm.count() == 0)
            std::cout << "  (Chua co task nao)\n";
        else
            tm.displayAll();
        std::cout << "  Hoan thanh: " << tm.countCompleted()
                  << "/" << tm.count() << "\n";
        InputValidator::pause();
    });

    taskMenu.addItem("Them task moi", [&]() {
        std::cout << "\n=== THEM TASK MOI ===\n";
        std::string title    = InputValidator::readString("  Ten task: ");
        std::string deadline = InputValidator::readDate(
            "  Han chot YYYY-MM-DD (Enter bo qua): ", true);
        std::cout << "  Uu tien  [1]LOW  [2]MEDIUM  [3]HIGH: ";
        int p = InputValidator::readInt("", 1, 3);
        Priority pr = (p == 1) ? Priority::LOW
                    : (p == 3) ? Priority::HIGH
                    : Priority::MEDIUM;

        tm.addTask(std::make_unique<Task>(nextTaskId++, today(), title, pr, deadline));
        saveData(username, wallet, tm);
        std::cout << "  [OK] Da them task!\n";
        InputValidator::pause();
    });

    taskMenu.addItem("Danh dau hoan thanh", [&]() {
        std::cout << "\n=== DANH DAU HOAN THANH ===\n";
        if (tm.count() == 0) { std::cout << "  Chua co task nao.\n"; InputValidator::pause(); return; }
        tm.displayAll();
        int id = InputValidator::readInt("  Nhap ID task: ", 1, 99999);
        tm.markDone(id);
        saveData(username, wallet, tm);
        std::cout << "  [OK] Da danh dau hoan thanh!\n";
        InputValidator::pause();
    });

    taskMenu.addItem("Xoa task", [&]() {
        std::cout << "\n=== XOA TASK ===\n";
        if (tm.count() == 0) { std::cout << "  Chua co task nao.\n"; InputValidator::pause(); return; }
        tm.displayAll();
        int id = InputValidator::readInt("  Nhap ID task can xoa: ", 1, 99999);
        if (InputValidator::confirm("  Chac chan muon xoa task nay?")) {
            tm.removeTask(id);
            saveData(username, wallet, tm);
            std::cout << "  [OK] Da xoa task!\n";
        }
        InputValidator::pause();
    });

    taskMenu.addItem("Sap xep theo uu tien", [&]() {
        tm.sortByPriority();
        saveData(username, wallet, tm);
        std::cout << "\n  [OK] Da sap xep: HIGH -> MEDIUM -> LOW\n";
        tm.displayAll();
        InputValidator::pause();
    });
}

// ── Sub-menu: Money ───────────────────────────────────────────────────────────

static void buildMoneyMenu(Menu&              moneyMenu,
                            Wallet&            wallet,
                            Budget&            budget,
                            const std::string& username,
                            const TaskManager& tm)
{
    moneyMenu.addItem("Xem giao dich", [&]() {
        std::cout << "\n=== DANH SACH GIAO DICH ===\n";
        const auto& txns = wallet.getTransactions();
        if (txns.empty())
            std::cout << "  (Chua co giao dich nao)\n";
        else
            for (const auto& t : txns) t->displayInfo();
        std::cout << "\n";
        wallet.displaySummary();
        InputValidator::pause();
    });

    moneyMenu.addItem("Them thu nhap", [&]() {
        std::cout << "\n=== THEM THU NHAP ===\n";
        double      amt   = InputValidator::readDouble("  So tien (VND): ", 1.0);
        std::string cat   = InputValidator::readString("  Danh muc (Luong/Thuong/Khac): ");
        std::string title = InputValidator::readString("  Mo ta: ");
        std::string note  = InputValidator::readString("  Ghi chu (Enter bo qua): ", 0);

        wallet.addIncome(amt, cat, title, today(), note);
        saveData(username, wallet, tm);
        std::cout << "  [OK] Da ghi thu nhap!\n";
        InputValidator::pause();
    });

    moneyMenu.addItem("Them chi tieu", [&]() {
        std::cout << "\n=== THEM CHI TIEU ===\n";
        double      amt   = InputValidator::readDouble("  So tien (VND): ", 1.0);
        std::string cat   = InputValidator::readString("  Danh muc (An uong/Di lai/Mua sam): ");
        std::string title = InputValidator::readString("  Mo ta: ");
        std::string note  = InputValidator::readString("  Ghi chu (Enter bo qua): ", 0);

        // Ghi vào budget trước (có thể ném BudgetExceededException)
        bool warn = false;
        bool budgetOk = true;
        try {
            warn = budget.recordExpense(cat, amt);
        } catch (const BudgetExceededException& e) {
            std::cout << "  [!] " << e.what() << "\n";
            budgetOk = InputValidator::confirm("  Tiep tuc chi vuot han muc?");
            if (!budgetOk) { InputValidator::pause(); return; }
        }

        wallet.addExpense(amt, cat, title, today(), note);
        saveData(username, wallet, tm);
        std::cout << "  [OK] Da ghi chi tieu!\n";
        if (warn) std::cout << "  [!] CANH BAO: Da dung >= 80% han muc '" << cat << "'!\n";
        InputValidator::pause();
    });

    moneyMenu.addSeparator();

    moneyMenu.addItem("Xem ngan sach thang nay", [&]() {
        budget.syncFromTransactions(wallet.getTransactions());
        std::cout << "\n";
        budget.displayBudget();
        InputValidator::pause();
    });

    moneyMenu.addItem("Dat han muc danh muc", [&]() {
        std::cout << "\n=== DAT HAN MUC NGAN SACH ===\n";
        std::string cat = InputValidator::readString("  Ten danh muc: ");
        double      lim = InputValidator::readDouble("  Han muc (VND/thang): ", 0.0);
        budget.setLimit(cat, lim);
        std::cout << "  [OK] Han muc " << cat << " = " << lim << " VND/thang\n";
        InputValidator::pause();
    });
}

// ── Session: sau khi đăng nhập ────────────────────────────────────────────────

static void runSession(std::shared_ptr<User> user, AuthManager& auth) {
    const std::string& uname = user->getUsername();

    Wallet      wallet(uname, 0.0);
    TaskManager tm;
    Budget      budget(currentMonth());
    int         nextTaskId = 1;

    std::cout << "\n  Dang tai du lieu...\n";
    loadData(uname, wallet, tm, nextTaskId);
    budget.syncFromTransactions(wallet.getTransactions());

    // Dashboard lần đầu
    Dashboard(wallet, tm, uname).render();

    // ── Menu chính ────────────────────────────────────────────────────────────
    Menu mainMenu("MENU CHINH  [" + uname + "]", "Dang xuat");

    Menu taskMenu("QUAN LY TASK");
    buildTaskMenu(taskMenu, tm, nextTaskId, uname, wallet);
    mainMenu.addItem("Quan ly Task", [&]() { taskMenu.run(); });

    Menu moneyMenu("QUAN LY TAI CHINH");
    buildMoneyMenu(moneyMenu, wallet, budget, uname, tm);
    mainMenu.addItem("Quan ly Tai chinh", [&]() { moneyMenu.run(); });

    mainMenu.addSeparator();

    mainMenu.addItem("Lam moi Dashboard", [&]() {
        budget.syncFromTransactions(wallet.getTransactions());
        Dashboard(wallet, tm, uname).render();
        InputValidator::pause();
    });

    mainMenu.addItem("Doi mat khau", [&]() {
        std::cout << "\n=== DOI MAT KHAU ===\n";
        std::string oldPw = InputValidator::readPassword("  Mat khau hien tai: ", 1);
        std::string newPw = InputValidator::readPassword("  Mat khau moi: ");
        std::string cfmPw = InputValidator::readPassword("  Nhap lai mat khau moi: ");
        if (newPw != cfmPw) {
            std::cout << "  [!] Mat khau nhap lai khong khop!\n";
        } else {
            auth.changePassword(uname, oldPw, newPw);
            std::cout << "  [OK] Da doi mat khau thanh cong!\n";
        }
        InputValidator::pause();
    });

    mainMenu.run();

    // Lưu khi thoát phiên
    std::cout << "\n  Dang luu du lieu...\n";
    saveData(uname, wallet, tm);
    std::cout << "  Tam biet, " << uname << "!\n\n";
}

// ── Auth flow ─────────────────────────────────────────────────────────────────

static void showAuthMenu(AuthManager& auth) {
    Menu authMenu("OPEP  -  PERSONAL BUDGET & TASK MANAGER", "Thoat");

    authMenu.addItem("Dang nhap", [&]() {
        std::cout << "\n=== DANG NHAP ===\n";
        std::string uname = InputValidator::readString("  Username: ", 3, 20);
        std::string pw    = InputValidator::readPassword("  Password: ", 1);
        auto user = auth.login(uname, pw);
        std::cout << "  [OK] Dang nhap thanh cong! Xin chao, " << uname << "!\n";
        runSession(user, auth);
    });

    authMenu.addItem("Dang ky tai khoan", [&]() {
        std::cout << "\n=== DANG KY ===\n";
        std::string uname = InputValidator::readString("  Username (3-20 ky tu, a-z 0-9 _): ", 3, 20);
        std::string pw    = InputValidator::readPassword("  Password (it nhat 6 ky tu): ");
        std::string cfm   = InputValidator::readPassword("  Nhap lai password: ");
        if (pw != cfm) {
            std::cout << "  [!] Password nhap lai khong khop!\n";
            InputValidator::pause();
            return;
        }
        auth.registerUser(uname, pw);
        std::cout << "  [OK] Dang ky thanh cong! Moi ban dang nhap.\n";
        InputValidator::pause();
    });

    authMenu.run();
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
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
