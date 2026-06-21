#ifndef OPEP_GUI_APPSERVICE_H
#define OPEP_GUI_APPSERVICE_H

#include "include/data/auth.h"
#include "include/data/file_manager.h"
#include "include/money/budget.h"
#include "include/money/wallet.h"
#include "include/task/task_manager.h"

#include <memory>
#include <string>

class AppService {
private:
    std::string dataDir;
    opep::AuthManager auth;
    std::shared_ptr<opep::User> currentUser;
    std::unique_ptr<opep::Wallet> wallet;
    opep::TaskManager taskManager;
    std::unique_ptr<opep::Budget> budget;
    int nextTaskId;

    static std::string today();
    static std::string currentMonth();
    std::string userDir() const;

    void loadSessionData();

public:
    explicit AppService(const std::string& dataDir = "data/");
    ~AppService();

    void registerUser(const std::string& username, const std::string& password);
    void login(const std::string& username, const std::string& password);
    void logout();
    void changePassword(const std::string& oldPassword, const std::string& newPassword);

    bool isLoggedIn() const;
    std::string username() const;

    opep::Wallet& getWallet();
    const opep::Wallet& getWallet() const;
    opep::TaskManager& getTaskManager();
    const opep::TaskManager& getTaskManager() const;
    opep::Budget& getBudget();
    const opep::Budget& getBudget() const;

    void addIncome(double amount,
                   const std::string& category,
                   const std::string& title,
                   const std::string& note);

    void addExpense(double amount,
                    const std::string& category,
                    const std::string& title,
                    const std::string& note);

    void addTask(const std::string& title,
                 const std::string& deadline,
                 opep::Priority priority);

    void markTaskDone(int id);
    void removeTask(int id);
    void sortTasksByPriority();
    void setBudgetLimit(const std::string& category, double limit);
    void removeBudgetLimit(const std::string& category);

    void saveSessionData() const;
};

#endif // OPEP_GUI_APPSERVICE_H
