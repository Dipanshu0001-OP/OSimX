#include "MemoryWindow.h"
#include "database/SimulationRepository.h"
#include "modules/memory_management/backend/FirstFit.h"
#include "modules/memory_management/backend/BestFit.h"
#include "modules/memory_management/backend/WorstFit.h"
#include "modules/memory_management/backend/FIFO.h"
#include "modules/memory_management/backend/LRU.h"
#include "modules/memory_management/backend/Optimal.h"
#include <QAbstractItemView>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSplitter>
#include <QSet>
#include <QStringList>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <initializer_list>
#include <utility>
#include <functional>
#include <memory>

namespace {
QLabel* makeCaption(const QString& text)
{
    auto* label = new QLabel(text);
    label->setObjectName("caption");
    return label;
}

QString niceNumber(double value, int decimals = 2)
{
    return QString::number(value, 'f', decimals);
}

// "100, 500, 200" -> {100, 500, 200}. Rejects the whole line if any token
// is not a whole number, so bad input is not silently dropped.
std::vector<int> parseIntList(const QString& text, bool& ok, bool positiveOnly)
{
    ok = true;
    std::vector<int> values;
    const QStringList parts = text.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        ok = false;
        return values;
    }
    for (const QString& part : parts) {
        bool tokenOk = false;
        const int value = part.toInt(&tokenOk);
        if (!tokenOk || value < 0 || (positiveOnly && value <= 0)) {
            ok = false;
            return {};
        }
        values.push_back(value);
    }
    return values;
}
}

MemoryWindow::MemoryWindow(QWidget* parent) : QWidget(parent)
{
    setupUI();
    setupProcessTable();
    updateAllocationAlgorithmInfo();
    updatePageAlgorithmInfo();
}

void MemoryWindow::setupUI()
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* header = new QWidget;
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(22, 16, 22, 16);
    auto* backButton = new QPushButton("‹  Simulations");
    backButton->setObjectName("secondaryButton");
    backButton->setCursor(Qt::PointingHandCursor);
    auto* titleBlock = new QVBoxLayout;
    auto* title = new QLabel("Memory Management");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Allocate processes to memory blocks, then walk through page replacement, one step at a time.");
    subtitle->setObjectName("pageSubtitle");
    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);
    headerLayout->addWidget(backButton, 0, Qt::AlignTop);
    headerLayout->addSpacing(12);
    headerLayout->addLayout(titleBlock);
    headerLayout->addStretch();
    outer->addWidget(header);
    connect(backButton, &QPushButton::clicked, this, &MemoryWindow::backRequested);

    tabs = new QTabWidget;
    tabs->addTab(buildAllocationTab(), "Memory Allocation");
    tabs->addTab(buildPageReplacementTab(), "Page Replacement");

    outer->addWidget(tabs, 1);
}

