#include "SchedulingWidget.h"
#include "modules/scheduling/FCFS.cpp"
#include "modules/scheduling/SJF.cpp"
#include "modules/scheduling/SRTF.cpp"
#include "modules/scheduling/RoundRobin.cpp"
#include "modules/scheduling/PriorityScheduling.cpp"

#include <QHeaderView>

using namespace std;

SchedulingWidget::SchedulingWidget(QWidget *parent)
    : QWidget(parent), currentUserId(-1) {
    setupUI();
}

SchedulingWidget::~SchedulingWidget() {}

void SchedulingWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    // TOP SECTION: Input + Algorithm
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(15);
    
    // LEFT: Process Table
    QGroupBox* inputGroup = new QGroupBox("Processes");
    inputGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    
    setupProcessTable();
    inputLayout->addWidget(processTable);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("➕ Add Process");
    clearButton = new QPushButton("🗑 Clear All");
    addButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 5px 15px;");
    clearButton->setStyleSheet("background-color: #f44336; color: white; padding: 5px 15px;");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    inputLayout->addLayout(buttonLayout);
    
    topLayout->addWidget(inputGroup, 2);
    
    // RIGHT: Algorithm Selection
    QGroupBox* algoGroup = new QGroupBox("Algorithm");
    algoGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout* algoLayout = new QVBoxLayout(algoGroup);
    
    algorithmCombo = new QComboBox();
    algorithmCombo->addItems({"FCFS", "SJF", "SRTF", "Round Robin", "Priority"});
    algorithmCombo->setStyleSheet("padding: 5px; font-size: 12px;");
    algoLayout->addWidget(algorithmCombo);
    
    QHBoxLayout* quantumLayout = new QHBoxLayout();
    quantumLayout->addWidget(new QLabel("Quantum:"));
    quantumSpinBox = new QSpinBox();
    quantumSpinBox->setRange(1, 100);
    quantumSpinBox->setValue(2);
    quantumSpinBox->setEnabled(false);
    quantumSpinBox->setStyleSheet("padding: 3px;");
    quantumLayout->addWidget(quantumSpinBox);
    quantumLayout->addStretch();
    algoLayout->addLayout(quantumLayout);
    
    algoLayout->addStretch();
    
    runButton = new QPushButton("▶ Run Simulation");
    runButton->setStyleSheet(
        "background-color: #2196F3; color: white; font-weight: bold; "
        "padding: 10px; border-radius: 5px; font-size: 14px;"
    );
    algoLayout->addWidget(runButton);
    
    topLayout->addWidget(algoGroup, 1);
    mainLayout->addLayout(topLayout);
    
    // MIDDLE: Gantt Chart - Now using dedicated GanttWidget
    QGroupBox* ganttGroup = new QGroupBox("Gantt Chart");
    ganttGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout* ganttLayout = new QVBoxLayout(ganttGroup);
    
    ganttWidget = new GanttWidget(this);
    ganttWidget->setMinimumHeight(150);
    ganttLayout->addWidget(ganttWidget);
    
    mainLayout->addWidget(ganttGroup);
    
    // BOTTOM: Metrics
    QGroupBox* metricsGroup = new QGroupBox("Metrics");
    metricsGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QHBoxLayout* metricsLayout = new QHBoxLayout(metricsGroup);
    
    metricsLabel = new QLabel(
        "⏱ Waiting: -- | ⏰ Turnaround: -- | ⚡ Response: -- | "
        "💻 CPU: --% | 📊 Throughput: --"
    );
    metricsLabel->setStyleSheet("font-size: 13px; padding: 5px; font-weight: bold;");
    metricsLayout->addWidget(metricsLabel);
    metricsLayout->addStretch();
    
    mainLayout->addWidget(metricsGroup);
    
    // BOTTOM BUTTONS
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    saveButton = new QPushButton("💾 Save to Database");
    saveButton->setEnabled(false);
    saveButton->setStyleSheet(
        "background-color: #4CAF50; color: white; font-weight: bold; "
        "padding: 8px 25px; border-radius: 5px;"
    );
    bottomLayout->addStretch();
    bottomLayout->addWidget(saveButton);
    mainLayout->addLayout(bottomLayout);
    
    // CONNECTIONS
    connect(addButton, &QPushButton::clicked, this, &SchedulingWidget::addProcess);
    connect(clearButton, &QPushButton::clicked, this, &SchedulingWidget::clearProcesses);
    connect(runButton, &QPushButton::clicked, this, &SchedulingWidget::runSimulation);
    connect(saveButton, &QPushButton::clicked, this, &SchedulingWidget::saveRequested);
    connect(algorithmCombo, &QComboBox::currentTextChanged, 
            this, &SchedulingWidget::onAlgorithmChange);
}

