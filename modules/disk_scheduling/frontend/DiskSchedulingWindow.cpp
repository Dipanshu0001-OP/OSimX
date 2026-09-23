#include "DiskSchedulingWindow.h"
#include "modules/disk_scheduling/backend/DiskScheduler.h"
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
#include <algorithm>
#include <functional>

DiskSchedulingWindow::DiskSchedulingWindow(QWidget* parent) : QWidget(parent)
{
    setupUI();
    setupRequestTable();
    updateAlgorithmInfo();
}

void DiskSchedulingWindow::setupUI()
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* header = new QWidget;
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(22, 16, 22, 16);
    auto* backButton = new QPushButton("‹  Simulations");
    backButton->setObjectName("secondaryButton");
    auto* titleBlock = new QVBoxLayout;
    auto* title = new QLabel("Disk Scheduling");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Experiment with request ordering, head movement, seek cost, and direction.");
    subtitle->setObjectName("pageSubtitle");
    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);
    headerLayout->addWidget(backButton, 0, Qt::AlignTop);
    headerLayout->addSpacing(12);
    headerLayout->addLayout(titleBlock);
    headerLayout->addStretch();
    outer->addWidget(header);
    connect(backButton, &QPushButton::clicked, this, &DiskSchedulingWindow::backRequested);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget;
    auto* main = new QVBoxLayout(content);
    main->setContentsMargins(22, 8, 22, 28);
    main->setSpacing(14);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->setMinimumHeight(285);

    auto* requestBox = new QGroupBox("Request queue");
    auto* requestLayout = new QVBoxLayout(requestBox);
    requestLayout->setContentsMargins(14, 18, 14, 14);
    requestsTable = new QTableWidget;
    requestsTable->setColumnCount(1);
    requestsTable->setHorizontalHeaderLabels({"Cylinder"});
    requestsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    requestsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    requestsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    requestsTable->setMinimumHeight(225);
    requestsTable->verticalHeader()->setVisible(false);
    requestsTable->verticalScrollBar()->setSingleStep(32);
    requestLayout->addWidget(requestsTable, 1);

    auto* rowButtons = new QHBoxLayout;
    auto* addButton = new QPushButton("＋ Add Request");
    auto* removeButton = new QPushButton("− Delete Selected");
    auto* clearButton = new QPushButton("Clear All");
    addButton->setObjectName("secondaryButton");
    removeButton->setObjectName("secondaryButton");
    clearButton->setObjectName("dangerButton");
    rowButtons->addWidget(addButton);
    rowButtons->addWidget(removeButton);
    rowButtons->addWidget(clearButton);
    rowButtons->addStretch();
    requestLayout->addLayout(rowButtons);

    auto* controlBox = new QGroupBox("Controls");
    auto* controls = new QVBoxLayout(controlBox);
    controls->setContentsMargins(14, 18, 14, 14);
    auto* algorithmLabel = new QLabel("Algorithm");
    algorithmCombo = new QComboBox;
    algorithmCombo->addItems({"FCFS", "SSTF", "SCAN", "CSCAN", "LOOK", "CLOOK"});
    controls->addWidget(algorithmLabel);
    controls->addWidget(algorithmCombo);

    auto* headRow = new QHBoxLayout;
    headRow->addWidget(new QLabel("Initial head"));
    headSpinBox = new QSpinBox;
    headSpinBox->setRange(0, 9999);
    headSpinBox->setValue(53);
    headRow->addWidget(headSpinBox);
    controls->addLayout(headRow);

    auto* maxRow = new QHBoxLayout;
    maxRow->addWidget(new QLabel("Max cylinder"));
    maxCylinderSpinBox = new QSpinBox;
    maxCylinderSpinBox->setRange(1, 9999);
    maxCylinderSpinBox->setValue(199);
    maxRow->addWidget(maxCylinderSpinBox);
    controls->addLayout(maxRow);

    auto* directionRow = new QHBoxLayout;
    directionRow->addWidget(new QLabel("Initial direction"));
    directionCombo = new QComboBox;
    directionCombo->addItems({"Up", "Down"});
    directionRow->addWidget(directionCombo);
    controls->addLayout(directionRow);

    algorithmInfo = new QLabel;
    algorithmInfo->setWordWrap(true);
    algorithmInfo->setObjectName("infoLabel");
    controls->addWidget(algorithmInfo);
    controls->addStretch();

    auto* runButton = new QPushButton("▶  Run Simulation");
    runButton->setObjectName("primaryButton");
    runButton->setMinimumHeight(44);
    controls->addWidget(runButton);

    splitter->addWidget(requestBox);
    splitter->addWidget(controlBox);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({680, 420});
    main->addWidget(splitter);

    auto* movementBox = new QGroupBox("Interactive head movement");
    auto* movementLayout = new QVBoxLayout(movementBox);
    movementLayout->setContentsMargins(12, 18, 12, 12);
    track = new DiskTrackWidget;
    movementLayout->addWidget(track);
    playback = new PlaybackBar;
    movementLayout->addWidget(playback);
    main->addWidget(movementBox);

    auto* statusRow = new QHBoxLayout;
    stepLabel = new QLabel("No simulation loaded");
    stepLabel->setObjectName("stepPill");
    statusLabel = new QLabel("Run the simulation to reveal why the disk head moves the way it does.");
    statusLabel->setWordWrap(true);
    statusRow->addWidget(stepLabel, 0);
    statusRow->addWidget(statusLabel, 1);
    main->addLayout(statusRow);

    auto* metricsBox = new QGroupBox("Seek metrics");
    auto* metricsLayout = new QHBoxLayout(metricsBox);
    metricsLayout->setContentsMargins(14, 18, 14, 14);
    metricsLabel = new QLabel("Total seek --   •   Average seek --");
    metricsLabel->setObjectName("metricsValue");
    metricsLayout->addWidget(metricsLabel);
    main->addWidget(metricsBox);

    auto* logsBox = new QGroupBox("Decision log · revealed step by step");
    auto* logsLayout = new QVBoxLayout(logsBox);
    logsLayout->setContentsMargins(12, 18, 12, 12);
    logs = new SimulationLogWidget;
    logsLayout->addWidget(logs);
    main->addWidget(logsBox);

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

    connect(addButton, &QPushButton::clicked, this, &DiskSchedulingWindow::addRequest);
    connect(removeButton, &QPushButton::clicked, this, &DiskSchedulingWindow::removeSelectedRequests);
    connect(clearButton, &QPushButton::clicked, this, &DiskSchedulingWindow::clearRequests);
    connect(runButton, &QPushButton::clicked, this, &DiskSchedulingWindow::runSimulation);
    connect(algorithmCombo, &QComboBox::currentTextChanged, this, [this](const QString&) { updateAlgorithmInfo(); });
    connect(maxCylinderSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &DiskSchedulingWindow::onMaxCylinderChanged);
    connect(playback, &PlaybackBar::indexChanged, this, &DiskSchedulingWindow::onPlaybackIndex);
    connect(saveButton, &QPushButton::clicked, this, &DiskSchedulingWindow::saveSimulation);
}

