#include "mainwindow.h"

#include "loginwindow.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QColor>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLocale>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <vector>

struct ChartSlice {
    QString label;
    double value;
    QColor color;
};

class DonutChartWidget : public QWidget {
private:
    QVector<ChartSlice> slices;
    double totalValue = 0.0;

public:
    explicit DonutChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(520, 255);
    }

    void setData(const QVector<ChartSlice>& data) {
        slices = data;
        totalValue = 0.0;
        for (const auto& slice : slices) totalValue += slice.value;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const double legendWidth = 310.0;
        const double side = std::max(150.0, std::min(width() - legendWidth - 80.0, height() - 46.0));
        QRectF chartRect(0, 0, side, side);
        chartRect.moveTop((height() - side) / 2.0);
        chartRect.moveLeft(34);

        QPen ringPen;
        ringPen.setWidth(28);
        ringPen.setCapStyle(Qt::FlatCap);

        if (totalValue <= 0.0 || slices.isEmpty()) {
            ringPen.setColor(QColor("#3b4652"));
            p.setPen(ringPen);
            p.drawArc(chartRect, 0, 360 * 16);
            p.setPen(QColor("#cbd5e1"));
            p.drawText(rect(), Qt::AlignCenter, "Chưa có dữ liệu\nchi tiêu");
            return;
        }

        int startAngle = 90 * 16;
        for (const auto& slice : slices) {
            int span = static_cast<int>(std::round((slice.value / totalValue) * 360.0 * 16.0));
            ringPen.setColor(slice.color);
            p.setPen(ringPen);
            p.drawArc(chartRect, startAngle, -span);
            startAngle -= span;
        }

        p.setPen(QColor("#e5eef8"));
        QFont centerFont = p.font();
        centerFont.setBold(true);
        centerFont.setPointSize(13);
        p.setFont(centerFont);
        p.drawText(chartRect, Qt::AlignCenter, "Chi tiêu\ntheo mục");

        QFont legendFont = p.font();
        legendFont.setBold(true);
        legendFont.setPointSize(12);
        p.setFont(legendFont);

        int legendX = static_cast<int>(chartRect.right()) + 44;
        int y = static_cast<int>((height() - slices.size() * 34) / 2.0);
        y = std::max(24, y);
        for (const auto& slice : slices) {
            double percent = totalValue > 0.0 ? (slice.value / totalValue) * 100.0 : 0.0;
            p.setBrush(slice.color);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(QRectF(legendX, y + 7, 15, 15), 4, 4);

            p.setPen(QColor("#f8fafc"));
            QString text = QString("%1  %2%").arg(slice.label).arg(percent, 0, 'f', 0);
            p.drawText(legendX + 26, y, width() - legendX - 36, 30, Qt::AlignLeft | Qt::AlignVCenter, text);
            y += 34;
            if (y > height() - 28) break;
        }
    }
};

QString formatIntegerWithDots(qint64 value) {
    bool negative = value < 0;
    QString digits = QString::number(std::llabs(value));
    for (int i = digits.length() - 3; i > 0; i -= 3) {
        digits.insert(i, '.');
    }
    return negative ? QString("-") + digits : digits;
}

QString digitsOnly(const QString& text) {
    QString digits;
    for (const QChar& ch : text) {
        if (ch.isDigit()) digits.append(ch);
    }
    return digits;
}

class MoneyLineEdit : public QLineEdit {
public:
    explicit MoneyLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {
        setAlignment(Qt::AlignRight);
        setPlaceholderText("Ví dụ: 50.000.000");
        connect(this, &QLineEdit::textChanged, this, [this](const QString& text) {
            QString digits = digitsOnly(text);
            if (digits.isEmpty()) {
                if (!text.isEmpty()) {
                    QSignalBlocker blocker(this);
                    clear();
                }
                return;
            }
            bool ok = false;
            qlonglong value = digits.toLongLong(&ok);
            if (!ok) return;
            QString formatted = formatIntegerWithDots(value);
            if (formatted == text) return;
            QSignalBlocker blocker(this);
            setText(formatted);
            setCursorPosition(formatted.length());
        });
    }

    double moneyValue() const {
        QString digits = digitsOnly(text());
        if (digits.isEmpty()) return 0.0;
        return digits.toDouble();
    }
};

namespace {
QFrame* makePanel(const QString& title) {
    auto* panel = new QFrame;
    panel->setObjectName("Panel");
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    auto* titleLabel = new QLabel(title);
    titleLabel->setObjectName("PanelTitle");
    layout->addWidget(titleLabel);
    return panel;
}

QFrame* makeStatCard(const QString& title, const QString& accent, QLabel*& valueLabel, const QString& sub = QString()) {
    auto* card = new QFrame;
    card->setObjectName("StatCard");
    card->setMinimumHeight(104);
    card->setStyleSheet(QString(
        "QFrame#StatCard {"
        " background:qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1f2d42, stop:1 #111827);"
        " border:1px solid #2a3952; border-left:7px solid %1; border-radius:18px;"
        "}"
    ).arg(accent));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 14, 18, 12);
    layout->setSpacing(5);

    auto* titleLabel = new QLabel(title);
    titleLabel->setObjectName("CardTitle");

    valueLabel = new QLabel("-");
    valueLabel->setObjectName("CardValue");
    valueLabel->setWordWrap(true);

    auto* subLabel = new QLabel(sub);
    subLabel->setObjectName("CardSub");

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    if (!sub.isEmpty()) layout->addWidget(subLabel);
    layout->addStretch();
    return card;
}

QColor categoryColor(int index) {
    static const QVector<QColor> colors = {
        QColor("#38bdf8"), QColor("#f97316"), QColor("#a78bfa"), QColor("#22c55e"),
        QColor("#f43f5e"), QColor("#eab308"), QColor("#14b8a6"), QColor("#fb7185"),
        QColor("#60a5fa"), QColor("#c084fc")
    };
    return colors[index % colors.size()];
}

