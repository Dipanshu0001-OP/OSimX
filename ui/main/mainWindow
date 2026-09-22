#include "MainWindow.h"
#include "database/AuthService.h"
#include "modules/cpu_scheduling/frontend/SchedulingWindow.h"
#include "modules/disk_scheduling/frontend/DiskSchedulingWindow.h"
#include <QFrame>
#include <QGridLayout>
#include <QIcon>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <functional>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("OSimX · Simulation Lab");
    setWindowIcon(QIcon(":/resources/logo.png"));
    setMinimumSize(1100, 700);

    pages = new QStackedWidget;
    hubPage = createHubPage();
    cpuPage = new SchedulingWindow;
    diskPage = new DiskSchedulingWindow;
    pages->addWidget(hubPage);
    pages->addWidget(cpuPage);
    pages->addWidget(diskPage);
    setCentralWidget(pages);

    connect(cpuPage, &SchedulingWindow::backRequested, this, &MainWindow::showHub);
    connect(diskPage, &DiskSchedulingWindow::backRequested, this, &MainWindow::showHub);
    showHub();
}

QWidget* MainWindow::createSimulationCard(const QString& icon,
                                          const QString& title,
                                          const QString& description,
                                          const QString& status,
                                          bool enabled,
                                          const std::function<void()>& onClicked)
{
    auto* card = new QFrame;
    card->setObjectName(enabled ? "simulationCard" : "simulationCardDisabled");
    card->setMinimumHeight(180);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(22, 20, 22, 20);
    layout->setSpacing(10);

    auto* iconLabel = new QLabel(icon);
    iconLabel->setObjectName("simulationIcon");
    auto* titleLabel = new QLabel(title);
    titleLabel->setObjectName("simulationTitle");
    auto* descriptionLabel = new QLabel(description);
    descriptionLabel->setObjectName("simulationDescription");
    descriptionLabel->setWordWrap(true);
    auto* statusLabel = new QLabel(status);
    statusLabel->setObjectName(enabled ? "statusActive" : "statusDisabled");

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel, 1);
    layout->addWidget(statusLabel, 0, Qt::AlignLeft);

    if (enabled) {
        auto* openButton = new QPushButton("Open simulation  →");
        openButton->setObjectName("secondaryButton");
        openButton->setCursor(Qt::PointingHandCursor);
        layout->addWidget(openButton, 0, Qt::AlignLeft);
        connect(openButton, &QPushButton::clicked, this, [onClicked] { onClicked(); });
    }

    return card;
}

QWidget* MainWindow::createHubPage()
{
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);

    auto* top = new QWidget;
    auto* topLayout = new QHBoxLayout(top);
    topLayout->setContentsMargins(26, 18, 26, 18);
    auto* brand = new QLabel("OSimX");
    brand->setObjectName("brand");
    auto* tagline = new QLabel("Interactive Operating System Simulator");
    tagline->setObjectName("tagline");
    topLayout->addWidget(brand);
    topLayout->addSpacing(12);
    topLayout->addWidget(tagline);
    topLayout->addStretch();

    QString userText = "Signed in";
    if (const auto user = AuthService::instance().currentUser())
        userText = QString("%1 · @%2").arg(user->fullName, user->username);
    auto* account = new QLabel(userText);
    account->setObjectName("accountPill");
    topLayout->addWidget(account);
    auto* logout = new QPushButton("Log out");
    logout->setObjectName("secondaryButton");
    logout->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(logout);
    root->addWidget(top);
    connect(logout, &QPushButton::clicked, this, [this] {
        AuthService::instance().logout();
        emit logoutRequested();
    });

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(30, 18, 30, 40);
    layout->setSpacing(18);

    auto* hero = new QLabel("Choose a simulation");
    hero->setObjectName("heroTitle");
    auto* heroText = new QLabel("Pick a topic to explore. Active modules open into guided, replayable simulations; the remaining modules are kept visible as the roadmap.");
    heroText->setObjectName("heroText");
    heroText->setWordWrap(true);
    heroText->setMaximumWidth(920);
    layout->addWidget(hero);
    layout->addWidget(heroText);

    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);

    auto* cpuCard = createSimulationCard("01", "CPU Scheduling",
        "FCFS, SJF, SRTF, Round Robin and Priority scheduling with a replayable Gantt timeline and decision logs.",
        "Available now", true, [this] { pages->setCurrentWidget(cpuPage); });
    auto* memoryCard = createSimulationCard("02", "Memory Management",
        "Reserved for allocation and page-replacement simulations.", "Coming soon", false, {});
    auto* fileCard = createSimulationCard("03", "File System",
        "Reserved for file allocation, directories, and file-system simulations.", "Coming soon", false, {});
    auto* diskCard = createSimulationCard("04", "Disk Scheduling",
        "FCFS, SSTF, SCAN, C-SCAN, LOOK and C-LOOK with seek-cost playback.",
        "Available now", true, [this] { pages->setCurrentWidget(diskPage); });
    auto* deadlockCard = createSimulationCard("05", "Deadlock",
        "Reserved for deadlock detection and avoidance simulations.", "Coming soon", false, {});

    grid->addWidget(cpuCard, 0, 0);
    grid->addWidget(memoryCard, 0, 1);
    grid->addWidget(fileCard, 1, 0);
    grid->addWidget(diskCard, 1, 1);
    grid->addWidget(deadlockCard, 2, 0);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    layout->addLayout(grid);

    auto* footer = new QLabel("More OS modules will be connected here as their native Qt implementations are completed.");
    footer->setObjectName("footerHint");
    footer->setWordWrap(true);
    layout->addWidget(footer);

    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::showHub()
{
    if (auto* account = hubPage->findChild<QLabel*>("accountPill")) {
        QString userText = "Signed in";
        if (const auto user = AuthService::instance().currentUser())
            userText = QString("%1 · @%2").arg(user->fullName, user->username);
        account->setText(userText);
    }
    pages->setCurrentWidget(hubPage);
}