void DiskSchedulingWindow::setupRequestTable()
{
    const int sample[5] = {98, 183, 37, 122, 14};
    requestsTable->setRowCount(5);
    for (int i = 0; i < 5; ++i)
        requestsTable->setItem(i, 0, new QTableWidgetItem(QString::number(sample[i])));
    requestsTable->setCurrentCell(0, 0);
}

void DiskSchedulingWindow::addRequest()
{
    const int row = requestsTable->rowCount();
    requestsTable->insertRow(row);
    requestsTable->setItem(row, 0, new QTableWidgetItem("0"));
    requestsTable->scrollToItem(requestsTable->item(row, 0));
    requestsTable->setCurrentCell(row, 0);
}

void DiskSchedulingWindow::removeSelectedRequests()
{
    const auto rows = requestsTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        statusLabel->setText("Select one or more request rows to delete.");
        return;
    }
    QList<int> indices;
    for (const auto& index : rows) indices << index.row();
    std::sort(indices.begin(), indices.end(), std::greater<int>());
    for (int row : indices) requestsTable->removeRow(row);
}

void DiskSchedulingWindow::clearRequests()
{
    requestsTable->setRowCount(0);
    result = DiskResult();
    resultLogs.clear();
    track->setResult(result, maxCylinderSpinBox->value(), headSpinBox->value(), {});
    playback->setTotal(0);
    logs->clearLogs();
    saveButton->setEnabled(false);
    metricsLabel->setText("Total seek --   •   Average seek --");
    stepLabel->setText("No simulation loaded");
    statusLabel->setText("Queue cleared. Add requests whenever you are ready.");
}

void DiskSchedulingWindow::onMaxCylinderChanged(int value)
{
    headSpinBox->setMaximum(value);
    if (headSpinBox->value() > value)
        headSpinBox->setValue(value);
    track->setResult(result, value, headSpinBox->value(), {});
}

void DiskSchedulingWindow::updateAlgorithmInfo()
{
    const QString algorithm = algorithmCombo->currentText();
    if (algorithm == "FCFS")
        algorithmInfo->setText("Requests are served in the exact order they entered the queue.");
    else if (algorithm == "SSTF")
        algorithmInfo->setText("At each step, serve the request closest to the current head position.");
    else if (algorithm == "SCAN")
        algorithmInfo->setText("Sweep in one direction, reach the boundary when needed, then reverse.");
    else if (algorithm == "CSCAN")
        algorithmInfo->setText("Sweep in one direction, wrap to the opposite boundary, then continue the sweep.");
    else if (algorithm == "LOOK")
        algorithmInfo->setText("Sweep like SCAN, but reverse at the last pending request instead of the disk boundary.");
    else
        algorithmInfo->setText("Sweep like C-SCAN, but wrap between the first and last pending request rather than the physical boundaries.");
}

