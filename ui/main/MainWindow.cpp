
#include "MainWindow.h"

#include "database/AuthService.h"
#include "database/ComparisonRepository.h"
#include "database/DatabaseManager.h"
#include "database/SimulationRepository.h"

#include "modules/cpu_scheduling/frontend/SchedulingWindow.h"
#include "modules/disk_scheduling/frontend/DiskSchedulingWindow.h"
#include "modules/memory_management/frontend/MemoryWindow.h"
#include "modules/file_system/frontend/FileSystemWindow.h"
#include "modules/deadlock/frontend/DeadlockWindow.h"

#include <QAbstractItemView>
#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>

namespace {

QLabel* textLabel(const QString& text,
                  const QString& name = QString())
{
    auto* label = new QLabel(text);
    if (!name.isEmpty())
        label->setObjectName(name);

    label->setWordWrap(true);
    label->setSizePolicy(
        QSizePolicy::Preferred,
        QSizePolicy::Minimum
    );
    return label;
}

QPushButton* actionButton(const QString& text,
                          const QString& name)
{
    auto* button = new QPushButton(text);
    button->setObjectName(name);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void configureTable(QTableWidget* table, int height)
{
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(
        QAbstractItemView::SelectRows
    );
    table->setSelectionMode(
        QAbstractItemView::SingleSelection
    );
    table->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setMinimumHeight(height);
    table->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
    table->horizontalHeader()->setHighlightSections(false);
    table->verticalHeader()->setDefaultSectionSize(38);
}

QFrame* sectionFrame(const QString& eyebrow,
                     const QString& heading,
                     const QString& description,
                     QVBoxLayout*& contentLayout)
{
    auto* frame = new QFrame;
    frame->setObjectName("dashboardSection");

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(11);

    layout->addWidget(textLabel(eyebrow, "sectionEyebrow"));
    layout->addWidget(textLabel(heading, "sectionTitle"));
    layout->addWidget(
        textLabel(description, "sectionDescription")
    );

    layout->addSpacing(8);
    contentLayout = layout;
    return frame;
}

QString formattedJson(const QString& value)
{
    const QJsonDocument document =
        QJsonDocument::fromJson(value.toUtf8());

    if (document.isNull())
        return value;

    return QString::fromUtf8(
        document.toJson(QJsonDocument::Indented)
    );
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("OSimX | Operating System Simulator");
    setWindowIcon(QIcon(":/resources/logo.png"));
    setMinimumSize(1100, 700);

    pages = new QStackedWidget;

    hubPage = createHubPage();
    cpuPage = new SchedulingWindow;
    diskPage = new DiskSchedulingWindow;
    memoryPage = new MemoryWindow;
    fileSystemPage = new FileSystemWindow;
    deadlockPage = new DeadlockWindow;

    pages->addWidget(hubPage);
    pages->addWidget(cpuPage);
    pages->addWidget(diskPage);
    pages->addWidget(memoryPage);
    pages->addWidget(fileSystemPage);
    pages->addWidget(deadlockPage);

    setCentralWidget(pages);

    connect(cpuPage,
            &SchedulingWindow::backRequested,
            this, &MainWindow::showHub);

    connect(diskPage,
            &DiskSchedulingWindow::backRequested,
            this, &MainWindow::showHub);

    connect(memoryPage,
            &MemoryWindow::backRequested,
            this, &MainWindow::showHub);

    connect(fileSystemPage,
            &FileSystemWindow::backRequested,
            this, &MainWindow::showHub);

    connect(deadlockPage,
            &DeadlockWindow::backRequested,
            this, &MainWindow::showHub);

    // Keep the History table synchronized immediately after saving.
    connect(deadlockPage, &DeadlockWindow::simulationSaved,
            this, &MainWindow::reloadHistory);
    connect(fileSystemPage, &FileSystemWindow::simulationSaved,
            this, &MainWindow::reloadHistory);

    connect(cpuPage, &SchedulingWindow::simulationSaved,
            this, &MainWindow::reloadHistory);
    connect(diskPage, &DiskSchedulingWindow::simulationSaved,
            this, &MainWindow::reloadHistory);
    connect(memoryPage, &MemoryWindow::simulationSaved,
            this, &MainWindow::reloadHistory);

    showHub();
}

QWidget* MainWindow::createSimulationCard(
    const QString& icon,
    const QString& title,
    const QString& description,
    const QString& status,
    bool enabled,
    const std::function<void()>& onClicked)
{
    // Keep the original function signature for
    // compatibility with MainWindow.h.
    Q_UNUSED(status);

    auto* card = new QFrame;
    card->setObjectName(
        enabled ? "simulationCard"
                : "simulationCardDisabled"
    );

    card->setMinimumHeight(205);
    card->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Preferred
    );

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(12);

    auto* number = textLabel(icon, "simulationIcon");
    number->setSizePolicy(
        QSizePolicy::Maximum,
        QSizePolicy::Fixed
    );

    auto* heading = textLabel(title, "simulationTitle");
    auto* body = textLabel(
        description, "simulationDescription"
    );

    layout->addWidget(number, 0, Qt::AlignLeft);
    layout->addSpacing(3);
    layout->addWidget(heading);
    layout->addWidget(body);
    layout->addStretch();

    if (enabled) {
        auto* open = actionButton(
            "Explore module", "secondaryButton"
        );

        layout->addWidget(open, 0, Qt::AlignLeft);

        connect(open, &QPushButton::clicked,
                this, [onClicked] {
            if (onClicked)
                onClicked();
        });
    }

    return card;
}

QWidget* MainWindow::createHubPage()
{
    auto* page = new QWidget;

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // HEADER
    auto* top = new QFrame;
    top->setObjectName("dashboardSection");

    auto* topLayout = new QHBoxLayout(top);
    topLayout->setContentsMargins(32, 15, 32, 15);
    topLayout->setSpacing(14);

    auto* brand = textLabel("OSimX", "brand");
    auto* tagline = textLabel(
        "Operating System Simulator", "tagline"
    );

    topLayout->addWidget(brand);
    topLayout->addSpacing(8);
    topLayout->addWidget(tagline);
    topLayout->addStretch(1);

    QString userText = "My account";

    if (const auto user =
            AuthService::instance().currentUser()) {
        userText = QString("%1  |  @%2")
            .arg(user->fullName, user->username);
    }

    auto* account = textLabel(
        userText, "accountPill"
    );

    topLayout->addWidget(account);

    auto* logout = actionButton(
        "Log out", "secondaryButton"
    );

    topLayout->addWidget(logout);
    root->addWidget(top);

    connect(logout, &QPushButton::clicked,
            this, [this] {
        AuthService::instance().logout();
        emit logoutRequested();
    });

    // SCROLLABLE DASHBOARD
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(34, 28, 34, 40);
    layout->setSpacing(22);
    layout->setAlignment(Qt::AlignTop);

    // HERO
    auto* hero = new QFrame;
    hero->setObjectName("dashboardHero");

    auto* heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(30, 28, 30, 30);
    heroLayout->setSpacing(10);

    heroLayout->addWidget(textLabel(
        "YOUR OSIMX WORKSPACE", "heroEyebrow"
    ));

    heroLayout->addWidget(textLabel(
        "Explore operating systems",
        "heroTitle"
    ));

    heroLayout->addWidget(textLabel(
        "Choose a topic, run a simulation, and "
        "see how different operating-system "
        "algorithms work.",
        "heroText"
    ));

    layout->addWidget(hero);

    // MODULE SECTION
    auto* moduleHeading = new QVBoxLayout;
    moduleHeading->setSpacing(5);

    moduleHeading->addWidget(textLabel(
        "SIMULATIONS", "sectionEyebrow"
    ));

    moduleHeading->addWidget(textLabel(
        "Choose a module", "sectionTitle"
    ));

    moduleHeading->addWidget(textLabel(
        "Explore the five core areas of operating systems.",
        "sectionDescription"
    ));

    layout->addLayout(moduleHeading);

    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);