QWidget* MemoryWindow::buildAllocationTab()
{
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto* content = new QWidget;
    auto* main = new QVBoxLayout(content);
    main->setContentsMargins(22, 14, 22, 28);
    main->setSpacing(14);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->setMinimumHeight(290);

    auto* inputBox = new QGroupBox("Processes to allocate");
    auto* inputLayout = new QVBoxLayout(inputBox);
    inputLayout->setContentsMargins(14, 18, 14, 14);
    processTable = new QTableWidget;
    processTable->setColumnCount(2);
    processTable->setHorizontalHeaderLabels({"PID", "Size"});
    processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    processTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    processTable->verticalHeader()->setVisible(false);
    processTable->setAlternatingRowColors(true);
    processTable->setMinimumHeight(225);
    processTable->verticalScrollBar()->setSingleStep(32);
    processTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inputLayout->addWidget(processTable, 1);

    auto* rowButtons = new QHBoxLayout;
    addProcessButton = new QPushButton("＋ Add Process");
    removeProcessButton = new QPushButton("− Delete Selected");
    clearProcessButton = new QPushButton("Clear All");
    addProcessButton->setObjectName("secondaryButton");
    removeProcessButton->setObjectName("secondaryButton");
    clearProcessButton->setObjectName("dangerButton");
    rowButtons->addWidget(addProcessButton);
    rowButtons->addWidget(removeProcessButton);
    rowButtons->addWidget(clearProcessButton);
    rowButtons->addStretch();
    inputLayout->addLayout(rowButtons);

    auto* controlBox = new QGroupBox("Memory & algorithm");
    auto* controlLayout = new QVBoxLayout(controlBox);
    controlLayout->setContentsMargins(14, 18, 14, 14);

    controlLayout->addWidget(makeCaption("Block sizes (comma separated)"));
    blockSizesEdit = new QLineEdit("100, 500, 200, 300, 600");
    controlLayout->addWidget(blockSizesEdit);

    controlLayout->addWidget(makeCaption("Choose an allocation policy"));
    allocationAlgorithmCombo = new QComboBox;
    allocationAlgorithmCombo->addItems({"First Fit", "Best Fit", "Worst Fit"});
    controlLayout->addWidget(allocationAlgorithmCombo);

    allocationAlgorithmInfo = new QLabel;
    allocationAlgorithmInfo->setWordWrap(true);
    allocationAlgorithmInfo->setObjectName("infoLabel");
    controlLayout->addWidget(allocationAlgorithmInfo);
    controlLayout->addStretch();

    runAllocationButton = new QPushButton("▶  Run Allocation");
    runAllocationButton->setObjectName("primaryButton");
    runAllocationButton->setMinimumHeight(44);
    controlLayout->addWidget(runAllocationButton);
    auto* compareAllocationButton = new QPushButton("Compare All Allocation Algorithms");
    compareAllocationButton->setObjectName("secondaryButton");
    compareAllocationButton->setMinimumHeight(40);
    controlLayout->addWidget(compareAllocationButton);
    connect(compareAllocationButton, &QPushButton::clicked, this, [this]() {
        std::vector<MemoryProcess> processes;
        QSet<int> ids;
        for (int row = 0; row < processTable->rowCount(); ++row) {
            auto* idItem = processTable->item(row, 0);
            auto* sizeItem = processTable->item(row, 1);
            QString idText = idItem ? idItem->text().trimmed() : QString();
            if (idText.startsWith('P', Qt::CaseInsensitive)) idText.remove(0, 1);
            bool idOk = false, sizeOk = false;
            int id = idText.toInt(&idOk);
            int size = sizeItem ? sizeItem->text().trimmed().toInt(&sizeOk) : 0;
            if (!idOk || !sizeOk || id <= 0 || size <= 0 || ids.contains(id)) {
                QMessageBox::warning(this, "Invalid process", "Enter unique positive PIDs and positive process sizes.");
                return;
            }
            ids.insert(id);
            processes.push_back({id, size});
        }
        bool ok = false;
        auto blocks = parseIntList(blockSizesEdit->text(), ok, true);
        if (!ok || processes.empty()) {
            QMessageBox::warning(this, "Invalid input", "Enter processes and positive memory block sizes first.");
            return;
        }
        FirstFit first; BestFit best; WorstFit worst;
        const auto a = first.allocate(blocks, processes);
        const auto b = best.allocate(blocks, processes);
        const auto c = worst.allocate(blocks, processes);
        auto* dialog = new QDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowTitle("Memory Allocation Comparison");
        dialog->resize(720, 360);
        auto* layout = new QVBoxLayout(dialog);
        auto* intro = new QLabel("All algorithms use the same process order and memory blocks.\n"
            "More successful allocations are better; fragmentation alone does not determine the winner.");
        intro->setWordWrap(true);
        layout->addWidget(intro);
        auto* table = new QTableWidget(3, 5, dialog);
        table->setHorizontalHeaderLabels({"Algorithm", "Allocated", "Failed", "Free memory", "Fragmentation"});
        const AllocationResult results[] = {a, b, c};
        const QString names[] = {"First Fit", "Best Fit", "Worst Fit"};
        for (int i = 0; i < 3; ++i) {
            const auto& r = results[i];
            const QString cells[] = {names[i], QString::number(r.allocatedCount),
                QString::number(r.failedCount), QString::number(r.totalFreeMemory),
                QString::number(r.externalFragmentation)};
            for (int j = 0; j < 5; ++j) table->setItem(i, j, new QTableWidgetItem(cells[j]));
        }
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        layout->addWidget(table);
        auto* close = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(close, &QDialogButtonBox::rejected, dialog, &QDialog::close);
        layout->addWidget(close);
        dialog->show();
    });

    splitter->addWidget(inputBox);
    splitter->addWidget(controlBox);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({680, 420});
    main->addWidget(splitter);

    auto* timelineBox = new QGroupBox("Interactive timeline");
    auto* timelineLayout = new QVBoxLayout(timelineBox);
    timelineLayout->setContentsMargins(12, 18, 12, 12);
    auto* timelineScroll = new QScrollArea;
    timelineScroll->setWidgetResizable(false);
    timelineScroll->setFrameShape(QFrame::NoFrame);
    timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    blockWidget = new MemoryBlockWidget;
    timelineScroll->setWidget(blockWidget);
    timelineLayout->addWidget(timelineScroll);
    allocationPlayback = new PlaybackBar;
    timelineLayout->addWidget(allocationPlayback);
    main->addWidget(timelineBox);

    auto* statusRow = new QHBoxLayout;
    allocationStepLabel = new QLabel("No simulation loaded");
    allocationStepLabel->setObjectName("stepPill");
    allocationStatusLabel = new QLabel("Add processes and block sizes, then run the allocation.");
    allocationStatusLabel->setWordWrap(true);
    statusRow->addWidget(allocationStepLabel, 0);
    statusRow->addWidget(allocationStatusLabel, 1);
    main->addLayout(statusRow);

    auto* metricsBox = new QGroupBox("Run metrics");
    auto* metricsLayout = new QHBoxLayout(metricsBox);
    metricsLayout->setContentsMargins(14, 18, 14, 14);
    allocationMetricsLabel = new QLabel("Allocated --   •   Failed --   •   Free memory --   •   External fragmentation --");
    allocationMetricsLabel->setObjectName("metricsValue");
    metricsLayout->addWidget(allocationMetricsLabel);
    main->addWidget(metricsBox);

    auto* logsBox = new QGroupBox("Decision log · revealed step by step");
    auto* logsLayout = new QVBoxLayout(logsBox);
    logsLayout->setContentsMargins(12, 18, 12, 12);
    allocationLogs = new SimulationLogWidget;
    logsLayout->addWidget(allocationLogs);
    main->addWidget(logsBox);

    auto* resultsBox = new QGroupBox("Allocation results");
    auto* resultsLayout = new QVBoxLayout(resultsBox);
    resultsLayout->setContentsMargins(12, 18, 12, 12);
    allocationResultsTable = new QTableWidget;
    allocationResultsTable->setColumnCount(4);
    allocationResultsTable->setHorizontalHeaderLabels({"PID", "Size", "Block", "Free blocks after"});
    allocationResultsTable->setMinimumHeight(210);
    allocationResultsTable->verticalHeader()->setVisible(false);
    allocationResultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    allocationResultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsLayout->addWidget(allocationResultsTable);
    main->addWidget(resultsBox);

    auto* saveRow = new QHBoxLayout;
    saveRow->addStretch();
    saveAllocationButton = new QPushButton("Save Simulation");
    saveAllocationButton->setObjectName("primaryButton");
    saveAllocationButton->setMinimumHeight(40);
    saveAllocationButton->setEnabled(false);
    saveRow->addWidget(saveAllocationButton);
    main->addLayout(saveRow);

    content->setMinimumWidth(980);
    scroll->setWidget(content);

    connect(addProcessButton, &QPushButton::clicked, this, &MemoryWindow::addProcessRow);
    connect(removeProcessButton, &QPushButton::clicked, this, &MemoryWindow::removeSelectedProcessRows);
    connect(clearProcessButton, &QPushButton::clicked, this, &MemoryWindow::clearProcessRows);
    connect(runAllocationButton, &QPushButton::clicked, this, &MemoryWindow::runAllocation);
    connect(allocationAlgorithmCombo, &QComboBox::currentTextChanged, this, &MemoryWindow::onAllocationAlgorithmChanged);
    connect(allocationPlayback, &PlaybackBar::indexChanged, this, &MemoryWindow::onAllocationPlaybackIndex);
    connect(saveAllocationButton, &QPushButton::clicked, this, &MemoryWindow::saveAllocationSimulation);

    return scroll;
}