QString priorityDisplay(const std::string& p) {
    if (p == "HIGH") return "Cao";
    if (p == "MEDIUM") return "Vừa";
    return "Thấp";
}

opep::Priority priorityFromDisplay(const QString& p) {
    if (p == "Cao") return opep::Priority::HIGH;
    if (p == "Thấp") return opep::Priority::LOW;
    return opep::Priority::MEDIUM;
}
}

MainWindow::MainWindow(AppService& svc, QWidget* parent)
    : QMainWindow(parent)
    , service(svc)
{
    setWindowTitle("OPEP - Bảng điều khiển trực quan");
    resize(1360, 820);
    setMinimumSize(1080, 680);
    applyModernTheme();

    auto* tabs = new QTabWidget;
    tabs->setObjectName("MainTabs");
    tabs->addTab(createDashboardTab(), "Tổng quan");
    tabs->addTab(createTransactionTab(), "Giao dịch");
    tabs->addTab(createTaskTab(), "Công việc");
    tabs->addTab(createAccountTab(), "Tài khoản");
    setCentralWidget(tabs);

    refreshAll();
}

void MainWindow::applyModernTheme() {
    setStyleSheet(R"(
        QMainWindow {
            background: #0b1220;
            color: #e5e7eb;
            font-family: Segoe UI, Arial;
            font-size: 11pt;
        }
        QWidget {
            color: #e5e7eb;
            font-family: Segoe UI, Arial;
            font-size: 11pt;
        }
        QLabel {
            background: transparent;
            border: none;
        }
        QTabWidget, QStackedWidget, QScrollArea, QScrollArea > QWidget > QWidget {
            background: #0b1220;
            border: none;
        }
        QTabWidget::pane {
            border: none;
            background: #0b1220;
        }
        QTabBar::tab {
            background: transparent;
            color: #94a3b8;
            border: none;
            padding: 12px 22px;
            margin-right: 6px;
            border-bottom: 3px solid transparent;
            font-weight: 700;
        }
        QTabBar::tab:selected {
            color: #ffffff;
            border-bottom: 3px solid #38bdf8;
            background: #111827;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
        }
        QTabBar::tab:hover {
            color: #ffffff;
            background: #172033;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
        }
        QLabel#HeaderTitle {
            color: #ffffff;
            font-size: 24pt;
            font-weight: 800;
            letter-spacing: 0.3px;
        }
        QLabel#HeaderSub {
            color: #aab7c8;
            font-size: 11pt;
        }
        QLabel#MetricLabel {
            color: #cbd5e1;
            font-size: 11pt;
            font-weight: 700;
        }
        QLabel#PanelTitle {
            color: #f8fafc;
            font-size: 15pt;
            font-weight: 800;
            padding-bottom: 4px;
        }
        QLabel#CardTitle {
            color: #aab7c8;
            font-size: 10pt;
            font-weight: 800;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        QLabel#CardValue {
            color: #ffffff;
            font-size: 20pt;
            font-weight: 900;
        }
        QLabel#CardSub {
            color: #94a3b8;
            font-size: 10pt;
        }
        QFrame#Panel {
            background: #172033;
            border: 1px solid #2a3952;
            border-radius: 18px;
        }
        QFrame#HeaderPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #172033, stop:1 #0f2236);
            border: 1px solid #2a3952;
            border-radius: 18px;
        }
        QProgressBar {
            background: #0f172a;
            border: 1px solid #2a3952;
            border-radius: 9px;
            text-align: center;
            height: 20px;
            color: #ffffff;
            font-weight: 800;
        }
        QProgressBar::chunk {
            background: #38bdf8;
            border-radius: 9px;
        }
        QTableWidget {
            font-size: 11pt;
            background: #0f172a;
            alternate-background-color: #131f33;
            gridline-color: transparent;
            border: 1px solid #2a3952;
            border-radius: 12px;
            selection-background-color: #2563eb;
            selection-color: #ffffff;
        }
        QTableWidget::item {
            padding: 10px;
            border: none;
            background: transparent;
        }
        QHeaderView::section {
            background: #1f2d42;
            color: #f8fafc;
            padding: 12px;
            border: none;
            font-weight: 800;
            font-size: 11pt;
        }
        QPushButton {
            background: #2563eb;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            padding: 11px 20px;
            font-weight: 800;
            font-size: 10.5pt;
        }
        QPushButton:hover {
            background: #1d4ed8;
        }
        QPushButton:pressed {
            background: #1e40af;
        }
        QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox {
            background: #0f172a;
            color: #e5e7eb;
            border: 1px solid #2a3952;
            border-radius: 9px;
            padding: 9px;
            font-size: 11pt;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus {
            border: 1px solid #38bdf8;
        }
        QDialog, QMessageBox {
            background: #0b1220;
        }
        QScrollBar:vertical {
            background: #0b1220;
            width: 12px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #334155;
            border-radius: 6px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #475569;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
}

QString MainWindow::formatMoney(double value) const {
    return formatIntegerWithDots(static_cast<qint64>(std::llround(value))) + " VND";
}

void MainWindow::showError(const QString& title, const std::exception& e) {
    QMessageBox::warning(this, title, QString::fromUtf8(e.what()));
}

void MainWindow::setupTable(QTableWidget* table) {
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(false);
    table->verticalHeader()->setDefaultSectionSize(42);
    table->horizontalHeader()->setMinimumHeight(46);
}

void MainWindow::setTableText(QTableWidget* table, int row, int column, const QString& text, Qt::Alignment alignment) {
    auto* item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    table->setItem(row, column, item);
}

bool MainWindow::transactionMatchesFilters(const opep::Transaction& transaction) const {
    if (!transactionMonthFilter || !transactionTypeFilter || !transactionCategoryFilter || !transactionKeywordFilter) return true;

    const QString date = QString::fromStdString(transaction.getDate());
    const QString type = transaction.getType() == opep::TransactionType::INCOME ? "Thu nhập" : "Chi tiêu";
    const QString category = QString::fromStdString(transaction.getCategory());
    const QString title = QString::fromStdString(transaction.getTitle());
    const QString note = QString::fromStdString(transaction.getNote());

    const QString monthFilter = transactionMonthFilter->text().trimmed();
    if (!monthFilter.isEmpty() && !date.startsWith(monthFilter)) return false;

    const QString typeFilter = transactionTypeFilter->currentText();
    if (typeFilter != "Tất cả" && typeFilter != type) return false;

    const QString categoryFilter = transactionCategoryFilter->currentText();
    if (categoryFilter != "Tất cả" && categoryFilter != category) return false;

    const QString keyword = transactionKeywordFilter->text().trimmed();
    if (!keyword.isEmpty()) {
        const QString haystack = (title + " " + note + " " + category).toLower();
        if (!haystack.contains(keyword.toLower())) return false;
    }

    return true;
}

void MainWindow::updateTransactionCategoryFilter() {
    if (!transactionCategoryFilter) return;

    const QString current = transactionCategoryFilter->currentText();
    QStringList categories;
    categories << "Tất cả";

    try {
        const auto& wallet = service.getWallet();
        for (const auto& t : wallet.getTransactions()) {
            const QString category = QString::fromStdString(t->getCategory()).trimmed();
            if (!category.isEmpty() && !categories.contains(category, Qt::CaseInsensitive)) {
                categories << category;
            }
        }
    } catch (...) {
        return;
    }

    QSignalBlocker blocker(transactionCategoryFilter);
    transactionCategoryFilter->clear();
    transactionCategoryFilter->addItems(categories);
    int index = transactionCategoryFilter->findText(current, Qt::MatchFixedString);
    transactionCategoryFilter->setCurrentIndex(index >= 0 ? index : 0);
}

void MainWindow::refreshTransactionTable() {
    if (!transactionTable) return;

    try {
        const auto& wallet = service.getWallet();
        transactionTable->setRowCount(0);

        int shown = 0;
        const int total = static_cast<int>(wallet.getTransactions().size());
        for (const auto& t : wallet.getTransactions()) {
            if (!transactionMatchesFilters(*t)) continue;

            const int row = transactionTable->rowCount();
            transactionTable->insertRow(row);
            bool isIncome = t->getType() == opep::TransactionType::INCOME;
            setTableText(transactionTable, row, 0, QString::number(t->getId()), Qt::AlignCenter);
            setTableText(transactionTable, row, 1, QString::fromStdString(t->getDate()), Qt::AlignCenter);
            setTableText(transactionTable, row, 2, isIncome ? "Thu nhập" : "Chi tiêu", Qt::AlignCenter);
            setTableText(transactionTable, row, 3, QString::fromStdString(t->getTitle()));
            setTableText(transactionTable, row, 4, QString::fromStdString(t->getCategory()));
            setTableText(transactionTable, row, 5, formatMoney(t->getAmount()), Qt::AlignVCenter | Qt::AlignRight);
            setTableText(transactionTable, row, 6, QString::fromStdString(t->getNote()));
            if (auto* typeItem = transactionTable->item(row, 2)) typeItem->setForeground(isIncome ? QColor("#22c55e") : QColor("#f97316"));
            if (auto* moneyItem = transactionTable->item(row, 5)) moneyItem->setForeground(isIncome ? QColor("#22c55e") : QColor("#f97316"));
            ++shown;
        }

        if (transactionFilterSummary) {
            transactionFilterSummary->setText(shown == total
                ? QString("Đang hiển thị toàn bộ %1 giao dịch").arg(total)
                : QString("Đang hiển thị %1/%2 giao dịch theo bộ lọc").arg(shown).arg(total));
        }
    } catch (const std::exception& e) {
        showError("Lỗi lọc giao dịch", e);
    }
}

void MainWindow::clearTransactionFilters() {
    if (!transactionMonthFilter || !transactionTypeFilter || !transactionCategoryFilter || !transactionKeywordFilter) return;
    transactionMonthFilter->clear();
    transactionTypeFilter->setCurrentIndex(0);
    transactionCategoryFilter->setCurrentIndex(0);
    transactionKeywordFilter->clear();
    refreshTransactionTable();
}

QWidget* MainWindow::createDashboardTab() {
    auto* page = new QWidget;
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(12, 12, 12, 12);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* header = new QFrame;
    header->setObjectName("HeaderPanel");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(22, 16, 22, 16);

    auto* headerText = new QVBoxLayout;
    auto* title = new QLabel("Bảng điều khiển OPEP");
    title->setObjectName("HeaderTitle");
    userLabel = new QLabel;
    userLabel->setObjectName("HeaderSub");
    headerText->addWidget(title);
    headerText->addWidget(userLabel);

    auto* refreshButton = new QPushButton("Làm mới dữ liệu");
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAll);
    headerLayout->addLayout(headerText);
    headerLayout->addStretch();
    headerLayout->addWidget(refreshButton);

    auto* cards = new QGridLayout;
    cards->setSpacing(10);
    cards->addWidget(makeStatCard("SỐ DƯ HIỆN TẠI", "#38bdf8", balanceLabel, "Tổng tiền còn lại trong ví"), 0, 0);
    cards->addWidget(makeStatCard("THU THÁNG NÀY", "#22c55e", incomeMonthLabel, "Tiền vào trong tháng hiện tại"), 0, 1);
    cards->addWidget(makeStatCard("CHI THÁNG NÀY", "#f97316", expenseMonthLabel, "Tiền ra trong tháng hiện tại"), 0, 2);
    cards->addWidget(makeStatCard("CHÊNH LỆCH THU - CHI", "#a78bfa", netMonthLabel, "Dương là tiết kiệm, âm là bội chi"), 0, 3);

    auto* middle = new QHBoxLayout;
    middle->setSpacing(12);

    auto* overview = makePanel("Tiến độ tổng quan");
    auto* overviewLayout = static_cast<QVBoxLayout*>(overview->layout());
    budgetSummaryLabel = new QLabel("-");
    budgetSummaryLabel->setObjectName("CardValue");
    budgetProgress = new QProgressBar;
    budgetProgress->setRange(0, 100);
    taskSummaryLabel = new QLabel("-");
    taskSummaryLabel->setObjectName("CardValue");
    taskProgress = new QProgressBar;
    taskProgress->setRange(0, 100);
    auto* budgetMetric = new QLabel("Ngân sách tháng này");
    budgetMetric->setObjectName("MetricLabel");
    overviewLayout->addWidget(budgetMetric);
    overviewLayout->addWidget(budgetSummaryLabel);
    overviewLayout->addWidget(budgetProgress);
    overviewLayout->addSpacing(10);
    auto* taskMetric = new QLabel("Tiến độ công việc");
    taskMetric->setObjectName("MetricLabel");
    overviewLayout->addWidget(taskMetric);
    overviewLayout->addWidget(taskSummaryLabel);
    overviewLayout->addWidget(taskProgress);
    overviewLayout->addStretch();

    auto* chartPanel = makePanel("Chi tiêu theo danh mục");
    auto* chartLayout = static_cast<QVBoxLayout*>(chartPanel->layout());
    expenseChart = new DonutChartWidget;
    emptyChartLabel = new QLabel;
    emptyChartLabel->setObjectName("HeaderSub");
    emptyChartLabel->setWordWrap(true);
    chartLayout->addWidget(expenseChart);
    chartLayout->addWidget(emptyChartLabel);

    middle->addWidget(overview, 1);
    middle->addWidget(chartPanel, 2);
    overview->setMinimumHeight(235);
    chartPanel->setMinimumHeight(235);

    auto* lower = new QHBoxLayout;
    lower->setSpacing(12);

    auto* recentPanel = makePanel("Giao dịch gần đây");
    auto* recentLayout = static_cast<QVBoxLayout*>(recentPanel->layout());
    recentTransactionTable = new QTableWidget;
    recentTransactionTable->setColumnCount(4);
    recentTransactionTable->setHorizontalHeaderLabels({"Ngày", "Loại", "Danh mục", "Số tiền"});
    setupTable(recentTransactionTable);
    recentTransactionTable->setMinimumHeight(190);
    recentTransactionTable->setMaximumHeight(220);
    recentLayout->addWidget(recentTransactionTable);

    auto* taskPanel = makePanel("Công việc cần chú ý");
    auto* taskLayout = static_cast<QVBoxLayout*>(taskPanel->layout());
    upcomingTaskTable = new QTableWidget;
    upcomingTaskTable->setColumnCount(4);
    upcomingTaskTable->setHorizontalHeaderLabels({"Trạng thái", "Công việc", "Ưu tiên", "Hạn chót"});
    setupTable(upcomingTaskTable);
    upcomingTaskTable->setMinimumHeight(190);
    upcomingTaskTable->setMaximumHeight(220);
    taskLayout->addWidget(upcomingTaskTable);

    lower->addWidget(recentPanel, 1);
    lower->addWidget(taskPanel, 1);

    auto* budgetPanel = makePanel("Ngân sách theo danh mục");
    auto* budgetLayout = static_cast<QVBoxLayout*>(budgetPanel->layout());
    budgetTable = new QTableWidget;
    budgetTable->setColumnCount(6);
    budgetTable->setHorizontalHeaderLabels({"Danh mục", "Đã chi", "Hạn mức", "Còn lại", "Tiến độ", "Trạng thái"});
    setupTable(budgetTable);
    budgetTable->setMinimumHeight(210);

    auto* buttonRow = new QHBoxLayout;
    auto* limitButton = new QPushButton("Đặt hạn mức");
    auto* removeLimitButton = new QPushButton("Xóa hạn mức");
    auto* refresh2Button = new QPushButton("Làm mới");
    connect(limitButton, &QPushButton::clicked, this, &MainWindow::setBudgetLimit);
    connect(removeLimitButton, &QPushButton::clicked, this, &MainWindow::removeBudgetLimit);
    connect(refresh2Button, &QPushButton::clicked, this, &MainWindow::refreshAll);
    buttonRow->addWidget(limitButton);
    buttonRow->addWidget(removeLimitButton);
    buttonRow->addWidget(refresh2Button);
    buttonRow->addStretch();

    budgetLayout->addWidget(budgetTable);
    budgetLayout->addLayout(buttonRow);

    layout->addWidget(header);
    layout->addLayout(cards);
    layout->addLayout(middle);
    layout->addLayout(lower);
    layout->addWidget(budgetPanel);
    layout->addStretch();

    scroll->setWidget(content);
    outer->addWidget(scroll);
    return page;
}

