#ifndef OPEP_GUI_MAINWINDOW_H
#define OPEP_GUI_MAINWINDOW_H

#include "appservice.h"

#include <QMainWindow>

class QComboBox;
class QLineEdit;

class QLabel;
class QProgressBar;
class QTableWidget;
class DonutChartWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    AppService& service;

    QLabel* userLabel;
    QLabel* balanceLabel;
    QLabel* incomeMonthLabel;
    QLabel* expenseMonthLabel;
    QLabel* netMonthLabel;
    QLabel* taskSummaryLabel;
    QLabel* budgetSummaryLabel;
    QLabel* emptyChartLabel;

    QProgressBar* budgetProgress;
    QProgressBar* taskProgress;
    DonutChartWidget* expenseChart;

    QTableWidget* transactionTable;
    QTableWidget* taskTable;
    QTableWidget* budgetTable;
    QTableWidget* recentTransactionTable;
    QTableWidget* upcomingTaskTable;

    QLineEdit* transactionMonthFilter;
    QComboBox* transactionTypeFilter;
    QComboBox* transactionCategoryFilter;
    QLineEdit* transactionKeywordFilter;
    QLabel* transactionFilterSummary;

    QWidget* createDashboardTab();
    QWidget* createTransactionTab();
    QWidget* createTaskTab();
    QWidget* createAccountTab();

    int selectedId(QTableWidget* table) const;
    QString formatMoney(double value) const;
    void showError(const QString& title, const std::exception& e);
    void applyModernTheme();
    void setupTable(QTableWidget* table);
    void setTableText(QTableWidget* table, int row, int column, const QString& text, Qt::Alignment alignment = Qt::AlignVCenter | Qt::AlignLeft);
    void refreshTransactionTable();
    void updateTransactionCategoryFilter();
    bool transactionMatchesFilters(const opep::Transaction& transaction) const;

private slots:
    void refreshAll();
    void clearTransactionFilters();
    void addIncome();
    void addExpense();
    void addTask();
    void markSelectedTaskDone();
    void deleteSelectedTask();
    void sortTasks();
    void setBudgetLimit();
    void removeBudgetLimit();
    void changePassword();
    void logout();

public:
    explicit MainWindow(AppService& service, QWidget* parent = nullptr);
};

#endif // OPEP_GUI_MAINWINDOW_H
