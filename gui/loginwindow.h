#ifndef OPEP_GUI_LOGINWINDOW_H
#define OPEP_GUI_LOGINWINDOW_H

#include "appservice.h"

#include <QDialog>

class QLineEdit;

class LoginWindow : public QDialog {
    Q_OBJECT

private:
    AppService& service;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* confirmPasswordEdit;

private slots:
    void handleLogin();
    void handleRegister();

public:
    explicit LoginWindow(AppService& service, QWidget* parent = nullptr);
};

#endif // OPEP_GUI_LOGINWINDOW_H
