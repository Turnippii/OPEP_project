#include "../../include/ui/dashboard.h"
#include "../../include/money/transaction.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace opep {

// Tách year và month từ "YYYY-MM-DD" thành cặp số nguyên để so sánh an toàn
static std::pair<int,int> parseYearMonth(const std::string& d) {
    if (d.size() < 7) return {0, 0};
    try {
        return { std::stoi(d.substr(0, 4)), std::stoi(d.substr(5, 2)) };
    } catch (...) { return {0, 0}; }
}

// --- Constructor ---

Dashboard::Dashboard(const Wallet& w, const TaskManager& tm, const std::string& uname)
    : wallet(w), taskManager(tm), username(uname), today(getCurrentDate()) {}

// --- Static helpers ---

std::string Dashboard::getCurrentDate() {
    // Dùng chrono để lấy thời điểm hiện tại, sau đó chuyển sang local time
    auto now      = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);   // thread-safe trên Windows
#else
    localtime_r(&t, &local);   // thread-safe trên POSIX
#endif
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &local);
    return buf;
}

std::string Dashboard::addDays(const std::string& date, int n) {
    int y, m, d;
    std::sscanf(date.c_str(), "%d-%d-%d", &y, &m, &d);
    std::tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon  = m - 1;
    t.tm_mday = d + n;
    std::mktime(&t); // normalize (xử lý tràn tháng)
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
    return buf;
}

std::string Dashboard::getCurrentMonth(const std::string& date) {
    return date.substr(0, 7); // "YYYY-MM"
}

// --- Tính toán ---

std::vector<const Task*> Dashboard::getUpcomingTasks(int days) const {
    std::string limit = addDays(today, days);
    std::vector<const Task*> result;
    for (const auto& uptr : taskManager.getAll()) {
        const Task* t = uptr.get();
        if (t->isCompleted()) continue;
        const std::string& dl = t->getDeadline();
        if (dl.empty()) continue;
        // Deadline trong khoảng [hôm nay, hôm nay + days ngày]
        if (dl >= today && dl <= limit)
            result.push_back(t);
    }
    // Sắp xếp theo deadline gần nhất
    std::sort(result.begin(), result.end(), [](const Task* a, const Task* b) {
        return a->getDeadline() < b->getDeadline();
    });
    return result;
}

double Dashboard::getTodayExpense() const {
    double total = 0.0;
    for (const auto& t : wallet.getTransactions())
        if (t->getType() == TransactionType::EXPENSE && t->getDate() == today)
            total += t->getAmount();
    return total;
}

double Dashboard::getMonthIncome() const {
    // So sánh year và month dưới dạng số nguyên — tránh lỗi so sánh chuỗi khi date bị thừa ký tự
    auto [curY, curM] = parseYearMonth(today);
    double total = 0.0;
    for (const auto& t : wallet.getTransactions()) {
        if (t->getType() != TransactionType::INCOME) continue;
        auto [tY, tM] = parseYearMonth(t->getDate());
        if (tY == curY && tM == curM)
            total += t->getAmount();
    }
    return total;
}

double Dashboard::getMonthExpense() const {
    auto [curY, curM] = parseYearMonth(today);
    double total = 0.0;
    for (const auto& t : wallet.getTransactions()) {
        if (t->getType() != TransactionType::EXPENSE) continue;
        auto [tY, tM] = parseYearMonth(t->getDate());
        if (tY == curY && tM == curM)
            total += t->getAmount();
    }
    return total;
}

// --- Tiện ích vẽ khung ---

void Dashboard::hLine(char corner, char fill, int w) {
    std::cout << corner << std::string(w - 2, fill) << corner << "\n";
}

void Dashboard::row(const std::string& content, int w) {
    int inner = w - 4; // 2 border + 2 padding
    std::string s = content.substr(0, static_cast<size_t>(inner));
    std::cout << "|  " << std::left << std::setw(inner) << s << "  |\n";
}

void Dashboard::rowCenter(const std::string& content, int w) {
    int inner = w - 2;
    int pad   = (inner - static_cast<int>(content.size())) / 2;
    pad       = std::max(0, pad);
    std::string line = std::string(pad, ' ') + content;
    // Đảm bảo không vượt quá inner
    if (static_cast<int>(line.size()) > inner)
        line = line.substr(0, static_cast<size_t>(inner));
    std::cout << "|" << std::left << std::setw(inner) << line << "|\n";
}