    auto* cpuCard = createSimulationCard(
        "01",
        "CPU Scheduling",
        "Explore FCFS, SJF, SRTF, Round Robin, and "
        "Priority Scheduling. Follow process execution "
        "and examine scheduling results.",
        QString(),
        true,
        [this] { pages->setCurrentWidget(cpuPage); }
    );

    auto* diskCard = createSimulationCard(
        "02",
        "Disk Scheduling",
        "Explore FCFS, SSTF, SCAN, C-SCAN, LOOK, "
        "and C-LOOK. Visualize disk-head movement "
        "and compare seek distances.",
        QString(),
        true,
        [this] { pages->setCurrentWidget(diskPage); }
    );

    auto* memoryCard = createSimulationCard(
        "03",
        "Memory Management",
        "Explore memory allocation and page replacement. "
        "Observe block allocation, frame changes, "
        "and page faults.",
        QString(),
        true,
        [this] { pages->setCurrentWidget(memoryPage); }
    );

    auto* deadlockCard = createSimulationCard(
        "04",
        "Deadlock",
        "Analyze resource allocation, safe sequences, "
        "resource requests, and deadlock detection.",
        QString(),
        true,
        [this] { pages->setCurrentWidget(deadlockPage); }
    );

    auto* fileCard = createSimulationCard(
        "05",
        "File System",
        "Create files and folders, explore file "
        "allocation methods, and inspect how "
        "disk blocks are used.",
        QString(),
        true,
        [this] {
            pages->setCurrentWidget(fileSystemPage);
        }
    );