QWidget* MemoryWindow::buildPageReplacementTab()
{
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto* content = new QWidget;
    auto* main = new QVBoxLayout(content);
    main->setContentsMargins(22, 14, 22, 28);
    main->setSpacing(14);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->setMinimumHeight(220);

    auto* inputBox = new QGroupBox("Page reference string");
    auto* inputLayout = new QVBoxLayout(inputBox);
    inputLayout->setContentsMargins(14, 18, 14, 14);
    inputLayout->addWidget(makeCaption("Pages, in the order they are referenced"));
    referenceStringEdit = new QLineEdit("7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1");
    referenceStringEdit->setMinimumHeight(36);
    inputLayout->addWidget(referenceStringEdit);
    inputLayout->addWidget(makeCaption("This is the classic textbook example. Edit it to try your own sequence."));
    inputLayout->addStretch();

    auto* controlBox = new QGroupBox("Frames & algorithm");
    auto* controlLayout = new QVBoxLayout(controlBox);
    controlLayout->setContentsMargins(14, 18, 14, 14);

    auto* frameRow = new QHBoxLayout;
    frameRow->addWidget(makeCaption("Number of frames"));
    frameCountSpinBox = new QSpinBox;
    frameCountSpinBox->setRange(1, 10);
    frameCountSpinBox->setValue(3);
    frameRow->addWidget(frameCountSpinBox);
    controlLayout->addLayout(frameRow);

    controlLayout->addWidget(makeCaption("Choose a page replacement policy"));
    pageAlgorithmCombo = new QComboBox;
    pageAlgorithmCombo->addItems({"FIFO", "LRU", "Optimal"});
    controlLayout->addWidget(pageAlgorithmCombo);

    pageAlgorithmInfo = new QLabel;
    pageAlgorithmInfo->setWordWrap(true);
    pageAlgorithmInfo->setObjectName("infoLabel");
    controlLayout->addWidget(pageAlgorithmInfo);
    controlLayout->addStretch();

    runPageButton = new QPushButton("▶  Run Page Replacement");
    runPageButton->setObjectName("primaryButton");
    runPageButton->setMinimumHeight(44);
    controlLayout->addWidget(runPageButton);
    auto* comparePagesButton = new QPushButton("Compare FIFO / LRU / Optimal");
    comparePagesButton->setObjectName("secondaryButton");
    comparePagesButton->setMinimumHeight(40);
    controlLayout->addWidget(comparePagesButton);
    connect(comparePagesButton, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        const auto refs = parseIntList(referenceStringEdit->text(), ok, false);
        if (!ok || refs.empty()) {
            QMessageBox::warning(this, "Invalid references", "Enter page numbers separated by commas or spaces.");
            return;
        }
        const int frames = frameCountSpinBox->value();
        FIFO fifo; LRU lru; Optimal optimal;
        const PageReplacementResult results[] = {
            fifo.replace(refs, frames), lru.replace(refs, frames), optimal.replace(refs, frames)};
        const QString names[] = {"FIFO", "LRU", "Optimal"};
        auto* dialog = new QDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowTitle("Page Replacement Comparison");
        dialog->resize(690, 350);
        auto* layout = new QVBoxLayout(dialog);
        auto* intro = new QLabel("All three algorithms use the same page references and frame count.\n"
            "Fewer faults are better. Optimal looks into the future, so it is a theoretical benchmark.");
        intro->setWordWrap(true);
        layout->addWidget(intro);
        auto* table = new QTableWidget(3, 4, dialog);
        table->setHorizontalHeaderLabels({"Algorithm", "Page faults", "Page hits", "Hit ratio"});
        for (int i = 0; i < 3; ++i) {
            const auto& r = results[i];
            const QString cells[] = {names[i], QString::number(r.faults),
                QString::number(r.hits), QString::number(r.hitRatio, 'f', 2)};
            for (int j = 0; j < 4; ++j) table->setItem(i, j, new QTableWidgetItem(cells[j]));
        }
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        layout->addWidget(table);
        auto* close = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(close, &QDialogButtonBox::rejected, dialog, &QDialog::close);
        layout->addWidget(close);
        dialog->show();
    });

    splitter->addWidget(inputBox);
    splitter->addWidget(controlBox);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({680, 420});
    main->addWidget(splitter);

    auto* timelineBox = new QGroupBox("Interactive timeline");
    auto* timelineLayout = new QVBoxLayout(timelineBox);
    timelineLayout->setContentsMargins(12, 18, 12, 12);
    auto* timelineScroll = new QScrollArea;
    timelineScroll->setWidgetResizable(false);
    timelineScroll->setFrameShape(QFrame::NoFrame);
    timelineScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frameWidget = new MemoryFrameWidget;
    timelineScroll->setWidget(frameWidget);
    timelineLayout->addWidget(timelineScroll);
    pagePlayback = new PlaybackBar;
    timelineLayout->addWidget(pagePlayback);
    main->addWidget(timelineBox);

    auto* statusRow = new QHBoxLayout;
    pageStepLabel = new QLabel("No simulation loaded");
    pageStepLabel->setObjectName("stepPill");
    pageStatusLabel = new QLabel("Set the reference string and frame count, then run the simulation.");
    pageStatusLabel->setWordWrap(true);
    statusRow->addWidget(pageStepLabel, 0);
    statusRow->addWidget(pageStatusLabel, 1);
    main->addLayout(statusRow);

    auto* metricsBox = new QGroupBox("Run metrics");
    auto* metricsLayout = new QHBoxLayout(metricsBox);
    metricsLayout->setContentsMargins(14, 18, 14, 14);
    pageMetricsLabel = new QLabel("Hits --   •   Faults --   •   Hit ratio --   •   Fault ratio --");
    pageMetricsLabel->setObjectName("metricsValue");
    metricsLayout->addWidget(pageMetricsLabel);
    main->addWidget(metricsBox);

    auto* logsBox = new QGroupBox("Decision log · revealed step by step");
    auto* logsLayout = new QVBoxLayout(logsBox);
    logsLayout->setContentsMargins(12, 18, 12, 12);
    pageLogs = new SimulationLogWidget;
    logsLayout->addWidget(pageLogs);
    main->addWidget(logsBox);

    auto* resultsBox = new QGroupBox("Reference results");
    auto* resultsLayout = new QVBoxLayout(resultsBox);
    resultsLayout->setContentsMargins(12, 18, 12, 12);
    pageResultsTable = new QTableWidget;
    pageResultsTable->setColumnCount(4);
    pageResultsTable->setHorizontalHeaderLabels({"Step", "Page", "Hit / Fault", "Frames after"});
    pageResultsTable->setMinimumHeight(210);
    pageResultsTable->verticalHeader()->setVisible(false);
    pageResultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    pageResultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsLayout->addWidget(pageResultsTable);
    main->addWidget(resultsBox);

    auto* saveRow = new QHBoxLayout;
    saveRow->addStretch();
    savePageButton = new QPushButton("Save Simulation");
    savePageButton->setObjectName("primaryButton");
    savePageButton->setMinimumHeight(40);
    savePageButton->setEnabled(false);
    saveRow->addWidget(savePageButton);
    main->addLayout(saveRow);

    content->setMinimumWidth(980);
    scroll->setWidget(content);

    connect(runPageButton, &QPushButton::clicked, this, &MemoryWindow::runPageReplacement);
    connect(pageAlgorithmCombo, &QComboBox::currentTextChanged, this, &MemoryWindow::onPageAlgorithmChanged);
    connect(pagePlayback, &PlaybackBar::indexChanged, this, &MemoryWindow::onPagePlaybackIndex);
    connect(savePageButton, &QPushButton::clicked, this, &MemoryWindow::savePageSimulation);

    return scroll;
}