void DiskSchedulingWindow::runSimulation()
{
    const int maxCylinder = maxCylinderSpinBox->value();
    const int head = headSpinBox->value();
    if (head < 0 || head > maxCylinder) {
        QMessageBox::warning(this, "Invalid head", "The initial head must lie within the selected disk range.");
        return;
    }

    std::vector<int> requests;
    requests.reserve(requestsTable->rowCount());
    for (int row = 0; row < requestsTable->rowCount(); ++row) {
        const auto* item = requestsTable->item(row, 0);
        bool ok = false;
        const int cylinder = item ? item->text().trimmed().toInt(&ok) : -1;
        if (!ok || cylinder < 0 || cylinder > maxCylinder) {
            QMessageBox::warning(this, "Invalid request", QString("Cylinder in row %1 must be between 0 and %2.").arg(row + 1).arg(maxCylinder));
            return;
        }
        requests.push_back(cylinder);
    }

    if (requests.empty()) {
        QMessageBox::warning(this, "No requests", "Add at least one disk request before running the simulation.");
        return;
    }

    result = DiskScheduler::schedule(algorithmCombo->currentText().toStdString(), head, requests,
                                     maxCylinder, directionCombo->currentIndex() == 0);
    track->setResult(result, maxCylinder, head, requests);
    playback->setTotal(static_cast<int>(result.steps.size()));
    resultLogs = buildLogs();
    logs->setLogs(resultLogs);
    metricsLabel->setText(QString("Total seek %1   •   Average seek %2 cylinders")
                              .arg(result.totalSeek)
                              .arg(result.averageSeek, 0, 'f', 2));
    updateView(0);
    saveButton->setEnabled(true);
    playback->startPlayback();
}

QStringList DiskSchedulingWindow::buildLogs() const
{
    QStringList output;
    const QString algorithm = algorithmCombo->currentText();
    for (int i = 0; i < static_cast<int>(result.steps.size()); ++i) {
        const auto& step = result.steps[i];
        output << QString("%1 · head %2→%3, move %4 cylinders, cumulative seek %5. %6")
                      .arg(QString("Step %1").arg(i + 1))
                      .arg(step.from)
                      .arg(step.to)
                      .arg(step.distance)
                      .arg(step.cumulative)
                      .arg(QString::fromStdString(step.reason));
    }
    if (output.isEmpty())
        output << QString("%1 produced no movement because there were no requests.").arg(algorithm);
    return output;
}

void DiskSchedulingWindow::saveSimulation()
{
    if (result.steps.empty())
        return;

    const QString algorithm = algorithmCombo->currentText();
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
    root["module"] = "Disk Scheduling";
    root["algorithm"] = algorithm;
    root["initialHead"] = headSpinBox->value();
    root["maxCylinder"] = maxCylinderSpinBox->value();
    root["direction"] = directionCombo->currentText();
    root["savedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonArray requests;
    for (const int request : result.sequence)
        requests.append(request);
    root["sequence"] = requests;

    QJsonArray steps;
    for (const auto& step : result.steps) {
        QJsonObject item;
        item["index"] = step.index;
        item["from"] = step.from;
        item["to"] = step.to;
        item["distance"] = step.distance;
        item["cumulative"] = step.cumulative;
        item["reason"] = QString::fromStdString(step.reason);
        QJsonArray remaining;
        for (const int cylinder : step.remaining)
            remaining.append(cylinder);
        item["remaining"] = remaining;
        steps.append(item);
    }
    root["steps"] = steps;
    root["totalSeek"] = result.totalSeek;
    root["averageSeek"] = result.averageSeek;

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

void DiskSchedulingWindow::onPlaybackIndex(int index)
{
    updateView(index);
}

void DiskSchedulingWindow::updateView(int index)
{
    if (result.steps.empty()) {
        track->setResult(result, maxCylinderSpinBox->value(), headSpinBox->value(), {});
        stepLabel->setText("No simulation loaded");
        statusLabel->setText("Run the simulation to reveal the disk head decisions.");
        logs->clearLogs();
        return;
    }

    index = qBound(0, index, static_cast<int>(result.steps.size()) - 1);
    track->setCurrentStep(index);
    logs->revealUpTo(index);
    const auto& step = result.steps[index];
    stepLabel->setText(QString("Step %1 / %2").arg(index + 1).arg(result.steps.size()));
    statusLabel->setText(QString("Head %1 → %2 · moved %3 cylinders · %4")
                             .arg(step.from)
                             .arg(step.to)
                             .arg(step.distance)
                             .arg(QString::fromStdString(step.reason)));
}