QWidget* MainWindow::createTransactionTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(12);

    auto* panel = makePanel("Quản lý giao dịch");
    auto* panelLayout = static_cast<QVBoxLayout*>(panel->layout());

    auto* filterPanel = new QFrame;
    filterPanel->setObjectName("Panel");
    auto* filterLayout = new QGridLayout(filterPanel);
    filterLayout->setContentsMargins(14, 12, 14, 12);
    filterLayout->setHorizontalSpacing(12);
    filterLayout->setVerticalSpacing(8);

    auto* filterTitle = new QLabel("Bộ lọc giao dịch");
    filterTitle->setObjectName("MetricLabel");

    transactionMonthFilter = new QLineEdit;
    transactionMonthFilter->setPlaceholderText("Ví dụ: 2026-06");
    transactionMonthFilter->setMaxLength(7);

    transactionTypeFilter = new QComboBox;
    transactionTypeFilter->addItems({"Tất cả", "Thu nhập", "Chi tiêu"});

    transactionCategoryFilter = new QComboBox;
    transactionCategoryFilter->addItem("Tất cả");

    transactionKeywordFilter = new QLineEdit;
    transactionKeywordFilter->setPlaceholderText("Tìm theo mô tả hoặc ghi chú");

    auto* clearFilterButton = new QPushButton("Xóa bộ lọc");
    transactionFilterSummary = new QLabel("Đang hiển thị toàn bộ giao dịch");
    transactionFilterSummary->setObjectName("HeaderSub");

    filterLayout->addWidget(filterTitle, 0, 0);
    filterLayout->addWidget(new QLabel("Tháng:"), 1, 0);
    filterLayout->addWidget(transactionMonthFilter, 1, 1);
    filterLayout->addWidget(new QLabel("Loại:"), 1, 2);
    filterLayout->addWidget(transactionTypeFilter, 1, 3);
    filterLayout->addWidget(new QLabel("Danh mục:"), 2, 0);
    filterLayout->addWidget(transactionCategoryFilter, 2, 1);
    filterLayout->addWidget(new QLabel("Từ khóa:"), 2, 2);
    filterLayout->addWidget(transactionKeywordFilter, 2, 3);
    filterLayout->addWidget(clearFilterButton, 1, 4, 2, 1);
    filterLayout->addWidget(transactionFilterSummary, 3, 0, 1, 5);
    filterLayout->setColumnStretch(1, 2);
    filterLayout->setColumnStretch(3, 2);

    transactionTable = new QTableWidget;
    transactionTable->setColumnCount(7);
    transactionTable->setHorizontalHeaderLabels({"ID", "Ngày", "Loại", "Mô tả", "Danh mục", "Số tiền", "Ghi chú"});
    setupTable(transactionTable);

    auto* buttonRow = new QHBoxLayout;
    auto* addIncomeButton = new QPushButton("+ Thêm thu nhập");
    auto* addExpenseButton = new QPushButton("- Thêm chi tiêu");
    auto* refreshButton = new QPushButton("Làm mới");

    connect(addIncomeButton, &QPushButton::clicked, this, &MainWindow::addIncome);
    connect(addExpenseButton, &QPushButton::clicked, this, &MainWindow::addExpense);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAll);
    connect(clearFilterButton, &QPushButton::clicked, this, &MainWindow::clearTransactionFilters);
    connect(transactionMonthFilter, &QLineEdit::textChanged, this, &MainWindow::refreshTransactionTable);
    connect(transactionTypeFilter, &QComboBox::currentTextChanged, this, &MainWindow::refreshTransactionTable);
    connect(transactionCategoryFilter, &QComboBox::currentTextChanged, this, &MainWindow::refreshTransactionTable);
    connect(transactionKeywordFilter, &QLineEdit::textChanged, this, &MainWindow::refreshTransactionTable);

    buttonRow->addWidget(addIncomeButton);
    buttonRow->addWidget(addExpenseButton);
    buttonRow->addWidget(refreshButton);
    buttonRow->addStretch();

    panelLayout->addWidget(filterPanel);
    panelLayout->addWidget(transactionTable);
    panelLayout->addLayout(buttonRow);

    layout->addWidget(panel);
    return page;
}