void MemoryWindow::setupProcessTable()
{
    const QString defaults[4][2] = {
        {"P1", "212"},
        {"P2", "417"},
        {"P3", "112"},
        {"P4", "426"}
    };

    processTable->setRowCount(4);
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 2; ++col)
            processTable->setItem(row, col, new QTableWidgetItem(defaults[row][col]));
    processTable->setCurrentCell(0, 0);
}

// =========================================================
// Allocation tab
// =========================================================

void MemoryWindow::addProcessRow()
{
    const int row = processTable->rowCount();
    processTable->insertRow(row);
    processTable->setItem(row, 0, new QTableWidgetItem(QString("P%1").arg(row + 1)));
    processTable->setItem(row, 1, new QTableWidgetItem("100"));
    processTable->scrollToItem(processTable->item(row, 0));
    processTable->setCurrentCell(row, 0);
}

void MemoryWindow::removeSelectedProcessRows()
{
    const auto rows = processTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        allocationStatusLabel->setText("Select one or more process rows to delete.");
        return;
    }

    QList<int> indices;
    for (const auto& index : rows) indices << index.row();
    std::sort(indices.begin(), indices.end(), std::greater<int>());
    for (int row : indices) processTable->removeRow(row);
}

void MemoryWindow::clearProcessRows()
{
    processTable->setRowCount(0);
    allocationResult = AllocationResult();
    allocationResultLogs.clear();
    blockWidget->clear();
    allocationPlayback->setTotal(0);
    allocationLogs->clearLogs();
    allocationResultsTable->setRowCount(0);
    saveAllocationButton->setEnabled(false);
    allocationMetricsLabel->setText("Allocated --   •   Failed --   •   Free memory --   •   External fragmentation --");
    allocationStepLabel->setText("No simulation loaded");
    allocationStatusLabel->setText("Queue cleared. Add processes whenever you are ready.");
}