void SchedulingWidget::setupProcessTable() {
    processTable = new QTableWidget();
    processTable->setColumnCount(4);
    processTable->setHorizontalHeaderLabels({"PID", "Arrival", "Burst", "Priority"});
    processTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    processTable->setStyleSheet("gridline-color: #ccc;");
    processTable->setAlternatingRowColors(true);
    
    // Add sample data
    QStringList sampleData = {
        "P1", "0", "5", "1",
        "P2", "1", "3", "2",
        "P3", "2", "8", "1",
        "P4", "3", "2", "3"
    };
    processTable->setRowCount(sampleData.size() / 4);
    for (int i = 0; i < sampleData.size() / 4; i++) {
        for (int j = 0; j < 4; j++) {
            processTable->setItem(i, j, new QTableWidgetItem(sampleData[i * 4 + j]));
        }
    }
}

void SchedulingWidget::addProcess() {
    int row = processTable->rowCount();
    processTable->insertRow(row);
    processTable->setItem(row, 0, new QTableWidgetItem("P" + QString::number(row + 1)));
    processTable->setItem(row, 1, new QTableWidgetItem("0"));
    processTable->setItem(row, 2, new QTableWidgetItem("1"));
    processTable->setItem(row, 3, new QTableWidgetItem("1"));
}

void SchedulingWidget::clearProcesses() {
    processTable->setRowCount(0);
    processes.clear();
    lastResult = SchedulingResult();
    ganttWidget->clear();
    metricsLabel->setText(
        "⏱ Waiting: -- | ⏰ Turnaround: -- | ⚡ Response: -- | "
        "💻 CPU: --% | 📊 Throughput: --"
    );
    saveButton->setEnabled(false);
}

void SchedulingWidget::runSimulation() {
    qDebug() << "========== RUN SIMULATION STARTED ==========";
    
    processes.clear();
    int rowCount = processTable->rowCount();
    qDebug() << "Row count:" << rowCount;
    
    if (rowCount == 0) {
        QMessageBox::warning(this, "No Processes", 
                            "Please add at least one process.");
        return;
    }
    
    for (int i = 0; i < rowCount; i++) {
        QTableWidgetItem* item0 = processTable->item(i, 0);
        QTableWidgetItem* item1 = processTable->item(i, 1);
        QTableWidgetItem* item2 = processTable->item(i, 2);
        QTableWidgetItem* item3 = processTable->item(i, 3);
        
        if (!item0 || !item1 || !item2 || !item3) {
            qDebug() << "ERROR: Null item at row" << i;
            continue;
        }
        
        QString pid = item0->text();
        int arrival = item1->text().toInt();
        int burst = item2->text().toInt();
        int priority = item3->text().toInt();
        
        qDebug() << "Process" << i << ":" << pid << arrival << burst << priority;
        
        int pidNum = pid.remove(0, 1).toInt();
        processes.push_back(Process(pidNum, arrival, burst, priority));
    }
    
    qDebug() << "Total processes read:" << processes.size();
    
    if (processes.empty()) {
        QMessageBox::warning(this, "No Processes", 
                            "Please add at least one valid process.");
        return;
    }
    
    QString algo = algorithmCombo->currentText();
    qDebug() << "Selected algorithm:" << algo;
    
    CPUScheduler* scheduler = nullptr;
    
    if (algo == "FCFS") {
        scheduler = new FCFS();
    } else if (algo == "SJF") {
        scheduler = new SJF();
    } else if (algo == "SRTF") {
        scheduler = new SRTF();
    } else if (algo == "Round Robin") {
        int quantum = quantumSpinBox->value();
        scheduler = new RoundRobin(quantum);
        qDebug() << "Quantum:" << quantum;
    } else if (algo == "Priority") {
        scheduler = new PriorityScheduling();
    }
    
    if (scheduler) {
        qDebug() << "Running scheduler...";
        lastResult = scheduler->schedule(processes);
        qDebug() << "Scheduler finished!";
        qDebug() << "Gantt chart size:" << lastResult.ganttChart.size();
        
        // Update Gantt widget with data
        ganttWidget->setGanttData(lastResult.ganttChart);
        
        updateMetrics();
        saveButton->setEnabled(true);
        emit simulationRun();
        
        delete scheduler;
        qDebug() << "========== RUN SIMULATION COMPLETE ==========";
    } else {
        qDebug() << "ERROR: scheduler is NULL!";
        QMessageBox::warning(this, "Error", "Failed to create scheduler!");
    }
}

void SchedulingWidget::onAlgorithmChange(const QString& algo) {
    quantumSpinBox->setEnabled(algo == "Round Robin");
}

void SchedulingWidget::updateMetrics() {
    metricsLabel->setText(
        QString("⏱ Waiting: %1 | ⏰ Turnaround: %2 | ⚡ Response: %3 | "
                "💻 CPU: %4% | 📊 Throughput: %5")
        .arg(lastResult.avgWaitingTime, 0, 'f', 2)
        .arg(lastResult.avgTurnaroundTime, 0, 'f', 2)
        .arg(lastResult.avgResponseTime, 0, 'f', 2)
        .arg(lastResult.cpuUtilization, 0, 'f', 1)
        .arg(lastResult.throughput, 0, 'f', 3)
    );
}