    grid->addWidget(cpuCard, 0, 0);
    grid->addWidget(diskCard, 0, 1);
    grid->addWidget(memoryCard, 1, 0);
    grid->addWidget(deadlockCard, 1, 1);
    grid->addWidget(fileCard, 2, 0);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    layout->addLayout(grid);

    // SAVED SIMULATIONS
    QVBoxLayout* historyLayout = nullptr;

    auto* historySection = sectionFrame(
        "YOUR WORK",
        "Saved simulations",
        "Review previous simulations and examine "
        "their results.",
        historyLayout
    );

    historyTable = new QTableWidget(0, 4);
    historyTable->setHorizontalHeaderLabels({
        "Date", "Module", "Algorithm", "Summary"
    });

    configureTable(historyTable, 210);

    historyTable->horizontalHeader()
        ->setStretchLastSection(true);

    historyTable->horizontalHeader()
        ->setSectionResizeMode(
            0, QHeaderView::ResizeToContents
        );

    historyTable->horizontalHeader()
        ->setSectionResizeMode(
            1, QHeaderView::ResizeToContents
        );

    historyTable->horizontalHeader()
        ->setSectionResizeMode(
            2, QHeaderView::ResizeToContents
        );

    historyLayout->addWidget(historyTable);

    historyStatus = textLabel(
        QString(), "footerHint"
    );

    historyLayout->addWidget(historyStatus);

    auto* historyActions = new QHBoxLayout;
    historyActions->setSpacing(10);

    auto* viewButton = actionButton(
        "View simulation", "secondaryButton"
    );

    auto* removeButton = actionButton(
        "Delete", "dangerButton"
    );

    historyActions->addWidget(viewButton);
    historyActions->addWidget(removeButton);
    historyActions->addStretch();

    historyLayout->addLayout(historyActions);
    layout->addWidget(historySection);

    connect(viewButton, &QPushButton::clicked,
            this, &MainWindow::viewSelectedSimulation);

    connect(removeButton, &QPushButton::clicked,
            this, &MainWindow::removeSelectedSimulation);

    connect(historyTable,
            &QTableWidget::cellDoubleClicked,
            this, [this](int, int) {
        viewSelectedSimulation();
    });

    // SAVED COMPARISONS
    QVBoxLayout* comparisonLayout = nullptr;

    auto* comparisonSection = sectionFrame(
        "ANALYSIS",
        "Saved comparisons",
        "Revisit algorithm comparisons and explore "
        "the differences between their results.",
        comparisonLayout
    );

    comparisonTable = new QTableWidget(0, 3);

    comparisonTable->setHorizontalHeaderLabels({
        "Date", "Module", "Comparison"
    });

