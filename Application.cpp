#include "Application.h"
#include "database/DatabaseManager.h"
#include "ui/authentication/LoginWindow.h"
#include "ui/authentication/SignupWindow.h"
#include "ui/main/MainWindow.h"

#include <QApplication>
#include <QIcon>

namespace {

void applyTheme(QApplication& app)
{
    app.setStyle("Fusion");

    app.setStyleSheet(R"(
        /* =========================================================
           GLOBAL
           ========================================================= */

        QWidget {
            font-family: "Segoe UI";
            font-size: 13px;
            color: #172033;
            background: #eef2f6;
        }

        QMainWindow {
            background: #eef2f6;
        }

        QLabel,
        QCheckBox {
            background: transparent;
        }


        /* =========================================================
           GENERAL / MAIN UI
           ========================================================= */

        QLabel#brand {
            font-size: 28px;
            font-weight: 800;
            color: #172033;
        }

        QLabel#tagline {
            color: #6b7484;
            font-size: 14px;
        }

        QLabel#heroTitle,
        QLabel#pageTitle {
            font-size: 28px;
            font-weight: 800;
            color: #172033;
        }

        QLabel#heroText,
        QLabel#pageSubtitle {
            color: #697586;
            font-size: 14px;
        }

        QLabel#accountPill,
        QLabel#stepPill {
            background: #ffffff;
            border: 1px solid #dfe4ec;
            border-radius: 16px;
            padding: 7px 12px;
            color: #516071;
        }

        QLabel#simulationIcon {
            font-size: 12px;
            font-weight: 800;
            color: #2aa56a;
            letter-spacing: 2px;
        }

        QLabel#simulationTitle {
            font-size: 18px;
            font-weight: 750;
            color: #172033;
        }

        QLabel#simulationDescription {
            color: #657083;
        }

        QLabel#statusActive {
            color: #17864a;
            font-weight: 700;
        }

        QLabel#statusDisabled {
            color: #8a93a3;
            font-weight: 700;
        }

        QLabel#footerHint {
            color: #7b8494;
            padding-top: 6px;
        }

        QLabel#infoLabel {
            background: #f3f7f4;
            border: 1px solid #d9eadf;
            border-radius: 10px;
            padding: 12px;
            color: #496050;
        }

        QLabel#metricsValue {
            font-size: 13px;
            font-weight: 650;
            color: #273444;
        }


        /* =========================================================
           CARDS
           ========================================================= */

        QFrame#simulationCard,
        QFrame#authCard {
            background: #ffffff;
            border: 1px solid #dfe4ec;
            border-radius: 16px;
        }

        QFrame#simulationCard:hover {
            border: 1px solid #2aa56a;
        }

        QFrame#simulationCardDisabled {
            background: #f7f8fa;
            border: 1px solid #e5e8ee;
            border-radius: 16px;
        }


        /* =========================================================
           LOGIN / BRAND PANEL
           ========================================================= */

        QFrame#brandPanel {
            background: #172033;
        }

        QLabel#brandTitle {
            background: transparent;
            color: #ffffff;
            font-size: 42px;
            font-weight: 850;
        }

        QLabel#brandSubtitle {
            background: transparent;
            color: #c7d0dd;
            font-size: 16px;
        }

        QLabel#brandBody {
            background: transparent;
            color: #e0e6ee;
            font-size: 15px;
        }

        QLabel#brandPoint {
            background: transparent;
            color: #f4f7fa;
            font-size: 14px;
            padding: 6px 0;
        }

        QFrame#brandDivider {
            color: #394357;
            background: #394357;
        }


        /* =========================================================
           AUTHENTICATION
           ========================================================= */

        QLabel#authTitle {
            background: transparent;
            font-size: 26px;
            font-weight: 800;
            color: #172033;
        }

        QLabel#authSubtitle {
            background: transparent;
            color: #667085;
            font-size: 14px;
        }

        QLabel#fieldLabel {
            background: transparent;
            color: #344054;
            font-weight: 700;
            margin-top: 4px;
        }

        QLabel#formHint {
            background: transparent;
            color: #8a93a3;
            font-size: 12px;
        }

        QLineEdit#authField {
            background: #ffffff;
            color: #172033;
            border: 1px solid #d4dae4;
            border-radius: 9px;
            padding: 9px 11px;
        }

        QLineEdit#authField:focus {
            border: 1px solid #2aa56a;
            background: #ffffff;
        }


        /* =========================================================
           BUTTONS
           ========================================================= */

        QPushButton {
            background: #2aa56a;
            color: #ffffff;
            border: none;
            border-radius: 9px;
            padding: 9px 14px;
            font-weight: 700;
        }

        QPushButton:hover {
            background: #228b58;
        }

        QPushButton:disabled {
            background: #b9c2ce;
        }

        QPushButton#primaryButton {
            background: #2aa56a;
            min-height: 40px;
        }

        QPushButton#secondaryButton {
            background: #ffffff;
            color: #394657;
            border: 1px solid #d7dde7;
        }

        QPushButton#secondaryButton:hover {
            background: #f3f6f8;
            border-color: #2aa56a;
        }

        QPushButton#dangerButton {
            background: #ffffff;
            color: #a33d3d;
            border: 1px solid #e0c9c9;
        }

        QPushButton#dangerButton:hover {
            background: #fff6f6;
        }

        QPushButton#linkButton {
            background: transparent;
            color: #208e5b;
            border: none;
            padding: 2px 4px;
        }

        QPushButton#linkButton:hover {
            color: #176c45;
            text-decoration: underline;
        }


        /* =========================================================
           CHECKBOX
           ========================================================= */

        QCheckBox {
            background: transparent;
            color: #526071;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #98a2b3;
            border-radius: 4px;
            background: #ffffff;
        }

        QCheckBox::indicator:hover {
            border: 1px solid #2aa56a;
        }

        QCheckBox::indicator:checked {
            background: #2aa56a;
            border: 1px solid #2aa56a;
        }

        QCheckBox::indicator:checked:hover {
            background: #228b58;
            border: 1px solid #228b58;
        }


        /* =========================================================
           FORMS / TABLES
           ========================================================= */

        QGroupBox {
            background: #ffffff;
            border: 1px solid #dfe4ec;
            border-radius: 14px;
            margin-top: 10px;
            padding: 14px;
            font-weight: 700;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #3a4657;
        }

        QTableWidget {
            background: #fbfcfe;
            border: 1px solid #e0e5ed;
            border-radius: 10px;
            gridline-color: #edf0f4;
            alternate-background-color: #f7f9fb;
        }

        QHeaderView::section {
            background: #eff3f7;
            border: none;
            padding: 9px;
            font-weight: 700;
            color: #586174;
        }

        QLineEdit,
        QSpinBox,
        QComboBox {
            background: #ffffff;
            color: #172033;
            border: 1px solid #d4dae4;
            border-radius: 9px;
            padding: 7px 9px;
        }

        QLineEdit:focus,
        QSpinBox:focus,
        QComboBox:focus {
            border: 1px solid #2aa56a;
        }

        QPlainTextEdit {
            background: #0f1724;
            color: #dce5ee;
            border: none;
            border-radius: 10px;
            padding: 12px;
        }


        /* =========================================================
           SCROLLING / SPLITTERS
           ========================================================= */

        QScrollArea {
            background: transparent;
            border: none;
        }

        QScrollArea > QWidget > QWidget {
            background: transparent;
        }

        QSplitter::handle {
            background: #e4e8ee;
        }


        /* =========================================================
           SLIDER
           ========================================================= */

        QSlider::groove:horizontal {
            height: 5px;
            background: #dfe4ec;
            border-radius: 3px;
        }

        QSlider::handle:horizontal {
            width: 14px;
            margin: -5px 0;
            background: #2aa56a;
            border-radius: 7px;
        }
    )");
}

} // namespace


