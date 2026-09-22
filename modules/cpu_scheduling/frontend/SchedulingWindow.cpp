#include "SchedulingWindow.h"
#include "modules/cpu_scheduling/backend/FCFS.h"
#include "modules/cpu_scheduling/backend/SJF.h"
#include "modules/cpu_scheduling/backend/SRTF.h"
#include "modules/cpu_scheduling/backend/RoundRobin.h"
#include "modules/cpu_scheduling/backend/PriorityScheduling.h"
#include <QAbstractItemView>
#include <QFrame>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QScrollBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <memory>
#include <functional>
#include <QStringList>
#include <algorithm>

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
}

SchedulingWindow::SchedulingWindow(QWidget* parent) : QWidget(parent)
{
    setupUI();
    setupProcessTable();
    updateAlgorithmInfo();
}

void SchedulingWindow::setupUI()
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
    auto* title = new QLabel("CPU Scheduling");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Build a process queue, run an algorithm, then walk through every scheduling decision.");
    subtitle->setObjectName("pageSubtitle");
    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);
    headerLayout->addWidget(backButton, 0, Qt::AlignTop);
    headerLayout->addSpacing(12);
    headerLayout->addLayout(titleBlock);
    headerLayout->addStretch();
    outer->addWidget(header);
    connect(backButton, &QPushButton::clicked, this, &SchedulingWindow::backRequested);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto* content = new QWidget;
    auto* main = new QVBoxLayout(content);
    main->setContentsMargins(22, 8, 22, 28);
    main->setSpacing(14);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->setMinimumHeight(290);

    auto* inputBox = new QGroupBox("Process queue");
    auto* inputLayout = new QVBoxLayout(inputBox);
    inputLayout->setContentsMargins(14, 18, 14, 14);
    processTable = new QTableWidget;
    processTable->setColumnCount(4);
    processTable->setHorizontalHeaderLabels({"PID", "Arrival", "Burst", "Priority"});
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
    addButton = new QPushButton("＋ Add Process");
    removeButton = new QPushButton("− Delete Selected");
    clearButton = new QPushButton("Clear All");
    addButton->setObjectName("secondaryButton");
    removeButton->setObjectName("secondaryButton");
    clearButton->setObjectName("dangerButton");
    rowButtons->addWidget(addButton);
    rowButtons->addWidget(removeButton);
    rowButtons->addWidget(clearButton);
    rowButtons->addStretch();
    inputLayout->addLayout(rowButtons);

    auto* controlBox = new QGroupBox("Algorithm");
    auto* controlLayout = new QVBoxLayout(controlBox);
    controlLayout->setContentsMargins(14, 18, 14, 14);
    algorithmCombo = new QComboBox;
    algorithmCombo->addItems({"FCFS", "SJF", "SRTF", "Round Robin", "Priority"});
    controlLayout->addWidget(makeCaption("Choose a scheduling policy"));
    controlLayout->addWidget(algorithmCombo);
    auto* quantumRow = new QHBoxLayout;
    quantumRow->addWidget(makeCaption("Time quantum"));
    quantumSpinBox = new QSpinBox;
    quantumSpinBox->setRange(1, 100);
    quantumSpinBox->setValue(2);
    quantumRow->addWidget(quantumSpinBox);
    controlLayout->addLayout(quantumRow);

    algorithmInfo = new QLabel;
    algorithmInfo->setWordWrap(true);
    algorithmInfo->setObjectName("infoLabel");
    controlLayout->addWidget(algorithmInfo);
    controlLayout->addStretch();

    runButton = new QPushButton("▶  Run Simulation");
    runButton->setObjectName("primaryButton");
    runButton->setMinimumHeight(44);
    controlLayout->addWidget(runButton);

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
    ganttWidget = new GanttWidget;
    timelineScroll->setWidget(ganttWidget);
    timelineLayout->addWidget(timelineScroll);
    playback = new PlaybackBar;
    timelineLayout->addWidget(playback);
    main->addWidget(timelineBox);

    auto* statusRow = new QHBoxLayout;
    currentStepLabel = new QLabel("No simulation loaded");
    currentStepLabel->setObjectName("stepPill");
    statusLabel = new QLabel("Choose an algorithm, then run the simulation.");
    statusLabel->setWordWrap(true);
    statusRow->addWidget(currentStepLabel, 0);
    statusRow->addWidget(statusLabel, 1);
    main->addLayout(statusRow);

    auto* metricsBox = new QGroupBox("Run metrics");
    auto* metricsLayout = new QHBoxLayout(metricsBox);
    metricsLayout->setContentsMargins(14, 18, 14, 14);
    metricsLabel = new QLabel("Waiting --   •   Turnaround --   •   Response --   •   CPU --   •   Throughput --");
    metricsLabel->setObjectName("metricsValue");
    metricsLayout->addWidget(metricsLabel);
    main->addWidget(metricsBox);

    auto* logsBox = new QGroupBox("Decision log · revealed step by step");
    auto* logsLayout = new QVBoxLayout(logsBox);
    logsLayout->setContentsMargins(12, 18, 12, 12);
    logs = new SimulationLogWidget;
    logsLayout->addWidget(logs);
    main->addWidget(logsBox);

    auto* resultsBox = new QGroupBox("Process results");
    auto* resultsLayout = new QVBoxLayout(resultsBox);
    resultsLayout->setContentsMargins(12, 18, 12, 12);
    resultsTable = new QTableWidget;
    resultsTable->setColumnCount(7);
    resultsTable->setHorizontalHeaderLabels({"PID", "Arrival", "Burst", "Priority", "Start", "Completion", "Waiting / TAT / Response"});
    resultsTable->setMinimumHeight(210);
    resultsTable->verticalHeader()->setVisible(false);
    resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsLayout->addWidget(resultsTable);
    main->addWidget(resultsBox);

    auto* saveRow = new QHBoxLayout;
    saveRow->addStretch();
    saveButton = new QPushButton("Save Simulation");
    saveButton->setObjectName("primaryButton");
    saveButton->setMinimumHeight(40);
    saveButton->setEnabled(false);
    saveRow->addWidget(saveButton);
    main->addLayout(saveRow);

    content->setMinimumWidth(980);
    scroll->setWidget(content);
    outer->addWidget(scroll, 1);

    connect(addButton, &QPushButton::clicked, this, &SchedulingWindow::addProcess);
    connect(removeButton, &QPushButton::clicked, this, &SchedulingWindow::removeSelectedProcesses);
    connect(clearButton, &QPushButton::clicked, this, &SchedulingWindow::clearProcesses);
    connect(runButton, &QPushButton::clicked, this, &SchedulingWindow::runSimulation);
    connect(algorithmCombo, &QComboBox::currentTextChanged, this, &SchedulingWindow::onAlgorithmChanged);
    connect(playback, &PlaybackBar::indexChanged, this, &SchedulingWindow::onPlaybackIndex);
    connect(saveButton, &QPushButton::clicked, this, &SchedulingWindow::saveSimulation);
}