    configureTable(comparisonTable, 175);

    comparisonTable->horizontalHeader()
        ->setStretchLastSection(true);

    comparisonTable->horizontalHeader()
        ->setSectionResizeMode(
            0, QHeaderView::ResizeToContents
        );

    comparisonTable->horizontalHeader()
        ->setSectionResizeMode(
            1, QHeaderView::ResizeToContents
        );

    comparisonLayout->addWidget(comparisonTable);

    comparisonStatus = textLabel(
        QString(), "footerHint"
    );

    comparisonLayout->addWidget(comparisonStatus);

    auto* comparisonActions = new QHBoxLayout;
    comparisonActions->setSpacing(10);

    auto* viewComparison = actionButton(
        "View comparison", "secondaryButton"
    );

    auto* removeComparison = actionButton(
        "Delete", "dangerButton"
    );

    comparisonActions->addWidget(viewComparison);
    comparisonActions->addWidget(removeComparison);
    comparisonActions->addStretch();

    comparisonLayout->addLayout(comparisonActions);
    layout->addWidget(comparisonSection);

    connect(viewComparison, &QPushButton::clicked,
            this, &MainWindow::viewSelectedComparison);

    connect(removeComparison, &QPushButton::clicked,
            this, &MainWindow::removeSelectedComparison);

    connect(comparisonTable,
            &QTableWidget::cellDoubleClicked,
            this, [this](int, int) {
        viewSelectedComparison();
    });

    // LOGIN HISTORY
    QVBoxLayout* loginLayout = nullptr;

    auto* loginSection = sectionFrame(
        "ACCOUNT",
        "Sign-in history",
        "See when your account was accessed.",
        loginLayout
    );

    loginTable = new QTableWidget(0, 1);
    loginTable->setHorizontalHeaderLabels({
        "Date and time"
    });

    configureTable(loginTable, 145);

    loginTable->horizontalHeader()
        ->setStretchLastSection(true);

    loginLayout->addWidget(loginTable);

    loginStatus = textLabel(
        QString(), "footerHint"
    );

    loginLayout->addWidget(loginStatus);
    layout->addWidget(loginSection);

    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    return page;
}

void MainWindow::showHub()
{
    if (auto* account =
            hubPage->findChild<QLabel*>("accountPill")) {
        QString userText = "My account";

        if (const auto user =
                AuthService::instance().currentUser()) {
            userText = QString("%1  |  @%2")
                .arg(user->fullName, user->username);
        }

        account->setText(userText);
    }

    pages->setCurrentWidget(hubPage);

    reloadHistory();
    reloadComparisons();
    reloadLoginHistory();
}

void MainWindow::refreshSession()
{
    showHub();
}

void MainWindow::reloadHistory()
{
    if (!historyTable || !historyStatus)
        return;

    historyTable->setRowCount(0);

    if (!AuthService::instance().isLoggedIn()) {
        historyStatus->setText(
            "Sign in to view your saved simulations."
        );
        return;
    }

    if (!DatabaseManager::instance().isOpen() &&
        !DatabaseManager::instance().open()) {
        historyStatus->setText(
            "Saved simulations are temporarily unavailable."
        );
        return;
    }

    QString error;

    const auto simulations =
        SimulationRepository::instance()
            .listForCurrentUser(error);

    if (!error.isEmpty()) {
        historyStatus->setText(
            "Could not load saved simulations: " + error
        );
        return;
    }

    for (const auto& simulation : simulations) {
        const int row = historyTable->rowCount();
        historyTable->insertRow(row);

        auto* when = new QTableWidgetItem(
            simulation.createdAt.toLocalTime()
                .toString("dd MMM yyyy, HH:mm")
        );

        when->setData(Qt::UserRole, simulation.id);

        historyTable->setItem(row, 0, when);
        historyTable->setItem(row, 1,
            new QTableWidgetItem(simulation.module));
        historyTable->setItem(row, 2,
            new QTableWidgetItem(simulation.algorithm));
        historyTable->setItem(row, 3,
            new QTableWidgetItem(simulation.summary));
    }

    historyStatus->setText(
        QString("%1 saved simulations")
            .arg(simulations.size())
    );
}

