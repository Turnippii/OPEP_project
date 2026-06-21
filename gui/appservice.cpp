#include "appservice.h"

#include "include/core/exceptions.h"

#include <ctime>
#include <filesystem>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

AppService::AppService(const std::string& dir)
    : dataDir(dir)
    , auth(dir)
    , wallet(nullptr)
    , budget(nullptr)
    , nextTaskId(1)
{}

AppService::~AppService() {
    try {
        saveSessionData();
    } catch (...) {
        // Không ném exception trong destructor.
    }
}

std::string AppService::today() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return buf;
}

std::string AppService::currentMonth() {
    return today().substr(0, 7);
}

std::string AppService::userDir() const {
    if (!currentUser) return dataDir;
    return dataDir + currentUser->getUsername() + "/";
}

void AppService::registerUser(const std::string& username, const std::string& password) {
    auth.registerUser(username, password);
}

void AppService::login(const std::string& username, const std::string& password) {
    currentUser = auth.login(username, password);
    loadSessionData();
}

void AppService::logout() {
    saveSessionData();
    currentUser.reset();
    wallet.reset();
    budget.reset();
    taskManager = opep::TaskManager{};
    nextTaskId = 1;
}

void AppService::changePassword(const std::string& oldPassword, const std::string& newPassword) {
    if (!currentUser) throw opep::AuthenticationException("Chưa đăng nhập");
    auth.changePassword(currentUser->getUsername(), oldPassword, newPassword);
}

bool AppService::isLoggedIn() const {
    return static_cast<bool>(currentUser);
}

std::string AppService::username() const {
    return currentUser ? currentUser->getUsername() : "";
}

void AppService::loadSessionData() {
    if (!currentUser) return;

    wallet = std::make_unique<opep::Wallet>(currentUser->getUsername(), 0.0);
    taskManager = opep::TaskManager{};
    budget = std::make_unique<opep::Budget>(currentMonth());
    nextTaskId = 1;

    fs::create_directories(userDir());

    opep::Database<opep::Transaction> txnDb(userDir() + "transactions.db");
    if (txnDb.exists()) {
        for (auto& t : txnDb.load()) {
            wallet->loadTransaction(t);
        }
    }

    opep::Database<opep::Task> taskDb(userDir() + "tasks.db");
    if (taskDb.exists()) {
        for (auto& t : taskDb.load()) {
            if (t.getId() >= nextTaskId) nextTaskId = t.getId() + 1;
            taskManager.addTask(std::make_unique<opep::Task>(t));
        }
    }

    budget->loadLimits(userDir() + "budget.cfg");
    budget->syncFromTransactions(wallet->getTransactions());
}

void AppService::saveSessionData() const {
    if (!currentUser || !wallet || !budget) return;

    fs::create_directories(userDir());

    std::vector<opep::Transaction> txns;
    for (const auto& t : wallet->getTransactions()) {
        txns.push_back(*t);
    }
    opep::Database<opep::Transaction>(userDir() + "transactions.db").save(txns);

    std::vector<opep::Task> tasks;
    for (const auto& t : taskManager.getAll()) {
        tasks.push_back(*t);
    }
    opep::Database<opep::Task>(userDir() + "tasks.db").save(tasks);

    budget->saveLimits(userDir() + "budget.cfg");
}

opep::Wallet& AppService::getWallet() {
    if (!wallet) throw std::runtime_error("Ví chưa được khởi tạo");
    return *wallet;
}

const opep::Wallet& AppService::getWallet() const {
    if (!wallet) throw std::runtime_error("Ví chưa được khởi tạo");
    return *wallet;
}

opep::TaskManager& AppService::getTaskManager() {
    return taskManager;
}

const opep::TaskManager& AppService::getTaskManager() const {
    return taskManager;
}

opep::Budget& AppService::getBudget() {
    if (!budget) throw std::runtime_error("Ngân sách chưa được khởi tạo");
    return *budget;
}

const opep::Budget& AppService::getBudget() const {
    if (!budget) throw std::runtime_error("Ngân sách chưa được khởi tạo");
    return *budget;
}

void AppService::addIncome(double amount,
                           const std::string& category,
                           const std::string& title,
                           const std::string& note) {
    getWallet().addIncome(amount, category, title, today(), note);
    getBudget().syncFromTransactions(getWallet().getTransactions());
    saveSessionData();
}

void AppService::addExpense(double amount,
                            const std::string& category,
                            const std::string& title,
                            const std::string& note) {
    getWallet().addExpense(amount, category, title, today(), note);
    getBudget().syncFromTransactions(getWallet().getTransactions());
    saveSessionData();
}

void AppService::addTask(const std::string& title,
                         const std::string& deadline,
                         opep::Priority priority) {
    taskManager.addTask(std::make_unique<opep::Task>(nextTaskId++, today(), title, priority, deadline));
    saveSessionData();
}

void AppService::markTaskDone(int id) {
    taskManager.markDone(id);
    saveSessionData();
}

void AppService::removeTask(int id) {
    taskManager.removeTask(id);
    saveSessionData();
}

void AppService::sortTasksByPriority() {
    taskManager.sortByPriority();
    saveSessionData();
}

void AppService::setBudgetLimit(const std::string& category, double limit) {
    getBudget().setLimit(category, limit);
    getBudget().syncFromTransactions(getWallet().getTransactions());
    saveSessionData();
}


void AppService::removeBudgetLimit(const std::string& category) {
    getBudget().removeLimit(category);
    getBudget().syncFromTransactions(getWallet().getTransactions());
    saveSessionData();
}