void SchedulingWindow::setupProcessTable()
{
    const QString defaults[5][4] = {
        {"P1", "0", "5", "2"},
        {"P2", "1", "3", "1"},
        {"P3", "2", "8", "3"},
        {"P4", "3", "2", "2"},
        {"P5", "5", "4", "1"}
    };

    processTable->setRowCount(5);
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 4; ++col)
            processTable->setItem(row, col, new QTableWidgetItem(defaults[row][col]));
    processTable->setCurrentCell(0, 0);
}

void SchedulingWindow::addProcess()
{
    const int row = processTable->rowCount();
    processTable->insertRow(row);
    processTable->setItem(row, 0, new QTableWidgetItem(QString("P%1").arg(row + 1)));
    processTable->setItem(row, 1, new QTableWidgetItem("0"));
    processTable->setItem(row, 2, new QTableWidgetItem("1"));
    processTable->setItem(row, 3, new QTableWidgetItem("1"));
    processTable->scrollToItem(processTable->item(row, 0));
    processTable->setCurrentCell(row, 0);
}

void SchedulingWindow::removeSelectedProcesses()
{
    const auto rows = processTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        statusLabel->setText("Select one or more process rows to delete.");
        return;
    }

    QList<int> indices;
    for (const auto& index : rows) indices << index.row();
    std::sort(indices.begin(), indices.end(), std::greater<int>());
    for (int row : indices) processTable->removeRow(row);
}

void SchedulingWindow::clearProcesses()
{
    processTable->setRowCount(0);
    result = SchedulingResult();
    resultLogs.clear();
    ganttWidget->clear();
    playback->setTotal(0);
    logs->clearLogs();
    resultsTable->setRowCount(0);
    saveButton->setEnabled(false);
    metricsLabel->setText("Waiting --   •   Turnaround --   •   Response --   •   CPU --   •   Throughput --");
    currentStepLabel->setText("No simulation loaded");
    statusLabel->setText("Queue cleared. Add processes whenever you are ready.");
}