void MemoryWindow::onAllocationAlgorithmChanged(const QString&)
{
    updateAllocationAlgorithmInfo();
}

void MemoryWindow::updateAllocationAlgorithmInfo()
{
    const QString key = allocationAlgorithmCombo->currentText();
    if (key == "First Fit")
        allocationAlgorithmInfo->setText("Scans the blocks in order and places the process in the first block that is large enough.");
    else if (key == "Best Fit")
        allocationAlgorithmInfo->setText("Checks every block and places the process in the smallest block that is still large enough, to leave the least waste.");
    else
        allocationAlgorithmInfo->setText("Checks every block and places the process in the largest available block, to keep the leftover space usable.");
}

void MemoryWindow::runAllocation()
{
    std::vector<MemoryProcess> processes;
    processes.reserve(processTable->rowCount());
    QSet<int> seenIds;

    for (int row = 0; row < processTable->rowCount(); ++row) {
        auto textAt = [&](int column) { return processTable->item(row, column) ? processTable->item(row, column)->text().trimmed() : QString(); };
        QString pidText = textAt(0);
        if (pidText.startsWith('P', Qt::CaseInsensitive)) pidText.remove(0, 1);

        bool okId = false, okSize = false;
        const int id = pidText.toInt(&okId);
        const int size = textAt(1).toInt(&okSize);

        if (!okId || !okSize || id <= 0 || size <= 0) {
            QMessageBox::warning(this, "Invalid process", "Every row needs a positive PID and a positive size.");
            return;
        }
        if (seenIds.contains(id)) {
            QMessageBox::warning(this, "Duplicate process", QString("PID %1 is listed more than once.").arg(id));
            return;
        }
        seenIds.insert(id);
        processes.push_back({id, size});
    }

    if (processes.empty()) {
        QMessageBox::warning(this, "No processes", "Add at least one process before running the allocation.");
        return;
    }

    bool blocksOk = false;
    const std::vector<int> blocks = parseIntList(blockSizesEdit->text(), blocksOk, true);
    if (!blocksOk || blocks.empty()) {
        QMessageBox::warning(this, "Invalid blocks", "Enter one or more positive block sizes, for example: 100, 500, 200.");
        return;
    }

    std::unique_ptr<MemoryAllocator> allocator;
    const QString algorithm = allocationAlgorithmCombo->currentText();
    if (algorithm == "First Fit") allocator = std::make_unique<FirstFit>();
    else if (algorithm == "Best Fit") allocator = std::make_unique<BestFit>();
    else allocator = std::make_unique<WorstFit>();

    allocationResult = allocator->allocate(blocks, processes);
    blockWidget->setResult(allocationResult);
    allocationPlayback->setTotal(static_cast<int>(allocationResult.steps.size()));
    allocationResultLogs = buildAllocationLogs();
    allocationLogs->setLogs(allocationResultLogs);
    updateAllocationMetrics();
    populateAllocationResultTable();
    updateAllocationPlaybackState(0);
    saveAllocationButton->setEnabled(true);
    allocationPlayback->startPlayback();
}

QStringList MemoryWindow::buildAllocationLogs() const
{
    QStringList output;
    const QString algorithm = allocationAlgorithmCombo->currentText();

    for (const auto& step : allocationResult.steps) {
        QString reason;
        if (step.blockIndex == -1) {
            reason = QString("No free block is large enough for P%1 (size %2). Allocation failed for this process.")
                         .arg(step.pid).arg(step.size);
        } else if (algorithm == "First Fit") {
            reason = QString("First Fit scans the blocks in order and P%1 (size %2) is placed in the first block that fits: Block %3.")
                         .arg(step.pid).arg(step.size).arg(step.blockIndex);
        } else if (algorithm == "Best Fit") {
            reason = QString("Best Fit compares every block that fits and chooses the smallest one, Block %1, for P%2 (size %3).")
                         .arg(step.blockIndex).arg(step.pid).arg(step.size);
        } else {
            reason = QString("Worst Fit compares every block that fits and chooses the largest one, Block %1, for P%2 (size %3).")
                         .arg(step.blockIndex).arg(step.pid).arg(step.size);
        }
        output << reason;
    }

    return output;
}

void MemoryWindow::updateAllocationMetrics()
{
    allocationMetricsLabel->setText(
        QString("Allocated %1   •   Failed %2   •   Free memory %3   •   External fragmentation %4")
            .arg(allocationResult.allocatedCount)
            .arg(allocationResult.failedCount)
            .arg(allocationResult.totalFreeMemory)
            .arg(allocationResult.externalFragmentation));
}

void MemoryWindow::populateAllocationResultTable()
{
    allocationResultsTable->setRowCount(static_cast<int>(allocationResult.steps.size()));
    for (int row = 0; row < allocationResultsTable->rowCount(); ++row) {
        const auto& step = allocationResult.steps[row];
        QStringList freeList;
        for (int size : step.blocksAfter) freeList << QString::number(size);

        allocationResultsTable->setItem(row, 0, new QTableWidgetItem(QString("P%1").arg(step.pid)));
        allocationResultsTable->setItem(row, 1, new QTableWidgetItem(QString::number(step.size)));
        allocationResultsTable->setItem(row, 2, new QTableWidgetItem(step.blockIndex == -1 ? "none" : QString::number(step.blockIndex)));
        allocationResultsTable->setItem(row, 3, new QTableWidgetItem(freeList.join(", ")));
    }
}