QWidget* MainWindow::createTaskTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(12);

    auto* panel = makePanel("Quản lý công việc");
    auto* panelLayout = static_cast<QVBoxLayout*>(panel->layout());

    taskTable = new QTableWidget;
    taskTable->setColumnCount(6);
    taskTable->setHorizontalHeaderLabels({"ID", "Trạng thái", "Tên công việc", "Ưu tiên", "Hạn chót", "Ngày tạo"});
    setupTable(taskTable);

    auto* buttonRow = new QHBoxLayout;
    auto* addButton = new QPushButton("+ Thêm công việc");
    auto* doneButton = new QPushButton("Đánh dấu xong");
    auto* deleteButton = new QPushButton("Xóa công việc");
    auto* sortButton = new QPushButton("Sắp xếp ưu tiên");
    auto* refreshButton = new QPushButton("Làm mới");

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(doneButton, &QPushButton::clicked, this, &MainWindow::markSelectedTaskDone);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTask);
    connect(sortButton, &QPushButton::clicked, this, &MainWindow::sortTasks);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshAll);

    buttonRow->addWidget(addButton);
    buttonRow->addWidget(doneButton);
    buttonRow->addWidget(deleteButton);
    buttonRow->addWidget(sortButton);
    buttonRow->addWidget(refreshButton);
    buttonRow->addStretch();

    panelLayout->addWidget(taskTable);
    panelLayout->addLayout(buttonRow);

    layout->addWidget(panel);
    return page;
}