int Application::run(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("OSimX");
    app.setApplicationDisplayName("OSimX");
    app.setOrganizationName("OSimX Team");
    app.setWindowIcon(QIcon(":/resources/logo.png"));

    applyTheme(app);

    DatabaseManager::instance().open();

    auto* login = new LoginWindow;
    auto* signup = new SignupWindow;
    auto* mainWindow = new MainWindow;

    login->setAttribute(Qt::WA_DeleteOnClose, false);
    signup->setAttribute(Qt::WA_DeleteOnClose, false);
    mainWindow->setAttribute(Qt::WA_DeleteOnClose, false);

    login->showMaximized();
    signup->hide();
    mainWindow->hide();

    QObject::connect(
        login,
        &LoginWindow::signupRequested,
        login,
        [login, signup] {
            login->hide();
            signup->showMaximized();
        }
    );

    QObject::connect(
        signup,
        &SignupWindow::loginRequested,
        signup,
        [signup, login] {
            signup->hide();
            login->showMaximized();
        }
    );

    QObject::connect(
        login,
        &LoginWindow::loginSuccessful,
        login,
        [login, mainWindow] {
            login->hide();
            mainWindow->showMaximized();
        }
    );

    QObject::connect(
        signup,
        &SignupWindow::signupSuccessful,
        signup,
        [signup, mainWindow] {
            signup->hide();
            mainWindow->showMaximized();
        }
    );

    QObject::connect(
        mainWindow,
        &MainWindow::logoutRequested,
        mainWindow,
        [login, mainWindow] {
            mainWindow->hide();
            login->showMaximized();
            login->raise();
            login->activateWindow();
        }
    );

    QObject::connect(
        &app,
        &QApplication::aboutToQuit,
        [&] {
            login->close();
            signup->close();
            mainWindow->close();
            DatabaseManager::instance().close();
        }
    );

    const int code = app.exec();

    delete mainWindow;
    delete signup;
    delete login;

    return code;
}
