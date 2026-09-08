#include "LoadingWindow.h"
#include <QPixmap>
#include <QApplication>
#include <QScreen>

LoadingWindow::LoadingWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LoadingWindow::updateProgress);
    timer->start(30);
}

void LoadingWindow::setupUI()
{
    // White background
    setStyleSheet("background-color: white;");
    setFixedSize(700, 480);
    setWindowTitle("OSimX");
    setWindowIcon(QIcon(":/resources/logo.png"));

    // Center the window
    if (QScreen *screen = QApplication::primaryScreen()) {
        QRect screenGeometry = screen->availableGeometry();
        move((screenGeometry.width() - width()) / 2,
             (screenGeometry.height() - height()) / 2);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(30);

    // Logo
    logoLabel = new QLabel(this);
    QPixmap logo(":/resources/logo.png");
    logoLabel->setPixmap(logo.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    // Horizontal Progress Bar
    progressBar = new QProgressBar(this);
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setFixedSize(280, 14);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);

    progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #cccccc;
            border-radius: 7px;
            background-color: #f0f0f0;
        }
        QProgressBar::chunk {
            background-color: #00c853;
            border-radius: 6px;
        }
    )");

    mainLayout->addWidget(logoLabel);
    mainLayout->addWidget(progressBar, 0, Qt::AlignHCenter);
}

void LoadingWindow::updateProgress()
{
    progressValue += 2;
    progressBar->setValue(progressValue);

    if (progressValue >= 100) {
        timer->stop();
        checkLoginStatus();
    }
}

void LoadingWindow::checkLoginStatus()
{
    // Temporary: always treat as not logged in
    bool isLoggedIn = false;
    emit loadingFinished(isLoggedIn);
}