
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

    app.setStyleSheet(R"QSS(
        QWidget {
            font-family: "Segoe UI";
            font-size: 13px;
            color: #193729;
            background-color: #F3F4EE;
        }

        QMainWindow, QDialog {
            background-color: #F3F4EE;
        }

        QLabel, QCheckBox {
            background: transparent;
        }

        QLabel#brand {
            color: #183427;
            font-size: 27px;
            font-weight: 800;
        }

        QLabel#tagline {
            color: #607365;
            font-size: 13px;
        }

        QLabel#heroTitle {
            color: #183427;
            font-size: 30px;
            font-weight: 800;
        }

        QLabel#pageTitle {
            color: #183427;
            font-size: 27px;
            font-weight: 800;
        }

        QLabel#heroText,
        QLabel#pageSubtitle {
            color: #53675A;
            font-size: 14px;
        }

        QLabel#sectionTitle {
            color: #183427;
            font-size: 21px;
            font-weight: 750;
        }

        QLabel#sectionDescription {
            color: #607365;
            font-size: 13px;
        }

        QLabel#sectionEyebrow {
            color: #356B4A;
            font-size: 11px;
            font-weight: 700;
        }

        QLabel#accountPill,
        QLabel#stepPill {
            color: #356B4A;
            background-color: #E4EDE2;
            border: 1px solid #C9DBC9;
            border-radius: 12px;
            padding: 9px 14px;
            font-weight: 600;
        }

        QLabel#simulationIcon {
            color: #356B4A;
            background-color: #E1EDE0;
            border: 1px solid #C9DDCA;
            border-radius: 10px;
            padding: 9px 13px;
            font-size: 15px;
            font-weight: 800;
        }

        QLabel#simulationTitle {
            color: #193729;
            font-size: 19px;
            font-weight: 750;
        }

        QLabel#simulationDescription {
            color: #53675A;
            font-size: 13px;
        }

        QLabel#footerHint {
            color: #6C7F70;
            font-size: 12px;
        }

        QLabel#infoLabel {
            background-color: #E7EFE4;
            border: 1px solid #CDDECC;
            border-radius: 10px;
            padding: 12px;
            color: #365843;
        }

        QLabel#metricsValue {
            color: #193729;
            font-size: 14px;
            font-weight: 700;
        }

        QFrame#simulationCard {
            background-color: #FFFFFF;
            border: 1px solid #D9E4D7;
            border-radius: 16px;
        }

        QFrame#simulationCard:hover {
            border: 1px solid #86B08C;
            background-color: #FBFDF9;
        }

        QFrame#simulationCardDisabled {
            background-color: #EFF2EC;
            border: 1px solid #D9E4D7;
            border-radius: 16px;
        }

        QFrame#dashboardSection {
            background-color: #FFFFFF;
            border: 1px solid #DCE6D9;
            border-radius: 16px;
        }

        QFrame#dashboardHero {
            background-color: #183427;
            border: 1px solid #2D5840;
            border-radius: 18px;
        }

        QFrame#dashboardHero QLabel#heroTitle {
            color: #FFFFFF;
        }

        QFrame#dashboardHero QLabel#heroText {
            color: #D1E4D1;
        }

        QLabel#heroEyebrow {
            color: #A5C7A5;
            font-size: 11px;
            font-weight: 700;
        }

        QPushButton {
            background-color: #356B4A;
            color: #FFFFFF;
            border: 1px solid #356B4A;
            border-radius: 9px;
            padding: 9px 15px;
            font-size: 13px;
            font-weight: 700;
            min-height: 20px;
        }

        QPushButton:hover {
            background-color: #28583B;
            border-color: #28583B;
        }

        QPushButton:pressed {
            background-color: #20472F;
        }

        QPushButton:disabled {
            background-color: #D5DDD3;
            color: #839186;
            border-color: #D5DDD3;
        }

        QPushButton#secondaryButton {
            background-color: #FFFFFF;
            color: #28583B;
            border: 1px solid #C7D8C7;
        }

        QPushButton#secondaryButton:hover {
            background-color: #E8F1E6;
            border-color: #86B08C;
        }

        QPushButton#dangerButton {
            background-color: #FFFFFF;
            color: #AD4545;
            border: 1px solid #E8CDCD;
        }

        QPushButton#dangerButton:hover {
            background-color: #FFF1F1;
            border-color: #DCA8A8;
        }

        QGroupBox {
            background-color: #FFFFFF;
            border: 1px solid #DCE6D9;
            border-radius: 12px;
            margin-top: 15px;
            padding: 16px 12px 12px 12px;
            font-weight: 700;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 14px;
            padding: 0 7px;
            color: #28583B;
        }

        QLineEdit,
        QSpinBox,
        QDoubleSpinBox,
        QComboBox,
        QTextEdit {
            background-color: #FFFFFF;
            color: #193729;
            border: 1px solid #C8D7C8;
            border-radius: 8px;
            padding: 7px 10px;
            selection-background-color: #356B4A;
            selection-color: #FFFFFF;
        }

        QLineEdit:focus,
        QSpinBox:focus,
        QDoubleSpinBox:focus,
        QComboBox:focus,
        QTextEdit:focus {
            border: 1px solid #356B4A;
        }

        QComboBox::drop-down {
            border: none;
            width: 25px;
        }

        QTableWidget {
            background-color: #FFFFFF;
            alternate-background-color: #F5F8F3;
            color: #193729;
            border: 1px solid #DCE6D9;
            border-radius: 9px;
            gridline-color: #E8EEE6;
            selection-background-color: #DCEBDD;
            selection-color: #193729;
        }

        QTableWidget::item {
            padding: 7px;
        }

        QHeaderView::section {
            background-color: #E8EFE5;
            color: #365843;
            border: none;
            border-bottom: 1px solid #D4E0D1;
            padding: 10px;
            font-weight: 700;
        }

        QPlainTextEdit {
            background-color: #172B21;
            color: #E2EEE0;
            border: 1px solid #31503A;
            border-radius: 9px;
            padding: 12px;
            selection-background-color: #356B4A;
        }

        QCheckBox {
            color: #425B49;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            background-color: #FFFFFF;
            border: 1px solid #A9BEAC;
            border-radius: 4px;
        }

        QCheckBox::indicator:checked {
            background-color: #356B4A;
            border-color: #356B4A;
        }

        QScrollArea {
            border: none;
            background: transparent;
        }

        QScrollBar:vertical {
            background-color: transparent;
            width: 9px;
            margin: 3px;
        }

        QScrollBar::handle:vertical {
            background-color: #B1C4B1;
            min-height: 32px;
            border-radius: 3px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #89A68E;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
            border: none;
        }

        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: transparent;
        }

        QSplitter::handle {
            background-color: #DCE6D9;
        }

        QSlider::groove:horizontal {
            height: 5px;
            background-color: #D4E2D1;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            width: 15px;
            margin: -5px 0;
            background-color: #356B4A;
            border-radius: 7px;
        }

        QProgressBar {
            background-color: #E4ECE1;
            border: 1px solid #D2E0D0;
            border-radius: 7px;
            text-align: center;
            color: #193729;
        }

        QProgressBar::chunk {
            background-color: #356B4A;
            border-radius: 6px;
        }

        QTabWidget::pane {
            border: 1px solid #DCE6D9;
            border-radius: 10px;
            background-color: #FFFFFF;
        }

        QTabBar::tab {
            background-color: #E8EFE5;
            color: #53675A;
            padding: 10px 17px;
            margin-right: 3px;
            border-radius: 7px;
        }

        QTabBar::tab:selected {
            background-color: #356B4A;
            color: #FFFFFF;
        }
    )QSS");
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
            mainWindow->refreshSession();
            mainWindow->showMaximized();
        }
    );

    QObject::connect(
        signup,
        &SignupWindow::signupSuccessful,
        signup,
        [signup, mainWindow] {
            signup->hide();
            mainWindow->refreshSession();
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