QWidget* MainWindow::createAccountTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 14, 14, 14);

    auto* panel = makePanel("Tài khoản");
    auto* panelLayout = static_cast<QVBoxLayout*>(panel->layout());

    auto* info = new QLabel("Dữ liệu vẫn được lưu trong thư mục data/ giống bản console. Bạn có thể đổi mật khẩu hoặc đăng xuất tại đây.");
    info->setWordWrap(true);
    info->setObjectName("HeaderSub");

    auto* changePwButton = new QPushButton("Đổi mật khẩu");
    auto* logoutButton = new QPushButton("Đăng xuất");

    connect(changePwButton, &QPushButton::clicked, this, &MainWindow::changePassword);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);

    panelLayout->addWidget(info);
    panelLayout->addSpacing(10);
    panelLayout->addWidget(changePwButton);
    panelLayout->addWidget(logoutButton);
    panelLayout->addStretch();

    layout->addWidget(panel);
    layout->addStretch();
    return page;
}

int MainWindow::selectedId(QTableWidget* table) const {
    const auto ranges = table->selectedRanges();
    if (ranges.isEmpty()) return -1;
    int row = ranges.first().topRow();
    auto* item = table->item(row, 0);
    return item ? item->text().toInt() : -1;
}

void MainWindow::refreshAll() {
    try {
        const auto& wallet = service.getWallet();
        const auto& tm = service.getTaskManager();
        auto& budget = service.getBudget();
        budget.syncFromTransactions(wallet.getTransactions());

        QString currentMonth = QDate::currentDate().toString("yyyy-MM");
        double incomeMonth = 0.0;
        double expenseMonth = 0.0;
        std::vector<const opep::Transaction*> monthExpenses;

        for (const auto& t : wallet.getTransactions()) {
            if (QString::fromStdString(t->getDate()).left(7) != currentMonth) continue;
            if (t->getType() == opep::TransactionType::INCOME) {
                incomeMonth += t->getAmount();
            } else {
                expenseMonth += t->getAmount();
                monthExpenses.push_back(t.get());
            }
        }

        userLabel->setText(QString("Xin chào, ") + QString::fromStdString(service.username()) + "  |  " + QDate::currentDate().toString("dd/MM/yyyy"));
        balanceLabel->setText(formatMoney(wallet.getBalance()));
        incomeMonthLabel->setText(formatMoney(incomeMonth));
        expenseMonthLabel->setText(formatMoney(expenseMonth));
        netMonthLabel->setText(formatMoney(incomeMonth - expenseMonth));

        int totalTasks = tm.count();
        int completedTasks = tm.countCompleted();
        int taskPercent = totalTasks > 0 ? static_cast<int>((completedTasks * 100.0) / totalTasks) : 0;
        taskSummaryLabel->setText(QString("%1/%2 công việc đã xong").arg(completedTasks).arg(totalTasks));
        taskProgress->setValue(taskPercent);

        double totalLimit = budget.totalLimit();
        double totalSpent = budget.totalSpent();
        int budgetPercent = totalLimit > 0.0 ? static_cast<int>(std::min(100.0, (totalSpent / totalLimit) * 100.0)) : 0;
        budgetSummaryLabel->setText(QString("Đã chi ") + formatMoney(totalSpent) + " / " + (totalLimit > 0.0 ? formatMoney(totalLimit) : QString("Không giới hạn")));
        budgetProgress->setValue(budgetPercent);
        if (totalLimit > 0.0 && totalSpent > totalLimit) {
            budgetProgress->setStyleSheet("QProgressBar::chunk { background:#ef4444; border-radius:8px; }");
        } else if (budgetPercent >= 80) {
            budgetProgress->setStyleSheet("QProgressBar::chunk { background:#f97316; border-radius:8px; }");
        } else {
            budgetProgress->setStyleSheet("QProgressBar::chunk { background:#22c55e; border-radius:8px; }");
        }

        updateTransactionCategoryFilter();
        refreshTransactionTable();

        // Bảng công việc chính.
        taskTable->setRowCount(static_cast<int>(tm.getAll().size()));
        int row = 0;
        for (const auto& taskPtr : tm.getAll()) {
            const auto& task = *taskPtr;
            QString priority = priorityDisplay(opep::priorityToString(task.getPriority()));
            setTableText(taskTable, row, 0, QString::number(task.getId()), Qt::AlignCenter);
            setTableText(taskTable, row, 1, task.isCompleted() ? "Đã xong" : "Chưa xong", Qt::AlignCenter);
            setTableText(taskTable, row, 2, QString::fromStdString(task.getTitle()));
            setTableText(taskTable, row, 3, priority, Qt::AlignCenter);
            setTableText(taskTable, row, 4, QString::fromStdString(task.getDeadline()).isEmpty() ? "-" : QString::fromStdString(task.getDeadline()), Qt::AlignCenter);
            setTableText(taskTable, row, 5, QString::fromStdString(task.getDate()), Qt::AlignCenter);
            if (auto* statusItem = taskTable->item(row, 1)) statusItem->setForeground(task.isCompleted() ? QColor("#22c55e") : QColor("#f97316"));
            if (auto* prItem = taskTable->item(row, 3)) {
                if (priority == "Cao") prItem->setForeground(QColor("#ef4444"));
                else if (priority == "Vừa") prItem->setForeground(QColor("#facc15"));
                else prItem->setForeground(QColor("#38bdf8"));
            }
            row++;
        }

        // Bang ngan sach + progress bar tung danh muc.
        const auto& categories = budget.getCategories();
        budgetTable->setRowCount(static_cast<int>(categories.size()));
        row = 0;
        QVector<ChartSlice> chartData;
        int colorIndex = 0;
        for (const auto& pair : categories) {
            const auto& cb = pair.second;
            const double remaining = cb.limit > 0.0 ? cb.remaining() : 0.0;
            const int percent = cb.limit > 0.0 ? static_cast<int>(std::min(100.0, cb.usageRatio() * 100.0)) : 0;
            QString status = "Không giới hạn";
            QColor statusColor("#94a3b8");
            if (cb.isExceeded()) { status = "Vượt hạn mức"; statusColor = QColor("#ef4444"); }
            else if (cb.isNearLimit()) { status = "Cảnh báo"; statusColor = QColor("#f97316"); }
            else if (cb.limit > 0.0) { status = "Ổn"; statusColor = QColor("#22c55e"); }

            setTableText(budgetTable, row, 0, QString::fromStdString(cb.category));
            setTableText(budgetTable, row, 1, formatMoney(cb.spent), Qt::AlignVCenter | Qt::AlignRight);
            setTableText(budgetTable, row, 2, cb.limit > 0.0 ? formatMoney(cb.limit) : "Không giới hạn", Qt::AlignVCenter | Qt::AlignRight);
            setTableText(budgetTable, row, 3, cb.limit > 0.0 ? formatMoney(remaining) : "-", Qt::AlignVCenter | Qt::AlignRight);

            auto* bar = new QProgressBar;
            bar->setRange(0, 100);
            bar->setValue(percent);
            bar->setFormat(cb.limit > 0.0 ? QString::number(percent) + "%" : "-");
            if (cb.isExceeded()) bar->setStyleSheet("QProgressBar::chunk { background:#ef4444; border-radius:8px; }");
            else if (cb.isNearLimit()) bar->setStyleSheet("QProgressBar::chunk { background:#f97316; border-radius:8px; }");
            else bar->setStyleSheet("QProgressBar::chunk { background:#22c55e; border-radius:8px; }");
            budgetTable->setCellWidget(row, 4, bar);

            setTableText(budgetTable, row, 5, status, Qt::AlignCenter);
            if (auto* statusItem = budgetTable->item(row, 5)) statusItem->setForeground(statusColor);

            if (cb.spent > 0.0) chartData.push_back({QString::fromStdString(cb.category), cb.spent, categoryColor(colorIndex++)});
            row++;
        }
        expenseChart->setData(chartData);
        emptyChartLabel->setText(chartData.isEmpty() ? "Chưa có chi tiêu trong tháng này. Hãy thêm giao dịch chi tiêu để hiện biểu đồ." : "Biểu đồ giúp nhìn nhanh danh mục nào đang chi nhiều nhất.");

        // Giao dich gan day tren dashboard.
        int recentCount = std::min<int>(6, wallet.getTransactions().size());
        recentTransactionTable->setRowCount(recentCount);
        for (int i = 0; i < recentCount; ++i) {
            const auto& t = wallet.getTransactions().at(wallet.getTransactions().size() - 1 - i);
            bool isIncome = t->getType() == opep::TransactionType::INCOME;
            setTableText(recentTransactionTable, i, 0, QString::fromStdString(t->getDate()), Qt::AlignCenter);
            setTableText(recentTransactionTable, i, 1, isIncome ? "Thu" : "Chi", Qt::AlignCenter);
            setTableText(recentTransactionTable, i, 2, QString::fromStdString(t->getCategory()));
            setTableText(recentTransactionTable, i, 3, formatMoney(t->getAmount()), Qt::AlignVCenter | Qt::AlignRight);
            if (auto* moneyItem = recentTransactionTable->item(i, 3)) moneyItem->setForeground(isIncome ? QColor("#22c55e") : QColor("#f97316"));
        }

        // Task can chu y tren dashboard.
        std::vector<const opep::Task*> pendingTasks;
        for (const auto& taskPtr : tm.getAll()) {
            if (!taskPtr->isCompleted()) pendingTasks.push_back(taskPtr.get());
        }
        std::sort(pendingTasks.begin(), pendingTasks.end(), [](const opep::Task* a, const opep::Task* b) {
            std::string da = a->getDeadline().empty() ? "9999-12-31" : a->getDeadline();
            std::string db = b->getDeadline().empty() ? "9999-12-31" : b->getDeadline();
            if (da == db) return a->getPriority() > b->getPriority();
            return da < db;
        });
        int pendingCount = std::min<int>(6, pendingTasks.size());
        upcomingTaskTable->setRowCount(pendingCount);
        for (int i = 0; i < pendingCount; ++i) {
            const auto& task = *pendingTasks[i];
            QString priority = priorityDisplay(opep::priorityToString(task.getPriority()));
            setTableText(upcomingTaskTable, i, 0, "Chưa xong", Qt::AlignCenter);
            setTableText(upcomingTaskTable, i, 1, QString::fromStdString(task.getTitle()));
            setTableText(upcomingTaskTable, i, 2, priority, Qt::AlignCenter);
            setTableText(upcomingTaskTable, i, 3, QString::fromStdString(task.getDeadline()).isEmpty() ? "-" : QString::fromStdString(task.getDeadline()), Qt::AlignCenter);
            if (auto* prItem = upcomingTaskTable->item(i, 2)) {
                if (priority == "Cao") prItem->setForeground(QColor("#ef4444"));
                else if (priority == "Vừa") prItem->setForeground(QColor("#facc15"));
                else prItem->setForeground(QColor("#38bdf8"));
            }
        }
    } catch (const std::exception& e) {
        showError("Lỗi làm mới dữ liệu", e);
    }
}

