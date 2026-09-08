#include <QApplication>
<<<<<<< HEAD
#include "ui/LoadingWindow.h"
#include "ui/LoginWindow.h"
#include "MainWindow.h"
#include "auth/DatabaseManager.h"
#include "auth/AuthService.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("OSimX");
    app.setApplicationDisplayName("OSimX");
    app.setOrganizationName("Team Monolith");
    app.setWindowIcon(QIcon(":/resources/logo.png"));
    app.setStyle("Fusion");

    // Connect to database
    DatabaseManager::instance().open();

    LoadingWindow loadingWindow;
    loadingWindow.show();

    QObject::connect(&loadingWindow, &LoadingWindow::loadingFinished,
                     [&](bool isLoggedIn) {
        loadingWindow.close();

        if (isLoggedIn) {
            MainWindow *mainWindow = new MainWindow();
            mainWindow->setAttribute(Qt::WA_DeleteOnClose);
            mainWindow->show();
        } else {
            LoginWindow *loginWindow = new LoginWindow();
            loginWindow->setAttribute(Qt::WA_DeleteOnClose);

            QObject::connect(loginWindow, &LoginWindow::loginSuccessful, [=]() {
                loginWindow->close();
                MainWindow *mainWindow = new MainWindow();
                mainWindow->setAttribute(Qt::WA_DeleteOnClose);
                mainWindow->show();
            });

            loginWindow->show();
        }
    });

=======
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setStyle("Fusion");
    
    MainWindow window;
    window.show();
    
>>>>>>> dfba9977f80e13ca5b71a4f411afe79c5b13a014
    return app.exec();
}