#include "../../include/money/budget.h"
#include "../../include/core/exceptions.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace opep {

// Chuẩn hóa tên category: trim đầu/cuối, collapse khoảng trắng giữa, lowercase ASCII
// Giữ nguyên bytes tiếng Việt UTF-8 (multi-byte) vì tolower không áp dụng được
static std::string normalizeKey(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    std::string r;
    r.reserve(end - start + 1);
    bool prevSpace = false;
    for (size_t i = start; i <= end; ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == ' ' || c == '\t') {
            if (!prevSpace) { r.push_back(' '); prevSpace = true; }
        } else {
            r.push_back(static_cast<char>(std::tolower(c)));
            prevSpace = false;
        }
    }
    return r;
}

// Số code point UTF-8 (= số ký tự hiển thị; bỏ continuation byte 10xxxxxx)
static int utf8Width(const std::string& s) {
    int w = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++w;
    return w;
}

// Trả về chuỗi s đã cắt/pad vừa đúng `width` cột hiển thị (left-aligned)
// Nếu dài hơn → cắt ở (width-3) ký tự và thêm "..."
static std::string utf8FitLeft(const std::string& s, int width) {
    int w = utf8Width(s);
    if (w <= width)
        return s + std::string(width - w, ' ');
    // Cắt: tìm vị trí byte sau (width-3) code points
    int target = width - 3;
    int cur = 0;
    size_t i = 0;
    while (i < s.size() && cur < target) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t cl = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
        cur++;
        i += cl;
    }
    return s.substr(0, i) + "...";
}

// --- CategoryBudget ---

double CategoryBudget::usageRatio() const {
    if (limit <= 0.0) return 0.0;
    return spent / limit;
}

double CategoryBudget::remaining() const { return limit - spent; }

bool CategoryBudget::isExceeded() const {
    return limit > 0.0 && spent > limit;
}

bool CategoryBudget::isNearLimit() const {
    return limit > 0.0 && usageRatio() >= WARNING_THRESHOLD;
}

// --- Budget ---

Budget::Budget(const std::string& m) : month(m) {}

void Budget::setLimit(const std::string& category, double limit) {
    if (limit < 0.0)
        throw InvalidInputException("Hạn mức ngân sách phải >= 0");

    // Map key luôn là normalized; CategoryBudget.category giữ tên gốc để hiển thị
    std::string key = normalizeKey(category);
    auto it = categories.find(key);
    if (it != categories.end())
        it->second.limit = limit;
    else
        categories.emplace(key, CategoryBudget(category, limit));
}

bool Budget::removeLimit(const std::string& category) {
    std::string key = normalizeKey(category);
    auto it = categories.find(key);
    if (it == categories.end())
        throw InvalidInputException("Không tìm thấy danh mục: " + category);

    if (it->second.limit <= 0.0)
        return false;

    it->second.limit = 0.0;

    // Nếu danh mục này chỉ tồn tại vì hạn mức và chưa có chi tiêu trong tháng,
    // xóa hẳn khỏi bảng để giao diện gọn hơn.
    if (it->second.spent <= 0.0)
        categories.erase(it);

    return true;
}

bool Budget::recordExpense(const std::string& category, double amount) {
    if (amount < 0.0)
        throw InvalidInputException("Số tiền chi tiêu phải >= 0");

    std::string key = normalizeKey(category);
    if (!categories.count(key))
        categories.emplace(key, CategoryBudget(category, 0.0));

    auto& cb = categories.at(key);
    cb.spent += amount;  // cập nhật trước khi kiểm tra để state luôn nhất quán

    if (cb.isExceeded())
        throw BudgetExceededException(category, cb.limit, cb.spent);

    return cb.isNearLimit();
}

void Budget::syncFromTransactions(const std::vector<std::shared_ptr<Transaction>>& txns) {
    // Reset toàn bộ spent về 0 trước khi tính lại
    for (auto& [key, cb] : categories)
        cb.spent = 0.0;

    for (const auto& t : txns) {
        if (t->getType() != TransactionType::EXPENSE) continue;
        if (t->getDate().substr(0, 7) != month) continue;

        const std::string& cat = t->getCategory();
        std::string key = normalizeKey(cat);
        // Nếu chưa có entry với key này, tạo mới (giữ tên gốc để hiển thị)
        if (!categories.count(key))
            categories.emplace(key, CategoryBudget(cat, 0.0));
        categories.at(key).spent += t->getAmount();
    }
}

const CategoryBudget& Budget::getCategory(const std::string& category) const {
    auto it = categories.find(normalizeKey(category));
    if (it == categories.end())
        throw InvalidInputException("Không tìm thấy danh mục: " + category);
    return it->second;
}

double Budget::totalLimit() const {
    double sum = 0.0;
    for (const auto& [key, cb] : categories) sum += cb.limit;
    return sum;
}

double Budget::totalSpent() const {
    double sum = 0.0;
    for (const auto& [key, cb] : categories) sum += cb.spent;
    return sum;
}

