#include "../../include/ui/dashboard.h"
#include "../../include/money/transaction.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace opep {

static int utf8Width(const std::string& s) {
    int w = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++w;
    return w;
}

static std::string utf8FitLeft(const std::string& s, int width) {
    if (width <= 0) return "";
    int w = utf8Width(s);
    if (w <= width) return s + std::string(width - w, ' ');

    int target = std::max(0, width - 3);
    int cur = 0;
    size_t i = 0;
    while (i < s.size() && cur < target) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t cl = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
        ++cur;
        i += cl;
    }
    return s.substr(0, i) + "...";
}

static std::string utf8Center(const std::string& s, int width) {
    int w = utf8Width(s);
    if (w >= width) return utf8FitLeft(s, width);
    int left = (width - w) / 2;
    int right = width - w - left;
    return std::string(left, ' ') + s + std::string(right, ' ');
}

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
    int inner = w - 2; // bỏ 2 ký tự viền trái/phải
    std::cout << "|" << utf8FitLeft("  " + content, inner) << "|\n";
}

void Dashboard::rowCenter(const std::string& content, int w) {
    int inner = w - 2;
    std::cout << "|" << utf8Center(content, inner) << "|\n";
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
    rowCenter("OPEP - QUẢN LÝ TÀI CHÍNH & CÔNG VIỆC", W);
    rowCenter("Xin chào, " + username + "!   |   " + today, W);
    hLine('+', '=', W);

    // ===================== SỐ DƯ VÍ =====================
    row("SỐ DƯ VÍ", W);
    hLine('+', '-', W);
    row("  " + fmtMoney(wallet.getBalance()), W);
    row("", W);
    row("  Thu tháng này:  " + fmtSigned(getMonthIncome()), W);
    row("  Chi tháng này:  " + fmtSigned(-getMonthExpense()), W);

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
        row(bar + "  " + pct + " đã chi", W);
    }

    // ===================== TASK SẮP HẠN =====================
    hLine('+', '=', W);
    row("TASK SẮP ĐẾN HẠN (trong 3 ngày tới)", W);
    hLine('+', '-', W);

    auto upcoming = getUpcomingTasks(3);
    if (upcoming.empty()) {
        row("  (Không có task nào sắp đến hạn)", W);
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
                ? " (HÔM NAY!)"
                : " (còn " + std::to_string(daysLeft) + " ngày)";
            row(prefix + t->getTitle() + suffix, W);
        }
    }

    // ===================== CHI TIÊU HÔM NAY =====================
    hLine('+', '=', W);
    row("CHI TIÊU HÔM NAY (" + today + ")", W);
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
    if (!hasToday) row("  (Chưa có giao dịch nào hôm nay)", W);

    hLine('+', '-', W);
    row("  Tổng chi hôm nay: " + fmtMoney(todayTotal), W);

    // ===================== FOOTER =====================
    hLine('+', '=', W);

    size_t total     = taskManager.count();
    size_t completed = taskManager.countCompleted();
    std::string taskStat = "Task hoàn thành: " + std::to_string(completed)
                         + "/" + std::to_string(total);
    std::string txnStat  = "Tổng giao dịch: "
                         + std::to_string(wallet.getTransactions().size());
    row(taskStat + "   |   " + txnStat, W);
    hLine('+', '=', W);
    std::cout << "\n";
}

} // namespace opep
