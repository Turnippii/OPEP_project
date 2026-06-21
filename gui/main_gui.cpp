#include "appservice.h"
#include "loginwindow.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDialog>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    AppService service("data/");

    LoginWindow login(service);
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(service);
    window.show();

    return app.exec();
}