void MainWindow::reloadComparisons()
{
    if (!comparisonTable || !comparisonStatus)
        return;

    comparisonTable->setRowCount(0);

    if (!AuthService::instance().isLoggedIn()) {
        comparisonStatus->setText(
            "Sign in to view your saved comparisons."
        );
        return;
    }

    QString error;

    const auto comparisons =
        ComparisonRepository::instance()
            .listForCurrentUser(error);

    if (!error.isEmpty()) {
        comparisonStatus->setText(
            "Could not load comparisons: " + error
        );
        return;
    }

    for (const auto& comparison : comparisons) {
        const int row = comparisonTable->rowCount();
        comparisonTable->insertRow(row);

        auto* when = new QTableWidgetItem(
            comparison.createdAt.toLocalTime()
                .toString("dd MMM yyyy, HH:mm")
        );

        when->setData(Qt::UserRole, comparison.id);

        comparisonTable->setItem(row, 0, when);
        comparisonTable->setItem(row, 1,
            new QTableWidgetItem(comparison.module));
        comparisonTable->setItem(row, 2,
            new QTableWidgetItem(comparison.title));
    }

    comparisonStatus->setText(
        QString("%1 saved comparisons")
            .arg(comparisons.size())
    );
}

void MainWindow::reloadLoginHistory()
{
    if (!loginTable || !loginStatus)
        return;

    loginTable->setRowCount(0);

    if (!AuthService::instance().isLoggedIn()) {
        loginStatus->setText(
            "Sign in to view your account activity."
        );
        return;
    }

    QString error;

    const auto records =
        AuthService::instance().loginHistory(error);

    if (!error.isEmpty()) {
        loginStatus->setText(
            "Could not load sign-in history: " + error
        );
        return;
    }

    for (const auto& record : records) {
        const int row = loginTable->rowCount();
        loginTable->insertRow(row);

        loginTable->setItem(
            row, 0,
            new QTableWidgetItem(
                record.loggedInAt.toLocalTime()
                    .toString("dd MMM yyyy, HH:mm:ss")
            )
        );
    }

    loginStatus->setText(
        QString("%1 sign-ins recorded")
            .arg(records.size())
    );
}