std::string Dashboard::fmtMoney(double amount) {
    // Định dạng: 1500000 -> "1,500,000 VND"
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << amount;
    std::string s = oss.str();
    // Chèn dấu phẩy mỗi 3 chữ số từ phải
    int insertPos = static_cast<int>(s.size()) - 3;
    while (insertPos > 0) {
        s.insert(static_cast<size_t>(insertPos), ",");
        insertPos -= 3;
    }
    return s + " VND";
}

std::string Dashboard::fmtSigned(double amount) {
    return (amount >= 0 ? "+" : "-") + fmtMoney(std::abs(amount));
}

// --- render ---

void Dashboard::render() const {
    const int W = WIDTH;

    // ===================== HEADER =====================
    hLine('+', '=', W);
    rowCenter("OPEP - PERSONAL BUDGET & TASK MANAGER", W);
    rowCenter("Xin chao, " + username + "!   |   " + today, W);
    hLine('+', '=', W);

    // ===================== SO DU VI =====================
    row("SO DU VI", W);
    hLine('+', '-', W);
    row("  " + fmtMoney(wallet.getBalance()), W);
    row("", W);
    row("  Thu thang nay:  " + fmtSigned(getMonthIncome()), W);
    row("  Chi thang nay:  " + fmtSigned(-getMonthExpense()), W);

    // Progress bar thu/chi trong tháng
    double inc = getMonthIncome();
    double exp = getMonthExpense();
    if (inc > 0.0) {
        int barW    = 30;
        int filled  = static_cast<int>((exp / inc) * barW);
        filled      = std::min(filled, barW);
        std::string bar = "  [" + std::string(filled, '#')
                        + std::string(barW - filled, '.') + "]";
        std::string pct = std::to_string(static_cast<int>((exp / inc) * 100)) + "%";
        row(bar + "  " + pct + " da chi", W);
    }

    // ===================== TASK SAP HAN =====================
    hLine('+', '=', W);
    row("TASK SAP DEN HAN (trong 3 ngay toi)", W);
    hLine('+', '-', W);

    auto upcoming = getUpcomingTasks(3);
    if (upcoming.empty()) {
        row("  (Khong co task nao sap den han)", W);
    } else {
        for (const Task* t : upcoming) {
            // Tính số ngày còn lại
            int daysLeft = 0;
            std::string dl = t->getDeadline();
            if (dl > today) {
                // So sánh chuỗi đủ vì định dạng YYYY-MM-DD đảm bảo thứ tự
                std::string cur = today;
                while (cur < dl && daysLeft < 99) {
                    cur = addDays(cur, 1);
                    ++daysLeft;
                }
            }
            std::string prefix = (daysLeft == 0) ? "  [!!] " : "  [!]  ";
            std::string suffix = (daysLeft == 0)
                ? " (HOM NAY!)"
                : " (con " + std::to_string(daysLeft) + " ngay)";
            row(prefix + t->getTitle() + suffix, W);
        }
    }

    // ===================== CHI TIEU HOM NAY =====================
    hLine('+', '=', W);
    row("CHI TIEU HOM NAY (" + today + ")", W);
    hLine('+', '-', W);

    double todayTotal = 0.0;
    bool   hasToday   = false;
    for (const auto& t : wallet.getTransactions()) {
        if (t->getType() != TransactionType::EXPENSE) continue;
        if (t->getDate() != today) continue;
        row("  " + t->getCategory() + ": " + fmtMoney(t->getAmount()), W);
        todayTotal += t->getAmount();
        hasToday = true;
    }
    if (!hasToday) row("  (Chua co giao dich nao hom nay)", W);

    hLine('+', '-', W);
    row("  Tong chi hom nay: " + fmtMoney(todayTotal), W);

    // ===================== FOOTER =====================
    hLine('+', '=', W);

    size_t total     = taskManager.count();
    size_t completed = taskManager.countCompleted();
    std::string taskStat = "Task hoan thanh: " + std::to_string(completed)
                         + "/" + std::to_string(total);
    std::string txnStat  = "Tong giao dich: "
                         + std::to_string(wallet.getTransactions().size());
    row(taskStat + "   |   " + txnStat, W);
    hLine('+', '=', W);
    std::cout << "\n";
}

} // namespace opep