void MemoryWindow::saveAllocationSimulation()
{
    if (allocationResult.steps.empty()) {
        QMessageBox::information(this, "Nothing to save", "Run a memory allocation simulation first.");
        return;
    }
    const QString algorithm = QString::fromStdString(allocationResult.algorithm);
    QJsonObject root;
    root["application"] = "OSimX";
    root["module"] = "Memory Management";
    root["mode"] = "Allocation";
    root["algorithm"] = algorithm;
    root["savedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    QJsonArray blocks;
    for (int size : allocationResult.initialBlocks) blocks.append(size);
    root["initialBlocks"] = blocks;
    QJsonArray savedProcesses;
    for (const auto& step : allocationResult.steps) {
        QJsonObject entry;
        entry["pid"] = step.pid;
        entry["size"] = step.size;
        savedProcesses.append(entry);
    }
    root["processes"] = savedProcesses;
    QJsonArray steps;
    for (const auto& step : allocationResult.steps) {
        QJsonObject item;
        item["pid"] = step.pid;
        item["size"] = step.size;
        item["blockIndex"] = step.blockIndex;
        QJsonArray after;
        for (int size : step.blocksAfter) after.append(size);
        item["blocksAfter"] = after;
        steps.append(item);
    }
    root["steps"] = steps;
    QJsonObject metrics;
    metrics["allocatedCount"] = allocationResult.allocatedCount;
    metrics["failedCount"] = allocationResult.failedCount;
    metrics["totalFreeMemory"] = allocationResult.totalFreeMemory;
    metrics["largestFreeBlock"] = allocationResult.largestFreeBlock;
    metrics["externalFragmentation"] = allocationResult.externalFragmentation;
    root["metrics"] = metrics;
    QJsonArray log;
    for (const auto& entry : allocationResultLogs) log.append(entry);
    root["decisionLog"] = log;
    QString error;
    if (!SimulationRepository::instance().save("Memory Allocation", algorithm,
            QString("allocated %1 · failed %2").arg(allocationResult.allocatedCount).arg(allocationResult.failedCount), root, error)) {
        QMessageBox::warning(this, "Save failed", error);
        return;
    }
    allocationStatusLabel->setText("Simulation saved to your profile.");
    emit simulationSaved();
    QMessageBox::information(this, "Simulation saved", "This run is saved in your OSimX profile on the home screen.");
}


void MemoryWindow::onAllocationPlaybackIndex(int index)
{
    updateAllocationPlaybackState(index);
}

void MemoryWindow::updateAllocationPlaybackState(int index)
{
    if (allocationResult.steps.empty()) {
        allocationStepLabel->setText("No simulation loaded");
        allocationStatusLabel->setText("Run the allocation to begin the guided walkthrough.");
        return;
    }

    index = qBound(0, index, static_cast<int>(allocationResult.steps.size()) - 1);
    blockWidget->setCurrentStep(index);
    allocationLogs->revealUpTo(index);
    allocationStepLabel->setText(QString("Step %1 / %2").arg(index + 1).arg(allocationResult.steps.size()));
    allocationStatusLabel->setText(allocationResultLogs.value(index, "Allocating..."));
}

// =========================================================
// Page replacement tab
// =========================================================

void MemoryWindow::onPageAlgorithmChanged(const QString&)
{
    updatePageAlgorithmInfo();
}

void MemoryWindow::updatePageAlgorithmInfo()
{
    const QString key = pageAlgorithmCombo->currentText();
    if (key == "FIFO")
        pageAlgorithmInfo->setText("Replaces whichever page has been sitting in a frame the longest, regardless of how often it was used since.");
    else if (key == "LRU")
        pageAlgorithmInfo->setText("Replaces the page that has gone the longest without being referenced again.");
    else
        pageAlgorithmInfo->setText("Belady's optimal algorithm: replaces the page that will not be needed for the longest time in the future. It needs the full reference string in advance, so it is used as a best-case comparison, not a real policy.");
}

void MemoryWindow::runPageReplacement()
{
    bool refsOk = false;
    const std::vector<int> refs = parseIntList(referenceStringEdit->text(), refsOk, false);
    if (!refsOk || refs.empty()) {
        QMessageBox::warning(this, "Invalid reference string", "Enter page numbers separated by commas or spaces, for example: 7, 0, 1, 2.");
        return;
    }

    const int frameCount = frameCountSpinBox->value();

    std::unique_ptr<PageReplacer> replacer;
    const QString algorithm = pageAlgorithmCombo->currentText();
    if (algorithm == "FIFO") replacer = std::make_unique<FIFO>();
    else if (algorithm == "LRU") replacer = std::make_unique<LRU>();
    else replacer = std::make_unique<Optimal>();

    pageResult = replacer->replace(refs, frameCount);
    frameWidget->setResult(pageResult);
    pagePlayback->setTotal(static_cast<int>(pageResult.steps.size()));
    pageResultLogs = buildPageLogs();
    pageLogs->setLogs(pageResultLogs);
    updatePageMetrics();
    populatePageResultTable();
    updatePagePlaybackState(0);
    savePageButton->setEnabled(true);
    pagePlayback->startPlayback();
}

QStringList MemoryWindow::buildPageLogs() const
{
    QStringList output;
    const QString algorithm = pageAlgorithmCombo->currentText();

    for (const auto& step : pageResult.steps) {
        QString reason;
        if (step.hit) {
            reason = QString("Page %1 is already in a frame. No fault, no eviction needed.").arg(step.page);
        } else if (step.victim == -1) {
            reason = QString("Page %1 is loaded into a free frame. No eviction needed yet.").arg(step.page);
        } else if (algorithm == "FIFO") {
            reason = QString("Page %1 causes a fault. FIFO evicts page %2, the page that has been in memory the longest.")
                         .arg(step.page).arg(step.victim);
        } else if (algorithm == "LRU") {
            reason = QString("Page %1 causes a fault. LRU evicts page %2, the page used least recently.")
                         .arg(step.page).arg(step.victim);
        } else {
            reason = QString("Page %1 causes a fault. Optimal evicts page %2, since it will not be needed again for the longest time.")
                         .arg(step.page).arg(step.victim);
        }
        output << reason;
    }

    return output;
}

void MemoryWindow::updatePageMetrics()
{
    pageMetricsLabel->setText(
        QString("Hits %1   •   Faults %2   •   Hit ratio %3   •   Fault ratio %4")
            .arg(pageResult.hits)
            .arg(pageResult.faults)
            .arg(niceNumber(pageResult.hitRatio))
            .arg(niceNumber(pageResult.faultRatio)));
}

void MemoryWindow::populatePageResultTable()
{
    pageResultsTable->setRowCount(static_cast<int>(pageResult.steps.size()));
    for (int row = 0; row < pageResultsTable->rowCount(); ++row) {
        const auto& step = pageResult.steps[row];
        QStringList framesList;
        for (int page : step.frames) framesList << (page == -1 ? "—" : QString::number(page));

        pageResultsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        pageResultsTable->setItem(row, 1, new QTableWidgetItem(QString::number(step.page)));
        pageResultsTable->setItem(row, 2, new QTableWidgetItem(step.hit ? "Hit" : "Fault"));
        pageResultsTable->setItem(row, 3, new QTableWidgetItem(framesList.join(", ")));
    }
}

void MemoryWindow::savePageSimulation()
{
    if (pageResult.steps.empty()) {
        QMessageBox::information(this, "Nothing to save", "Run a page replacement simulation first.");
        return;
    }
    const QString algorithm = QString::fromStdString(pageResult.algorithm);
    QJsonObject root;
    root["application"] = "OSimX";
    root["module"] = "Memory Management";
    root["mode"] = "Page Replacement";
    root["algorithm"] = algorithm;
    root["frameCount"] = pageResult.frameCount;
    QJsonArray references;
    for (const auto& step : pageResult.steps)
        references.append(step.page);
    root["referenceString"] = references;
    root["savedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    QJsonArray steps;
    for (const auto& step : pageResult.steps) {
        QJsonObject item;
        item["page"] = step.page;
        item["hit"] = step.hit;
        item["victim"] = step.victim;
        QJsonArray frames;
        for (int page : step.frames) frames.append(page);
        item["frames"] = frames;
        steps.append(item);
    }
    root["steps"] = steps;
    QJsonObject metrics;
    metrics["hits"] = pageResult.hits;
    metrics["faults"] = pageResult.faults;
    metrics["hitRatio"] = pageResult.hitRatio;
    metrics["faultRatio"] = pageResult.faultRatio;
    root["metrics"] = metrics;
    QJsonArray log;
    for (const auto& entry : pageResultLogs) log.append(entry);
    root["decisionLog"] = log;
    QString error;
    if (!SimulationRepository::instance().save("Page Replacement", algorithm,
            QString("hits %1 · faults %2").arg(pageResult.hits).arg(pageResult.faults), root, error)) {
        QMessageBox::warning(this, "Save failed", error);
        return;
    }
    pageStatusLabel->setText("Simulation saved to your profile.");
    emit simulationSaved();
    QMessageBox::information(this, "Simulation saved", "This run is saved in your OSimX profile on the home screen.");
}


void MemoryWindow::onPagePlaybackIndex(int index)
{
    updatePagePlaybackState(index);
}

void MemoryWindow::updatePagePlaybackState(int index)
{
    if (pageResult.steps.empty()) {
        pageStepLabel->setText("No simulation loaded");
        pageStatusLabel->setText("Run the simulation to begin the guided walkthrough.");
        return;
    }

    index = qBound(0, index, static_cast<int>(pageResult.steps.size()) - 1);
    frameWidget->setCurrentStep(index);
    pageLogs->revealUpTo(index);
    pageStepLabel->setText(QString("Step %1 / %2").arg(index + 1).arg(pageResult.steps.size()));
    pageStatusLabel->setText(pageResultLogs.value(index, "Replacing..."));
}


bool MemoryWindow::restoreSimulation(const QJsonObject& saved)
{
    const QString mode = saved.value("mode").toString();
    const QString algorithm = saved.value("algorithm").toString();
    const QJsonObject metrics = saved.value("metrics").toObject();
    if (!saved.value("steps").isArray() || !saved.value("metrics").isObject())
        return false;
    const QJsonArray steps = saved.value("steps").toArray();
    if (steps.isEmpty() || steps.size() > 100000) return false;

    auto readIntegers = [](const QJsonValue& value, std::vector<int>& values) {
        if (!value.isArray()) return false;
        for (const auto& entry : value.toArray()) {
            if (!entry.isDouble()) return false;
            values.push_back(entry.toInt());
        }
        return true;
    };
    QStringList savedLogs;
    if (saved.value("decisionLog").isArray()) {
        for (const auto& value : saved.value("decisionLog").toArray()) {
            if (!value.isString()) return false;
            savedLogs << value.toString();
        }
    }

    if (mode == "Allocation") {
        const int index = allocationAlgorithmCombo->findText(algorithm);
        if (index < 0) return false;
        AllocationResult restored;
        restored.algorithm = algorithm.toStdString();
        if (!readIntegers(saved.value("initialBlocks"), restored.initialBlocks) ||
            restored.initialBlocks.empty()) return false;
        for (int block : restored.initialBlocks)
            if (block <= 0) return false;
        for (const auto& value : steps) {
            if (!value.isObject()) return false;
            const QJsonObject item = value.toObject();
            for (const char* key : {"pid", "size", "blockIndex"})
                if (!item.value(QLatin1String(key)).isDouble()) return false;
            AllocationStep step;
            step.pid = item.value("pid").toInt();
            step.size = item.value("size").toInt();
            step.blockIndex = item.value("blockIndex").toInt();
            if (step.pid <= 0 || step.size <= 0 ||
                step.blockIndex < -1 ||
                step.blockIndex >= static_cast<int>(restored.initialBlocks.size()) ||
                !readIntegers(item.value("blocksAfter"), step.blocksAfter) ||
                step.blocksAfter.size() != restored.initialBlocks.size()) return false;
            restored.steps.push_back(std::move(step));
        }
        for (const char* key : {"allocatedCount", "failedCount", "totalFreeMemory", "largestFreeBlock", "externalFragmentation"})
            if (!metrics.value(QLatin1String(key)).isDouble()) return false;
        restored.allocatedCount = metrics.value("allocatedCount").toInt();
        restored.failedCount = metrics.value("failedCount").toInt();
        restored.totalFreeMemory = metrics.value("totalFreeMemory").toInt();
        restored.largestFreeBlock = metrics.value("largestFreeBlock").toInt();
        restored.externalFragmentation = metrics.value("externalFragmentation").toInt();

        allocationAlgorithmCombo->setCurrentIndex(index);
        QStringList blockTexts;
        for (int block : restored.initialBlocks)
            blockTexts << QString::number(block);
        blockSizesEdit->setText(blockTexts.join(", "));
        processTable->setRowCount(static_cast<int>(restored.steps.size()));
        for (int i = 0; i < processTable->rowCount(); ++i) {
            const auto& step = restored.steps[static_cast<std::size_t>(i)];
            processTable->setItem(i, 0, new QTableWidgetItem(QString("P%1").arg(step.pid)));
            processTable->setItem(i, 1, new QTableWidgetItem(QString::number(step.size)));
        }
        allocationResult = std::move(restored);
        allocationResultLogs = savedLogs.isEmpty() ? buildAllocationLogs() : savedLogs;
        blockWidget->setResult(allocationResult);
        allocationPlayback->setTotal(static_cast<int>(allocationResult.steps.size()));
        allocationLogs->setLogs(allocationResultLogs);
        updateAllocationMetrics();
        populateAllocationResultTable();
        updateAllocationPlaybackState(0);
        saveAllocationButton->setEnabled(true);
        tabs->setCurrentIndex(0);
        return true;
    }

    if (mode == "Page Replacement") {
        const int index = pageAlgorithmCombo->findText(algorithm);
        const int frameCount = saved.value("frameCount").toInt(-1);
        if (index < 0 || frameCount < frameCountSpinBox->minimum() ||
            frameCount > frameCountSpinBox->maximum()) return false;
        PageReplacementResult restored;
        restored.algorithm = algorithm.toStdString();
        restored.frameCount = frameCount;
        QStringList references;
        for (const auto& value : steps) {
            if (!value.isObject()) return false;
            const QJsonObject item = value.toObject();
            if (!item.value("page").isDouble() || !item.value("hit").isBool() ||
                !item.value("victim").isDouble()) return false;
            PageStep step;
            step.page = item.value("page").toInt();
            step.hit = item.value("hit").toBool();
            step.victim = item.value("victim").toInt();
            if (step.page < 0 || !readIntegers(item.value("frames"), step.frames) ||
                static_cast<int>(step.frames.size()) != frameCount) return false;
            restored.steps.push_back(std::move(step));
            references << QString::number(restored.steps.back().page);
        }
        for (const char* key : {"hits", "faults", "hitRatio", "faultRatio"})
            if (!metrics.value(QLatin1String(key)).isDouble()) return false;
        restored.hits = metrics.value("hits").toInt();
        restored.faults = metrics.value("faults").toInt();
        restored.hitRatio = metrics.value("hitRatio").toDouble();
        restored.faultRatio = metrics.value("faultRatio").toDouble();

        pageAlgorithmCombo->setCurrentIndex(index);
        frameCountSpinBox->setValue(frameCount);
        referenceStringEdit->setText(references.join(", "));
        pageResult = std::move(restored);
        pageResultLogs = savedLogs.isEmpty() ? buildPageLogs() : savedLogs;
        frameWidget->setResult(pageResult);
        pagePlayback->setTotal(static_cast<int>(pageResult.steps.size()));
        pageLogs->setLogs(pageResultLogs);
        updatePageMetrics();
        populatePageResultTable();
        updatePagePlaybackState(0);
        savePageButton->setEnabled(true);
        tabs->setCurrentIndex(1);
        return true;
    }
    return false;
}