void MainWindow::viewSelectedSimulation()
{
    // Check whether a simulation is selected.
    const int row = historyTable
        ? historyTable->currentRow()
        : -1;

    if (row < 0 || !historyTable->item(row, 0)) {
        QMessageBox::information(
            this,
            "Saved Simulations",
            "Please select a simulation to open."
        );
        return;
    }

    // Read the saved simulation ID.
    const int simulationId =
        historyTable->item(row, 0)
            ->data(Qt::UserRole)
            .toInt();

    if (simulationId <= 0) {
        QMessageBox::warning(
            this,
            "Invalid Simulation",
            "The selected simulation has an invalid ID."
        );
        return;
    }

    // Load the simulation from the database.
    SavedSimulation simulation;
    QString errorMessage;

    const bool loaded =
        SimulationRepository::instance().load(
            simulationId,
            simulation,
            errorMessage
        );

    if (!loaded) {
        QMessageBox::warning(
            this,
            "Unable to Open Simulation",
            errorMessage.isEmpty()
                ? "Could not load the saved simulation."
                : errorMessage
        );
        return;
    }

    // Parse JSON safely.
    auto parseObject = [](
        const QString& jsonText,
        QJsonObject& object
    ) -> bool {

        if (jsonText.trimmed().isEmpty())
            return false;

        QJsonParseError parseError;

        const QJsonDocument document =
            QJsonDocument::fromJson(
                jsonText.toUtf8(),
                &parseError
            );

        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject()) {
            return false;
        }

        object = document.object();
        return true;
    };

    QJsonObject inputData;
    QJsonObject outputData;
    QJsonObject payload;

    const bool hasInputs =
        parseObject(simulation.inputData, inputData);

    const bool hasOutputs =
        parseObject(simulation.outputData, outputData);

    const bool hasPayload =
        parseObject(simulation.payload, payload);

    // Support records where input/output data
    // was stored inside the payload object.
    if (!hasInputs && hasPayload) {
        const QJsonValue value =
            payload.value("inputData");

        if (value.isObject())
            inputData = value.toObject();
    }

    if (!hasOutputs && hasPayload) {
        const QJsonValue value =
            payload.value("outputData");

        if (value.isObject())
            outputData = value.toObject();
    }

    const QString module =
        simulation.module.trimmed();

    // ---------------------------------------
    // DEADLOCK SIMULATIONS
    // ---------------------------------------

    if (module.compare(
            "Deadlock",
            Qt::CaseInsensitive
        ) == 0 ||
        module.compare(
            "Deadlock Management",
            Qt::CaseInsensitive
        ) == 0) {

        if (!deadlockPage) {
            QMessageBox::warning(
                this,
                "Module Unavailable",
                "The Deadlock module is not available."
            );
            return;
        }

        if (inputData.isEmpty() ||
            outputData.isEmpty()) {
            QMessageBox::warning(
                this,
                "Cannot Restore Simulation",
                "This saved Deadlock simulation does not "
                "contain the structured inputs and outputs "
                "required for restoration."
            );
            return;
        }

        const bool restored =
            deadlockPage->restoreSimulation(
                inputData,
                outputData
            );

        if (!restored) {
            QMessageBox::warning(
                this,
                "Restore Failed",
                "The Deadlock simulation could not be "
                "restored. The saved data may be incomplete "
                "or incompatible with this version."
            );
            return;
        }

        // Open the original module.
        pages->setCurrentWidget(deadlockPage);

        return;
    }

    // ---------------------------------------
    // FILE SYSTEM SIMULATIONS
    // ---------------------------------------

    if (module.compare(
            "File System",
            Qt::CaseInsensitive
        ) == 0 ||
        module.compare(
            "File System Management",
            Qt::CaseInsensitive
        ) == 0 ||
        module.compare(
            "FileSystem",
            Qt::CaseInsensitive
        ) == 0) {

        if (!fileSystemPage) {
            QMessageBox::warning(
                this,
                "Module Unavailable",
                "The File System module is not available."
            );
            return;
        }

        if (inputData.isEmpty() ||
            outputData.isEmpty()) {
            QMessageBox::warning(
                this,
                "Cannot Restore Simulation",
                "This saved File System simulation does "
                "not contain the filesystem snapshot "
                "required for restoration."
            );
            return;
        }

        if (!inputData.value("snapshot").isObject()) {
            QMessageBox::warning(
                this,
                "Snapshot Missing",
                "The selected simulation does not contain "
                "a complete filesystem snapshot. "
                "It may have been saved using an older "
                "version of OSimX."
            );
            return;
        }

        const bool restored =
            fileSystemPage->restoreSimulation(
                inputData,
                outputData
            );

        if (!restored) {
            QMessageBox::warning(
                this,
                "Restore Failed",
                "The File System simulation could not "
                "be restored. Please check that the "
                "saved snapshot is valid."
            );
            return;
        }

        // Open the original File System module.
        pages->setCurrentWidget(fileSystemPage);

        return;
    }

    // Existing CPU, Disk and Memory saves keep their complete results in payload.
    if (module.compare("CPU Scheduling", Qt::CaseInsensitive) == 0) {
        if (!hasPayload || !cpuPage->restoreSimulation(payload)) {
            QMessageBox::warning(this, "Cannot Restore Simulation",
                "CPU Scheduling saved data is incomplete or incompatible.");
            return;
        }
        pages->setCurrentWidget(cpuPage);
        return;
    }
    if (module.compare("Disk Scheduling", Qt::CaseInsensitive) == 0) {
        if (!hasPayload || !diskPage->restoreSimulation(payload)) {
            QMessageBox::warning(this, "Cannot Restore Simulation",
                "Disk Scheduling saved data is incomplete or incompatible.");
            return;
        }
        pages->setCurrentWidget(diskPage);
        return;
    }
    if (module.compare("Memory Allocation", Qt::CaseInsensitive) == 0 ||
        module.compare("Page Replacement", Qt::CaseInsensitive) == 0) {
        if (!hasPayload || !memoryPage->restoreSimulation(payload)) {
            QMessageBox::warning(this, "Cannot Restore Simulation",
                "Memory Management saved data is incomplete or incompatible.");
            return;
        }
        pages->setCurrentWidget(memoryPage);
        return;
    }

    // ---------------------------------------
    // OTHER MODULES
    // ---------------------------------------

    // CPU, Disk Scheduling and Memory Management
    // require their own restoreSimulation() support.
    // Do not silently show raw JSON as a substitute.

    QMessageBox::information(
        this,
        "Module Restoration Not Available",
        QString(
            "The saved simulation belongs to '%1'.\n\n"
            "Direct restoration has been implemented "
            "for Deadlock and File System simulations.\n\n"
            "The %1 module still needs its own "
            "saved-state restoration integration."
        ).arg(module)
    );
}