const std::string& Budget::getMonth() const { return month; }

const std::map<std::string, CategoryBudget>& Budget::getCategories() const { return categories; }

// --- displayBudget: bảng ngân sách với progress bar ASCII ---

void Budget::displayBudget() const {
    // Cột: Danh mục(25) | Đã chi(15) | Hạn mức(15) | %(6) | Tiến độ(20) | Trạng thái(20)
    constexpr int W_CAT    = 25;
    constexpr int W_SPENT  = 15;
    constexpr int W_LIMIT  = 15;
    constexpr int W_PCT    = 6;
    constexpr int BAR_FILL = 18;   // ký tự bên trong "[...]" → bar dài đúng 20 char
    constexpr int W_BAR    = 20;   // "[" + BAR_FILL + "]" = 20
    constexpr int W_STATUS = 20;
    constexpr int TOTAL_W  = W_CAT + W_SPENT + W_LIMIT + W_PCT + 2 + W_BAR + W_STATUS;

    std::cout << "\n=== Ngân sách tháng " << month << " ===\n";

    // Header
    std::cout << std::left  << std::setw(W_CAT)   << "Danh mục"
              << std::right << std::setw(W_SPENT)  << "Đã chi (VND)"
              << std::setw(W_LIMIT)                << "Hạn mức (VND)"
              << std::setw(W_PCT)                  << "%"
              << "  "
              << std::left  << std::setw(W_BAR)    << "Tiến độ"
              << std::setw(W_STATUS)               << "Trạng thái"
              << "\n";
    std::cout << std::string(TOTAL_W, '-') << "\n";

    if (categories.empty()) {
        std::cout << "  (Chưa có danh mục nào. Hãy đặt hạn mức qua mục 'Đặt hạn mức'.)\n";
    }

    for (const auto& [key, cb] : categories) {
        // Cắt và pad tên category theo chiều rộng hiển thị UTF-8 (không dùng setw)
        std::string dispCat = utf8FitLeft(cb.category, W_CAT);

        // Phần trăm và progress bar
        int pct    = (cb.limit > 0.0) ? static_cast<int>(cb.usageRatio() * 100) : 0;
        int filled = (cb.limit > 0.0)
            ? std::min(static_cast<int>(cb.usageRatio() * BAR_FILL), BAR_FILL)
            : 0;
        std::string bar = "[" + std::string(filled, '#')
                        + std::string(BAR_FILL - filled, '.') + "]";

        // Chuỗi % (right-aligned trong W_PCT)
        std::string pctStr = (cb.limit > 0.0) ? (std::to_string(pct) + "%") : "không";

        // Trạng thái
        std::string status;
        if (cb.limit <= 0.0)       status = "(không giới hạn)";
        else if (cb.isExceeded())  status = "[!!] VƯỢT HẠN MỨC!";
        else if (cb.isNearLimit()) status = "[!]  CẢNH BÁO >=80%";
        else                       status = "[ ]  OK";

        std::cout << dispCat  // đã có padding chính xác W_CAT cột
                  << std::right << std::fixed << std::setprecision(0)
                  << std::setw(W_SPENT) << cb.spent
                  << std::setw(W_LIMIT) << (cb.limit > 0.0 ? cb.limit : 0.0)
                  << std::setw(W_PCT)   << pctStr
                  << "  "
                  << std::left  << std::setw(W_BAR)    << bar
                  << std::setw(W_STATUS) << status
                  << "\n";
    }

    std::cout << std::string(TOTAL_W, '-') << "\n";
    std::cout << std::left  << std::setw(W_CAT)   << "TỔNG CỘNG"
              << std::right << std::fixed << std::setprecision(0)
              << std::setw(W_SPENT) << totalSpent()
              << std::setw(W_LIMIT) << totalLimit()
              << "\n\n";
}

// --- Persistence: lưu/nạp giới hạn ngân sách ---

void Budget::saveLimits(const std::string& filename) const {
    std::ofstream f(filename);
    if (!f.is_open()) return;
    f << month << "\n";
    for (const auto& [key, cb] : categories)
        if (cb.limit > 0.0)
            f << cb.category << "|" << std::fixed << std::setprecision(2) << cb.limit << "\n";
}

void Budget::loadLimits(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) return;
    std::string m;
    if (!std::getline(f, m) || m != month) return;  // Sai tháng → bỏ qua
    std::string line;
    while (std::getline(f, line)) {
        size_t sep = line.find('|');
        if (sep == std::string::npos || sep == 0) continue;
        std::string cat = line.substr(0, sep);
        try {
            double lim = std::stod(line.substr(sep + 1));
            if (lim > 0.0) setLimit(cat, lim);
        } catch (...) { continue; }
    }
}

// --- operator<< ---

std::ostream& operator<<(std::ostream& os, const Budget& b) {
    os << "Budget[" << b.month << "] "
       << std::fixed << std::setprecision(0)
       << b.totalSpent() << "/" << b.totalLimit() << " VND";
    return os;
}

} // namespace opep