void SchedulingWindow::onAlgorithmChanged(const QString&)
{
    quantumSpinBox->setEnabled(algorithmCombo->currentText() == "Round Robin");
    updateAlgorithmInfo();
}

void SchedulingWindow::updateAlgorithmInfo()
{
    const QString key = algorithmCombo->currentText();
    if (key == "FCFS")
        algorithmInfo->setText("Earliest arrival is served first. Non-preemptive: once a process starts, it runs to completion.");
    else if (key == "SJF")
        algorithmInfo->setText("Among the processes already waiting, the shortest burst is selected. Non-preemptive.");
    else if (key == "SRTF")
        algorithmInfo->setText("The process with the smallest remaining time gets the CPU. A newly arrived shorter job can preempt the current one.");
    else if (key == "Round Robin")
        algorithmInfo->setText("Ready processes take turns. Each process receives at most the selected time quantum before returning to the queue if work remains.");
    else
        algorithmInfo->setText("The ready process with the lowest numeric priority value runs first. This implementation is preemptive.");
}

QString SchedulingWindow::algorithmKey() const
{
    return algorithmCombo->currentText();
}

void SchedulingWindow::runSimulation()
{
    std::vector<Process> processes;
    processes.reserve(processTable->rowCount());

    for (int row = 0; row < processTable->rowCount(); ++row) {
        auto textAt = [&](int column) { return processTable->item(row, column) ? processTable->item(row, column)->text().trimmed() : QString(); };
        QString pidText = textAt(0);
        if (pidText.startsWith('P', Qt::CaseInsensitive)) pidText.remove(0, 1);

        bool okId = false, okArrival = false, okBurst = false, okPriority = false;
        const int id = pidText.toInt(&okId);
        const int arrival = textAt(1).toInt(&okArrival);
        const int burst = textAt(2).toInt(&okBurst);
        const int priority = textAt(3).toInt(&okPriority);

        if (!okId || !okArrival || !okBurst || !okPriority || id <= 0 || arrival < 0 || burst <= 0 || priority < 0) {
            QMessageBox::warning(this, "Invalid process", "Every row needs a positive PID, a non-negative arrival time, a positive burst time, and a non-negative priority.");
            return;
        }
        processes.emplace_back(id, arrival, burst, priority);
    }

    if (processes.empty()) {
        QMessageBox::warning(this, "No processes", "Add at least one process before running the simulation.");
        return;
    }

    std::unique_ptr<CPUScheduler> scheduler;
    const QString algorithm = algorithmKey();
    if (algorithm == "FCFS") scheduler = std::make_unique<FCFS>();
    else if (algorithm == "SJF") scheduler = std::make_unique<SJF>();
    else if (algorithm == "SRTF") scheduler = std::make_unique<SRTF>();
    else if (algorithm == "Round Robin") scheduler = std::make_unique<RoundRobin>(quantumSpinBox->value());
    else scheduler = std::make_unique<PriorityScheduling>();

    result = scheduler->schedule(processes);
    ganttWidget->setGanttData(result.ganttChart);
    playback->setTotal(static_cast<int>(result.ganttChart.size()));
    resultLogs = buildLogs();
    logs->setLogs(resultLogs);
    updateMetrics();
    populateResultTable();
    updatePlaybackState(0);
    saveButton->setEnabled(true);
    playback->startPlayback();
}