void MainWindow::addIncome() {
    QDialog dialog(this);
    dialog.setWindowTitle("Thêm thu nhập");

    auto* amount = new MoneyLineEdit;

    auto* category = new QComboBox;
    category->setEditable(true);
    category->addItems({"Lương", "Thưởng", "Trợ cấp", "Đầu tư", "Bán hàng", "Khác"});
    category->setCurrentText("Lương");
    auto* title = new QLineEdit;
    auto* note = new QLineEdit;

    auto* form = new QFormLayout(&dialog);
    form->addRow("Số tiền (VND):", amount);
    form->addRow("Danh mục:", category);
    form->addRow("Mô tả:", title);
    form->addRow("Ghi chú:", note);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    try {
        if (amount->moneyValue() <= 0.0) {
            QMessageBox::warning(this, "Số tiền không hợp lệ", "Số tiền phải lớn hơn 0 VND.");
            return;
        }

        QString categoryText = category->currentText().trimmed();
        QString titleText = title->text().trimmed();
        if (titleText.isEmpty()) {
            titleText = categoryText.isEmpty() ? "Thu nhập" : categoryText;
        }

        service.addIncome(amount->moneyValue(), categoryText.toStdString(), titleText.toStdString(), note->text().trimmed().toStdString());
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không thêm được thu nhập", e);
    }
}