void MainWindow::removeSelectedSimulation()
{
    const int row =
        historyTable ? historyTable->currentRow() : -1;

    if (row < 0 || !historyTable->item(row, 0)) {
        QMessageBox::information(
            this, "Saved simulations",
            "Select a simulation to delete."
        );
        return;
    }

    if (QMessageBox::question(
            this,
            "Delete simulation",
            "Delete this saved simulation permanently?"
        ) != QMessageBox::Yes) {
        return;
    }

    const int id = historyTable->item(row, 0)
        ->data(Qt::UserRole).toInt();

    QString error;

    if (!SimulationRepository::instance()
             .remove(id, error)) {
        QMessageBox::warning(
            this, "Unable to delete simulation", error
        );
        return;
    }

    reloadHistory();
}

void MainWindow::viewSelectedComparison()
{
    const int row =
        comparisonTable
            ? comparisonTable->currentRow() : -1;

    if (row < 0 || !comparisonTable->item(row, 0)) {
        QMessageBox::information(
            this, "Saved comparisons",
            "Select a comparison to view."
        );
        return;
    }

    const int id = comparisonTable->item(row, 0)
        ->data(Qt::UserRole).toInt();

    SavedComparison comparison;
    QString error;

    if (!ComparisonRepository::instance()
             .load(id, comparison, error)) {
        QMessageBox::warning(
            this, "Unable to open comparison", error
        );
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(
        comparison.module + " | " + comparison.title
    );
    dialog.resize(760, 560);

    auto* content = new QVBoxLayout(&dialog);
    content->setContentsMargins(24, 24, 24, 24);
    content->setSpacing(14);

    content->addWidget(textLabel(
        comparison.title, "sectionTitle"
    ));

    content->addWidget(textLabel(
        comparison.module + " | " +
        comparison.createdAt.toLocalTime()
            .toString("dd MMM yyyy, HH:mm"),
        "sectionDescription"
    ));

    auto* details = new QTextEdit(&dialog);
    details->setReadOnly(true);
    details->setPlainText(
        formattedJson(comparison.comparisonData)
    );

    content->addWidget(details, 1);

    auto* close = actionButton(
        "Close", "secondaryButton"
    );

    connect(close, &QPushButton::clicked,
            &dialog, &QDialog::accept);

    content->addWidget(close, 0, Qt::AlignRight);

    dialog.exec();
}

void MainWindow::removeSelectedComparison()
{
    const int row =
        comparisonTable
            ? comparisonTable->currentRow() : -1;

    if (row < 0 || !comparisonTable->item(row, 0)) {
        QMessageBox::information(
            this, "Saved comparisons",
            "Select a comparison to delete."
        );
        return;
    }

    if (QMessageBox::question(
            this,
            "Delete comparison",
            "Delete this saved comparison permanently?"
        ) != QMessageBox::Yes) {
        return;
    }

    const int id = comparisonTable->item(row, 0)
        ->data(Qt::UserRole).toInt();

    QString error;

    if (!ComparisonRepository::instance()
             .remove(id, error)) {
        QMessageBox::warning(
            this, "Unable to delete comparison", error
        );
        return;
    }

    reloadComparisons();
}