QStringList SchedulingWindow::buildLogs() const
{
    QStringList output;
    const QString algorithm = algorithmKey();
    const int quantum = quantumSpinBox->value();

    auto processById = [&](int id) -> const Process* {
        for (const auto& process : result.processes)
            if (process.id == id)
                return &process;
        return nullptr;
    };

    auto remainingAt = [&](const Process& process, int time) {
        int used = 0;
        for (const auto& segment : result.ganttChart) {
            if (segment.pid == process.id && segment.startTime < time)
                used += std::max(0, std::min(segment.endTime, time) - segment.startTime);
        }
        return std::max(0, process.burstTime - used);
    };

    auto readyProcesses = [&](int time) {
        std::vector<const Process*> ready;
        for (const auto& process : result.processes) {
            if (process.arrivalTime <= time && remainingAt(process, time) > 0)
                ready.push_back(&process);
        }

        if (algorithm == "FCFS") {
            std::sort(ready.begin(), ready.end(), [](const Process* a, const Process* b) {
                if (a->arrivalTime != b->arrivalTime) return a->arrivalTime < b->arrivalTime;
                return a->id < b->id;
            });
        } else if (algorithm == "SJF") {
            std::sort(ready.begin(), ready.end(), [](const Process* a, const Process* b) {
                if (a->burstTime != b->burstTime) return a->burstTime < b->burstTime;
                if (a->arrivalTime != b->arrivalTime) return a->arrivalTime < b->arrivalTime;
                return a->id < b->id;
            });
        } else if (algorithm == "SRTF") {
            std::sort(ready.begin(), ready.end(), [&](const Process* a, const Process* b) {
                const int ar = remainingAt(*a, time);
                const int br = remainingAt(*b, time);
                if (ar != br) return ar < br;
                return a->id < b->id;
            });
        } else if (algorithm == "Priority") {
            std::sort(ready.begin(), ready.end(), [](const Process* a, const Process* b) {
                if (a->priority != b->priority) return a->priority < b->priority;
                if (a->arrivalTime != b->arrivalTime) return a->arrivalTime < b->arrivalTime;
                return a->id < b->id;
            });
        }
        return ready;
    };

    for (int i = 0; i < static_cast<int>(result.ganttChart.size()); ++i) {
        const auto& segment = result.ganttChart[i];
        const QString window = QString("t=%1 → %2").arg(segment.startTime).arg(segment.endTime);

        if (segment.pid < 0) {
            output << QString("%1\nCPU is idle because no process has arrived yet. Time advances to the next arrival.")
                              .arg(window);
            continue;
        }

        const Process* process = processById(segment.pid);
        const auto ready = readyProcesses(segment.startTime);
        QStringList readyNames;
        for (const auto* candidate : ready)
            readyNames << QString("P%1").arg(candidate->id);

        const int remainingBefore = process ? remainingAt(*process, segment.startTime) : 0;
        QString reason;

        if (algorithm == "FCFS") {
            reason = QString("FCFS chooses the ready process with the earliest arrival time. P%1 arrived at t=%2, earlier than the other ready processes.")
                         .arg(segment.pid)
                         .arg(process ? process->arrivalTime : segment.startTime);
        } else if (algorithm == "SJF") {
            reason = QString("SJF compares the burst times of the ready processes and chooses the shortest job. P%1 has burst %2.")
                         .arg(segment.pid)
                         .arg(process ? process->burstTime : 0);
        } else if (algorithm == "SRTF") {
            reason = QString("SRTF compares remaining execution time. P%1 has %2 unit(s) remaining, which is the smallest among the ready processes.")
                         .arg(segment.pid)
                         .arg(remainingBefore);
        } else if (algorithm == "Round Robin") {
            const bool completedHere = process && process->completionTime == segment.endTime;
            reason = completedHere
                ? QString("Round Robin gives P%1 up to %2 unit(s). It needs %3 unit(s) here, so it finishes its CPU burst.")
                      .arg(segment.pid).arg(quantum).arg(segment.endTime - segment.startTime)
                : QString("Round Robin gives P%1 up to its %2-unit quantum. Because work remains, P%1 returns to the ready queue after this turn.")
                      .arg(segment.pid).arg(quantum);
        } else {
            reason = QString("Priority scheduling compares numeric priority values. P%1 has priority %2, where a lower number means higher priority.")
                         .arg(segment.pid)
                         .arg(process ? process->priority : 0);
        }

        QString log = QString("%1\nReady: %2\nSelected: P%3\nWhy: %4")
                          .arg(window)
                          .arg(readyNames.isEmpty() ? "none" : readyNames.join(", "))
                          .arg(segment.pid)
                          .arg(reason);

        if (process) {
            log += QString("\nEffect: P%1 runs for %2 unit(s)%3.")
                       .arg(segment.pid)
                       .arg(segment.endTime - segment.startTime)
                       .arg(process->completionTime == segment.endTime ? " and completes." : ".");
        }

        output << log;
    }

    return output;
}

void SchedulingWindow::updateMetrics()
{
    metricsLabel->setText(QString("Waiting %1   •   Turnaround %2   •   Response %3   •   CPU %4%   •   Throughput %5 processes/unit")
                              .arg(niceNumber(result.avgWaitingTime))
                              .arg(niceNumber(result.avgTurnaroundTime))
                              .arg(niceNumber(result.avgResponseTime))
                              .arg(niceNumber(result.cpuUtilization, 1))
                              .arg(niceNumber(result.throughput, 3)));
}