void MainWindow::addExpense() {
    QDialog dialog(this);
    dialog.setWindowTitle("Thêm chi tiêu");

    auto* amount = new MoneyLineEdit;

    auto* category = new QComboBox;
    category->setEditable(true);
    category->addItems({"Ăn uống", "Di chuyển", "Du lịch", "Học tập", "Giải trí", "Nhà ở", "Mua sắm", "Sức khỏe", "Hóa đơn", "Khác"});
    category->setCurrentText("Ăn uống");
    auto* title = new QLineEdit;
    auto* note = new QLineEdit;

    auto* form = new QFormLayout(&dialog);
    form->addRow("Số tiền (VND):", amount);
    form->addRow("Danh mục:", category);
    form->addRow("Mô tả:", title);
    form->addRow("Ghi chú:", note);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    try {
        if (amount->moneyValue() <= 0.0) {
            QMessageBox::warning(this, "Số tiền không hợp lệ", "Số tiền phải lớn hơn 0 VND.");
            return;
        }

        QString categoryText = category->currentText().trimmed();
        QString titleText = title->text().trimmed();
        if (titleText.isEmpty()) {
            titleText = categoryText.isEmpty() ? "Chi tiêu" : categoryText;
        }

        service.addExpense(amount->moneyValue(), categoryText.toStdString(), titleText.toStdString(), note->text().trimmed().toStdString());
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không thêm được chi tiêu", e);
    }
}

