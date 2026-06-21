#include "loginwindow.h"

#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(AppService& svc, QWidget* parent)
    : QDialog(parent)
    , service(svc)
{
    setWindowTitle("OPEP - Đăng nhập");
    setMinimumWidth(420);

    auto* title = new QLabel("OPEP - Quản lý chi tiêu và công việc cá nhân");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);

    usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Ví dụ: thien_01");

    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);

    confirmPasswordEdit = new QLineEdit;
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("Chỉ cần nhập khi đăng ký");

    auto* form = new QFormLayout;
    form->addRow("Tên đăng nhập:", usernameEdit);
    form->addRow("Mật khẩu:", passwordEdit);
    form->addRow("Nhập lại mật khẩu:", confirmPasswordEdit);

    auto* loginButton = new QPushButton("Đăng nhập");
    auto* registerButton = new QPushButton("Đăng ký");

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::handleRegister);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(loginButton);
    buttons->addWidget(registerButton);

    auto* layout = new QVBoxLayout;
    layout->addWidget(title);
    layout->addSpacing(12);
    layout->addLayout(form);
    layout->addLayout(buttons);
    setLayout(layout);
}

void LoginWindow::handleLogin() {
    try {
        service.login(usernameEdit->text().trimmed().toStdString(),
                      passwordEdit->text().toStdString());
        accept();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Đăng nhập thất bại", QString::fromUtf8(e.what()));
    }
}

void LoginWindow::handleRegister() {
    try {
        const QString username = usernameEdit->text().trimmed();
        const QString password = passwordEdit->text();
        const QString confirm = confirmPasswordEdit->text();

        if (password != confirm) {
            QMessageBox::warning(this, "Lỗi đăng ký", "Mật khẩu nhập lại không khớp.");
            return;
        }

        service.registerUser(username.toStdString(), password.toStdString());
        QMessageBox::information(this, "Đăng ký thành công", "Tài khoản đã được tạo. Bạn có thể đăng nhập ngay.");
        confirmPasswordEdit->clear();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Đăng ký thất bại", QString::fromUtf8(e.what()));
    }
}