void SchedulingWindow::populateResultTable()
{
    resultsTable->setRowCount(static_cast<int>(result.processes.size()));
    for (int row = 0; row < resultsTable->rowCount(); ++row) {
        const auto& p = result.processes[row];
        resultsTable->setItem(row, 0, new QTableWidgetItem(QString("P%1").arg(p.id)));
        resultsTable->setItem(row, 1, new QTableWidgetItem(QString::number(p.arrivalTime)));
        resultsTable->setItem(row, 2, new QTableWidgetItem(QString::number(p.burstTime)));
        resultsTable->setItem(row, 3, new QTableWidgetItem(QString::number(p.priority)));
        resultsTable->setItem(row, 4, new QTableWidgetItem(QString::number(p.startTime)));
        resultsTable->setItem(row, 5, new QTableWidgetItem(QString::number(p.completionTime)));
        resultsTable->setItem(row, 6, new QTableWidgetItem(QString("%1 / %2 / %3").arg(p.waitingTime).arg(p.turnaroundTime).arg(p.responseTime)));
    }
}

void SchedulingWindow::saveSimulation()
{
    if (result.processes.empty() || result.ganttChart.empty())
        return;

    const QString algorithm = algorithmKey();
    QString fileAlgorithm = algorithm;
    fileAlgorithm.replace(' ', '_');
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString defaultName = QString("OSimX_%1_%2.osimx.json")
                                    .arg(fileAlgorithm)
                                    .arg(timestamp);

    const QString path = QFileDialog::getSaveFileName(
        this,
        "Save Simulation",
        defaultName,
        "OSimX Simulation (*.osimx.json);;JSON Files (*.json)"
    );

    if (path.isEmpty())
        return;

    QJsonObject root;
    root["application"] = "OSimX";
    root["module"] = "CPU Scheduling";
    root["algorithm"] = algorithm;
    root["timeQuantum"] = quantumSpinBox->value();
    root["savedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonArray processes;
    for (const auto& p : result.processes) {
        QJsonObject item;
        item["pid"] = p.id;
        item["arrival"] = p.arrivalTime;
        item["burst"] = p.burstTime;
        item["priority"] = p.priority;
        item["start"] = p.startTime;
        item["completion"] = p.completionTime;
        item["waiting"] = p.waitingTime;
        item["turnaround"] = p.turnaroundTime;
        item["response"] = p.responseTime;
        processes.append(item);
    }
    root["processes"] = processes;

    QJsonArray timeline;
    for (const auto& segment : result.ganttChart) {
        QJsonObject item;
        item["pid"] = segment.pid;
        item["start"] = segment.startTime;
        item["end"] = segment.endTime;
        timeline.append(item);
    }
    root["timeline"] = timeline;

    QJsonObject metrics;
    metrics["averageWaiting"] = result.avgWaitingTime;
    metrics["averageTurnaround"] = result.avgTurnaroundTime;
    metrics["averageResponse"] = result.avgResponseTime;
    metrics["cpuUtilization"] = result.cpuUtilization;
    metrics["throughput"] = result.throughput;
    root["metrics"] = metrics;

    QJsonArray decisionLog;
    for (const auto& entry : resultLogs)
        decisionLog.append(entry);
    root["decisionLog"] = decisionLog;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Save failed", "OSimX could not write the simulation file.");
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    statusLabel->setText(QString("Simulation saved to %1").arg(path));
}

void SchedulingWindow::onPlaybackIndex(int index)
{
    updatePlaybackState(index);
}

void SchedulingWindow::updatePlaybackState(int index)
{
    if (result.ganttChart.empty()) {
        currentStepLabel->setText("No simulation loaded");
        statusLabel->setText("Run the simulation to begin the guided walkthrough.");
        return;
    }

    index = qBound(0, index, static_cast<int>(result.ganttChart.size()) - 1);
    const auto& segment = result.ganttChart[index];
    ganttWidget->setCurrentStep(index);
    logs->revealUpTo(index);
    currentStepLabel->setText(QString("Step %1 / %2").arg(index + 1).arg(result.ganttChart.size()));

    if (segment.pid < 0) {
        statusLabel->setText(QString("t=%1→%2 · CPU is idle while the scheduler waits for the next process.").arg(segment.startTime).arg(segment.endTime));
    } else {
        statusLabel->setText(resultLogs.value(index, QString("t=%1→%2 · P%3 is executing.").arg(segment.startTime).arg(segment.endTime).arg(segment.pid)));
    }
}