void MainWindow::addTask() {
    QDialog dialog(this);
    dialog.setWindowTitle("Thêm công việc");

    auto* title = new QLineEdit;
    auto* deadline = new QDateEdit;
    deadline->setCalendarPopup(true);
    deadline->setDisplayFormat("yyyy-MM-dd");
    deadline->setDate(QDate::currentDate());

    auto* noDeadline = new QCheckBox("Không có hạn chót");
    auto* priority = new QComboBox;
    priority->addItems({"Thấp", "Vừa", "Cao"});
    priority->setCurrentText("Vừa");

    auto* form = new QFormLayout(&dialog);
    form->addRow("Tên công việc:", title);
    form->addRow("Hạn chót:", deadline);
    form->addRow("", noDeadline);
    form->addRow("Ưu tiên:", priority);

    connect(noDeadline, &QCheckBox::toggled, deadline, &QDateEdit::setDisabled);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    try {
        opep::Priority pr = priorityFromDisplay(priority->currentText());

        service.addTask(title->text().trimmed().toStdString(),
                        noDeadline->isChecked() ? "" : deadline->date().toString("yyyy-MM-dd").toStdString(),
                        pr);
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không thêm được công việc", e);
    }
}

void MainWindow::markSelectedTaskDone() {
    int id = selectedId(taskTable);
    if (id < 0) {
        QMessageBox::information(this, "Chọn công việc", "Hãy chọn một công việc trong bảng trước.");
        return;
    }

    try {
        service.markTaskDone(id);
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không cập nhật được công việc", e);
    }
}

void MainWindow::deleteSelectedTask() {
    int id = selectedId(taskTable);
    if (id < 0) {
        QMessageBox::information(this, "Chọn công việc", "Hãy chọn một công việc trong bảng trước.");
        return;
    }

    if (QMessageBox::question(this, "Xóa công việc", "Bạn có chắc muốn xóa công việc này?") != QMessageBox::Yes) return;

    try {
        service.removeTask(id);
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không xóa được công việc", e);
    }
}

void MainWindow::sortTasks() {
    try {
        service.sortTasksByPriority();
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không sắp xếp được công việc", e);
    }
}

void MainWindow::setBudgetLimit() {
    QDialog dialog(this);
    dialog.setWindowTitle("Đặt hạn mức ngân sách");

    auto* category = new QComboBox;
    category->setEditable(true);
    category->addItems({"Ăn uống", "Di chuyển", "Du lịch", "Học tập", "Giải trí", "Nhà ở", "Mua sắm", "Sức khỏe", "Hóa đơn", "Khác"});
    category->setCurrentText("Ăn uống");
    auto* limit = new MoneyLineEdit;

    auto* form = new QFormLayout(&dialog);
    form->addRow("Danh mục:", category);
    form->addRow("Hạn mức (VND):", limit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    try {
        if (limit->moneyValue() <= 0.0) {
            QMessageBox::warning(this, "Hạn mức không hợp lệ", "Hạn mức phải lớn hơn 0 VND.");
            return;
        }
        service.setBudgetLimit(category->currentText().trimmed().toStdString(), limit->moneyValue());
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không đặt được hạn mức", e);
    }
}


void MainWindow::removeBudgetLimit() {
    const auto& categories = service.getBudget().getCategories();
    QStringList limitedCategories;
    for (const auto& pair : categories) {
        const auto& cb = pair.second;
        if (cb.limit > 0.0) {
            limitedCategories << QString::fromStdString(cb.category);
        }
    }

    if (limitedCategories.isEmpty()) {
        QMessageBox::information(this, "Xóa hạn mức", "Hiện chưa có danh mục nào đang đặt hạn mức.");
        return;
    }

    bool ok = false;
    QString category = QInputDialog::getItem(this,
                                             "Xóa hạn mức",
                                             "Chọn danh mục cần xóa hạn mức:",
                                             limitedCategories,
                                             0,
                                             false,
                                             &ok);
    if (!ok || category.trimmed().isEmpty()) return;

    if (QMessageBox::question(this,
                              "Xóa hạn mức",
                              QString("Bạn có chắc muốn xóa hạn mức của danh mục \"") + category + "\" không?\nDữ liệu chi tiêu vẫn được giữ lại.") != QMessageBox::Yes) {
        return;
    }

    try {
        service.removeBudgetLimit(category.trimmed().toStdString());
        refreshAll();
    } catch (const std::exception& e) {
        showError("Không xóa được hạn mức", e);
    }
}

void MainWindow::changePassword() {
    QDialog dialog(this);
    dialog.setWindowTitle("Đổi mật khẩu");

    auto* oldPassword = new QLineEdit;
    auto* newPassword = new QLineEdit;
    auto* confirmPassword = new QLineEdit;
    oldPassword->setEchoMode(QLineEdit::Password);
    newPassword->setEchoMode(QLineEdit::Password);
    confirmPassword->setEchoMode(QLineEdit::Password);

    auto* form = new QFormLayout(&dialog);
    form->addRow("Mật khẩu cũ:", oldPassword);
    form->addRow("Mật khẩu mới:", newPassword);
    form->addRow("Nhập lại:", confirmPassword);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    if (newPassword->text() != confirmPassword->text()) {
        QMessageBox::warning(this, "Lỗi", "Mật khẩu mới nhập lại không khớp.");
        return;
    }

    try {
        service.changePassword(oldPassword->text().toStdString(), newPassword->text().toStdString());
        QMessageBox::information(this, "Thành công", "Đã đổi mật khẩu.");
    } catch (const std::exception& e) {
        showError("Không đổi được mật khẩu", e);
    }
}

void MainWindow::logout() {
    service.logout();
    LoginWindow login(service, this);
    if (login.exec() == QDialog::Accepted) {
        refreshAll();
    } else {
        close();
    }
}
