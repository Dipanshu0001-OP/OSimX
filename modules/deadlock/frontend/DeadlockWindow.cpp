
#include "DeadlockWindow.h"
#include "database/SimulationRepository.h"

#include <QComboBox>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QStringList>

#include <utility>
#include <vector>

namespace {

QTableWidget* makeTable()
{
    auto* t = new QTableWidget;
    t->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );
    t->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents
    );
    t->setMinimumHeight(115);
    return t;
}

void put(QTableWidget* t, int row, int col, int value)
{
    t->setItem(
        row, col,
        new QTableWidgetItem(QString::number(value))
    );
}

QJsonArray vectorToJson(const std::vector<int>& values)
{
    QJsonArray result;

    for (int value : values)
        result.append(value);

    return result;
}

QJsonArray stringsToJson(
    const std::vector<std::string>& values
)
{
    QJsonArray result;

    for (const auto& value : values)
        result.append(QString::fromStdString(value));

    return result;
}

bool validMatrix(
    const QJsonValue& value,
    int rows,
    int columns
)
{
    if (!value.isArray())
        return false;

    const QJsonArray matrix = value.toArray();

    if (matrix.size() != rows)
        return false;

    for (const auto& rowValue : matrix) {
        if (!rowValue.isArray())
            return false;

        const QJsonArray row = rowValue.toArray();

        if (row.size() != columns)
            return false;

        for (const auto& cell : row) {
            if (!cell.isDouble() ||
                cell.toDouble() < 0 ||
                cell.toDouble() != cell.toInt())
                return false;
        }
    }

    return true;
}

void restoreMatrix(
    QTableWidget* table,
    const QJsonArray& matrix
)
{
    for (int i = 0; i < matrix.size(); ++i) {
        const QJsonArray row = matrix[i].toArray();

        for (int j = 0; j < row.size(); ++j)
            put(table, i, j, row[j].toInt());
    }
}

void restoreVector(
    QTableWidget* table,
    const QJsonArray& values
)
{
    for (int j = 0; j < values.size(); ++j)
        put(table, 0, j, values[j].toInt());
}

}

DeadlockWindow::DeadlockWindow(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("deadlockWindow");

    setStyleSheet(R"QSS(
        QWidget#deadlockWindow {
            background: #F3F4EE;
            color: #193729;
        }

        QWidget#deadlockWindow QFrame#dlCard {
            background: #FFFFFF;
            border: 1px solid #DCE6D9;
            border-radius: 14px;
        }

        QWidget#deadlockWindow QLabel#dlTitle {
            color: #193729;
            font-size: 27px;
            font-weight: 800;
        }

        QWidget#deadlockWindow QLabel#dlEyebrow {
            color: #356B4A;
            font-size: 11px;
            font-weight: 700;
        }

        QWidget#deadlockWindow QLabel#dlSection {
            color: #193729;
            font-size: 17px;
            font-weight: 700;
        }

        QWidget#deadlockWindow QLabel#dlMuted {
            color: #607365;
            font-size: 13px;
        }

        QWidget#deadlockWindow QLabel#dlStatus {
            background: #E4EDE2;
            border: 1px solid #C9DBC9;
            border-radius: 9px;
            padding: 12px;
            color: #28583B;
            font-weight: 700;
        }

        QWidget#deadlockWindow QTableWidget {
            background: #FFFFFF;
            alternate-background-color: #F6F9F4;
            color: #193729;
            border: 1px solid #DCE6D9;
            border-radius: 8px;
            gridline-color: #E8EEE6;
            selection-background-color: #DCEBDD;
            selection-color: #193729;
        }

        QWidget#deadlockWindow QHeaderView::section {
            background: #E8EFE5;
            color: #365843;
            border: none;
            border-bottom: 1px solid #D4E0D1;
            padding: 8px;
            font-weight: 700;
        }

        QWidget#deadlockWindow QSpinBox,
        QWidget#deadlockWindow QComboBox {
            background: white;
            color: #193729;
            border: 1px solid #C8D7C8;
            border-radius: 8px;
            padding: 8px 10px;
            min-height: 22px;
        }

        QWidget#deadlockWindow QTextEdit {
            background: #F8FAF7;
            color: #193729;
            border: 1px solid #DCE6D9;
            border-radius: 8px;
            padding: 12px;
        }

        QWidget#deadlockWindow QPushButton {
            background: #FFFFFF;
            color: #28583B;
            border: 1px solid #C7D8C7;
            border-radius: 9px;
            padding: 9px 15px;
            font-weight: 700;
        }

        QWidget#deadlockWindow QPushButton:hover {
            background: #E8F1E6;
        }

        QWidget#deadlockWindow QPushButton#dlPrimary {
            background: #356B4A;
            color: white;
            border-color: #356B4A;
        }

        QWidget#deadlockWindow QPushButton#dlPrimary:hover {
            background: #28583B;
        }

        QWidget#deadlockWindow QPushButton:disabled {
            background: #E8ECE6;
            color: #839186;
            border-color: #D5DDD3;
        }

        QWidget#deadlockWindow QScrollArea {
            background: transparent;
            border: none;
        }
    )QSS");

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    outer->addWidget(scroll);

    auto* content = new QWidget;
    scroll->setWidget(content);

    auto* root = new QVBoxLayout(content);
    root->setContentsMargins(32, 26, 32, 36);
    root->setSpacing(18);
    root->setAlignment(Qt::AlignTop);

    auto* top = new QHBoxLayout;
    auto* titles = new QVBoxLayout;

    auto* eyebrow = new QLabel("RESOURCE MANAGEMENT");
    eyebrow->setObjectName("dlEyebrow");

    auto* heading = new QLabel("Deadlock Analysis");
    heading->setObjectName("dlTitle");

    auto* intro = new QLabel(
        "Explore safe states, resource requests, "
        "and deadlock detection."
    );
    intro->setObjectName("dlMuted");
    intro->setWordWrap(true);

    titles->addWidget(eyebrow);
    titles->addWidget(heading);
    titles->addWidget(intro);

    top->addLayout(titles, 1);

    auto* back = new QPushButton("Back to dashboard");
    top->addWidget(back, 0, Qt::AlignTop);

    connect(
        back,
        &QPushButton::clicked,
        this,
        &DeadlockWindow::backRequested
    );

    root->addLayout(top);

    auto makeCard = [](const QString& title) {
        auto* card = new QFrame;
        card->setObjectName("dlCard");

        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(20, 18, 20, 20);
        layout->setSpacing(12);

        auto* label = new QLabel(title);
        label->setObjectName("dlSection");
        layout->addWidget(label);

        return std::make_pair(card, layout);
    };

    // SIMULATION SETUP

    auto setup = makeCard("Simulation setup");

    auto* controls = new QHBoxLayout;
    controls->setSpacing(12);

    mode = new QComboBox;
    mode->addItems({
        "Banker's Algorithm (avoidance)",
        "Deadlock Detection"
    });

    controls->addWidget(mode, 2);
    controls->addWidget(new QLabel("Processes"));

    processCount = new QSpinBox;
    processCount->setRange(1, 20);
    processCount->setValue(5);
    controls->addWidget(processCount);

    controls->addWidget(new QLabel("Resources"));

    resourceCount = new QSpinBox;
    resourceCount->setRange(1, 10);
    resourceCount->setValue(3);
    controls->addWidget(resourceCount);

    auto* sample = new QPushButton("Load example");
    controls->addWidget(sample);

    setup.second->addLayout(controls);
    root->addWidget(setup.first);

    // MATRICES

    auto* matrices = new QHBoxLayout;
    matrices->setSpacing(16);

    auto allocCard = makeCard("Allocation matrix");

    allocation = makeTable();
    allocation->setMinimumHeight(210);

    allocCard.second->addWidget(allocation);
    matrices->addWidget(allocCard.first, 1);

    auto secondCard = makeCard("Maximum claim matrix");

    auto* secondLabel = qobject_cast<QLabel*>(
        secondCard.second->itemAt(0)->widget()
    );

    second = makeTable();
    second->setMinimumHeight(210);

    secondCard.second->addWidget(second);
    matrices->addWidget(secondCard.first, 1);

    root->addLayout(matrices);

    // AVAILABLE RESOURCES

    auto availCard = makeCard("Available resources");

    available = makeTable();
    available->setFixedHeight(87);

    availCard.second->addWidget(available);
    root->addWidget(availCard.first);

    // ANALYSIS RESULTS

    auto resultCard = makeCard("Analysis results");
    auto* resultTop = new QHBoxLayout;

    status = new QLabel("Ready to analyze");
    status->setObjectName("dlStatus");
    status->setWordWrap(true);

    auto* execute = new QPushButton("Run analysis");
    execute->setObjectName("dlPrimary");

    saveButton = new QPushButton("Save simulation");
    saveButton->setEnabled(false);

    resultTop->addWidget(status, 1);
    resultTop->addWidget(execute);
    resultTop->addWidget(saveButton);

    resultCard.second->addLayout(resultTop);

    explanation = new QTextEdit;
    explanation->setReadOnly(true);
    explanation->setMinimumHeight(190);

    explanation->setPlaceholderText(
        "The analysis and execution trace "
        "will appear here."
    );

    resultCard.second->addWidget(explanation);
    root->addWidget(resultCard.first);

    // RESOURCE REQUEST

    auto requestCard = makeCard("Resource request");

    auto* requestNote = new QLabel(
        "Test whether a process can safely "
        "receive additional resources."
    );
    requestNote->setObjectName("dlMuted");
    requestNote->setWordWrap(true);

    requestCard.second->addWidget(requestNote);

    auto* requestRow = new QHBoxLayout;
    requestRow->setSpacing(12);

    requestPid = new QSpinBox;
    requestPid->setPrefix("P");

    requestRow->addWidget(new QLabel("Process"));
    requestRow->addWidget(requestPid);

    requestTable = makeTable();
    requestTable->setFixedHeight(87);

    requestRow->addWidget(requestTable, 1);

    auto* requestButton = new QPushButton(
        "Evaluate request"
    );
    requestRow->addWidget(requestButton);

    requestCard.second->addLayout(requestRow);

    auto* note = new QLabel(
        "An unsafe state is not necessarily a deadlock. "
        "Detection evaluates outstanding requests "
        "rather than maximum claims."
    );
    note->setObjectName("dlMuted");
    note->setWordWrap(true);

    requestCard.second->addWidget(note);
    root->addWidget(requestCard.first);

    // CONNECTIONS

    connect(
        processCount,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &DeadlockWindow::resizeTables
    );

    connect(
        resourceCount,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        &DeadlockWindow::resizeTables
    );

    connect(
        mode,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [=](int index) {
            secondLabel->setText(
                index
                    ? "Outstanding request matrix"
                    : "Maximum claim matrix"
            );

            requestButton->setEnabled(!index);
            requestPid->setEnabled(!index);
            requestTable->setEnabled(!index);

            invalidateSavedResult();
        }
    );

    connect(
        sample,
        &QPushButton::clicked,
        this,
        &DeadlockWindow::loadExample
    );

    connect(
        execute,
        &QPushButton::clicked,
        this,
        &DeadlockWindow::runSimulation
    );

    connect(
        requestButton,
        &QPushButton::clicked,
        this,
        &DeadlockWindow::evaluateRequest
    );

    connect(
        saveButton,
        &QPushButton::clicked,
        this,
        &DeadlockWindow::saveSimulation
    );

    for (auto* table : {
        allocation, second, available, requestTable
    }) {
        connect(
            table,
            &QTableWidget::itemChanged,
            this,
            [this](QTableWidgetItem*) {
                invalidateSavedResult();
            }
        );
    }

    connect(
        requestPid,
        QOverload<int>::of(&QSpinBox::valueChanged),
        this,
        [this](int) {
            invalidateSavedResult();
        }
    );

    resizeTables();
    loadExample();
}

void DeadlockWindow::invalidateSavedResult()
{
    hasResult = false;
    lastInputData = QJsonObject();
    lastOutputData = QJsonObject();

    if (saveButton)
        saveButton->setEnabled(false);
}

void DeadlockWindow::resizeTables()
{
    invalidateSavedResult();

    const int n = processCount->value();
    const int m = resourceCount->value();

    requestPid->setMaximum(n - 1);

    QStringList cols;
    QStringList rows;

    for (int j = 0; j < m; ++j)
        cols << QString("R%1").arg(j);

    for (int i = 0; i < n; ++i)
        rows << QString("P%1").arg(i);

    for (auto* t : {allocation, second}) {
        t->setRowCount(n);
        t->setColumnCount(m);
        t->setHorizontalHeaderLabels(cols);
        t->setVerticalHeaderLabels(rows);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (!t->item(i, j))
                    put(t, i, j, 0);
            }
        }
    }

    for (auto* t : {available, requestTable}) {
        t->setRowCount(1);
        t->setColumnCount(m);
        t->setHorizontalHeaderLabels(cols);

        for (int j = 0; j < m; ++j) {
            if (!t->item(0, j))
                put(t, 0, j, 0);
        }
    }
}

void DeadlockWindow::loadExample()
{
    invalidateSavedResult();

    processCount->setValue(5);
    resourceCount->setValue(3);
    mode->setCurrentIndex(0);

    const int a[5][3] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    const int m[5][3] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    const int v[3] = {3, 3, 2};

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 3; ++j) {
            put(allocation, i, j, a[i][j]);
            put(second, i, j, m[i][j]);
        }
    }

    for (int j = 0; j < 3; ++j) {
        put(available, 0, j, v[j]);
        put(requestTable, 0, j, 0);
    }

    requestPid->setValue(1);

    status->setText(
        "Loaded classic 5-process Banker's example. "
        "Click Run analysis."
    );

    explanation->clear();
}

bool DeadlockWindow::readTable(
    QTableWidget* t,
    int row,
    int col,
    int& value
)
{
    auto* item = t->item(row, col);
    bool ok = false;

    const int number = item
        ? item->text().trimmed().toInt(&ok)
        : 0;

    if (!ok || number < 0) {
        QMessageBox::warning(
            this,
            "Invalid input",
            QString(
                "Enter a non-negative integer "
                "in row %1, column %2."
            )
            .arg(row + 1)
            .arg(col + 1)
        );

        return false;
    }

    value = number;
    return true;
}

bool DeadlockWindow::readInputs(
    deadlock::Matrix& a,
    deadlock::Matrix& b,
    std::vector<int>& v
)
{
    const int n = processCount->value();
    const int m = resourceCount->value();

    a.assign(n, std::vector<int>(m));
    b = a;
    v.resize(m);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (!readTable(allocation, i, j, a[i][j]) ||
                !readTable(second, i, j, b[i][j])) {
                return false;
            }
        }
    }

    for (int j = 0; j < m; ++j) {
        if (!readTable(available, 0, j, v[j]))
            return false;
    }

    return true;
}

void DeadlockWindow::showResult(
    const deadlock::Result& result,
    const QString& prefix
)
{
    if (!result.valid) {
        status->setText(
            "Invalid input: " +
            QString::fromStdString(result.error)
        );

        explanation->clear();
        return;
    }

    QStringList sequence;
    QStringList unfinished;

    for (int pid : result.sequence)
        sequence << QString("P%1").arg(pid);

    for (int pid : result.unfinished)
        unfinished << QString("P%1").arg(pid);

    const bool detection = mode->currentIndex() == 1;

    status->setText(
        prefix +
        (result.safe
            ? (detection
                ? "No deadlock detected"
                : "SAFE state")
            : (detection
                ? "DEADLOCK detected"
                : "UNSAFE state"))
    );

    QStringList lines;

    lines << "Completion order: " +
        (sequence.isEmpty()
            ? QString("(none)")
            : sequence.join(" → "));

    if (!unfinished.isEmpty()) {
        lines << "Unfinished processes: " +
            unfinished.join(", ");
    }

    lines << "";
    lines << "Execution trace:";

    for (const auto& step : result.steps)
        lines << QString::fromStdString(step);

    explanation->setPlainText(lines.join('\n'));
}

QJsonObject DeadlockWindow::captureInputs() const
{
    QJsonObject input;

    input["schemaVersion"] = 1;
    input["mode"] = mode->currentIndex();
    input["processCount"] = processCount->value();
    input["resourceCount"] = resourceCount->value();
    input["requestPid"] = requestPid->value();

    auto captureTable = [](QTableWidget* t) {
        QJsonArray rows;

        for (int i = 0; i < t->rowCount(); ++i) {
            QJsonArray row;

            for (int j = 0; j < t->columnCount(); ++j) {
                const auto* item = t->item(i, j);
                row.append(item ? item->text().toInt() : 0);
            }

            rows.append(row);
        }

        return rows;
    };

    input["allocation"] = captureTable(allocation);
    input["second"] = captureTable(second);
    input["available"] = captureTable(available);
    input["requestTable"] = captureTable(requestTable);

    return input;
}

QJsonObject DeadlockWindow::captureResult(
    const deadlock::Result& result,
    const QString& operation,
    const QString& message
) const
{
    QJsonObject output;

    output["schemaVersion"] = 1;
    output["operation"] = operation;
    output["valid"] = result.valid;
    output["safe"] = result.safe;
    output["error"] = QString::fromStdString(result.error);

    output["sequence"] = vectorToJson(result.sequence);
    output["unfinished"] = vectorToJson(result.unfinished);
    output["steps"] = stringsToJson(result.steps);

    output["message"] = message;
    output["statusText"] = status->text();
    output["explanationText"] = explanation->toPlainText();

    return output;
}

void DeadlockWindow::runSimulation()
{
    invalidateSavedResult();

    deadlock::Matrix a, b;
    std::vector<int> v;

    if (!readInputs(a, b, v))
        return;

    const deadlock::Result result =
        mode->currentIndex()
            ? deadlock::detect(a, b, v)
            : deadlock::bankers(a, b, v);

    showResult(result, "");

    if (!result.valid)
        return;

    lastInputData = captureInputs();

    lastOutputData = captureResult(
        result,
        "analysis"
    );

    hasResult = true;
    saveButton->setEnabled(true);
}

void DeadlockWindow::evaluateRequest()
{
    if (mode->currentIndex() != 0)
        return;

    invalidateSavedResult();

    deadlock::Matrix a, b;
    std::vector<int> v;

    if (!readInputs(a, b, v))
        return;

    std::vector<int> request(resourceCount->value());

    for (int j = 0; j < resourceCount->value(); ++j) {
        if (!readTable(requestTable, 0, j, request[j]))
            return;
    }

    const auto result = deadlock::requestResources(
        a,
        b,
        v,
        requestPid->value(),
        request
    );

    const QString reason =
        QString::fromStdString(result.reason);

    status->setText(reason);

    if (result.safety.valid) {
        showResult(
            result.safety,
            reason + " | "
        );
    } else {
        explanation->setPlainText(
            "The request was not granted; "
            "original matrices remain unchanged."
        );
    }

    lastInputData = captureInputs();

    lastOutputData = captureResult(
        result.safety,
        "resourceRequest",
        reason
    );

    lastOutputData["granted"] = result.granted;
    lastOutputData["request"] = vectorToJson(request);

    hasResult = true;
    saveButton->setEnabled(true);
}

void DeadlockWindow::saveSimulation()
{
    if (!hasResult) {
        QMessageBox::information(
            this,
            "Save simulation",
            "Run an analysis before saving."
        );
        return;
    }

    const QString algorithm =
        mode->currentIndex() == 0
            ? "Banker's Algorithm"
            : "Deadlock Detection";

    const QString summary =
        QString("%1 | %2 processes, %3 resources | %4")
            .arg(algorithm)
            .arg(processCount->value())
            .arg(resourceCount->value())
            .arg(status->text());

    QJsonObject payload;
    payload["module"] = "Deadlock";
    payload["algorithm"] = algorithm;
    payload["inputData"] = lastInputData;
    payload["outputData"] = lastOutputData;

    QString error;

    const bool saved =
        SimulationRepository::instance().save(
            "Deadlock",
            algorithm,
            summary,
            lastInputData,
            lastOutputData,
            payload,
            error
        );

    if (!saved) {
        QMessageBox::warning(
            this,
            "Save failed",
            error
        );
        return;
    }

    QMessageBox::information(
        this,
        "Simulation saved",
        "Your Deadlock simulation has been saved "
        "to your account."
    );

    emit simulationSaved();
}

bool DeadlockWindow::restoreSimulation(
    const QJsonObject& inputData,
    const QJsonObject& outputData
)
{
    const int n = inputData.value("processCount").toInt();
    const int m = inputData.value("resourceCount").toInt();
    const int algorithmMode = inputData.value("mode").toInt(-1);
    const int pid = inputData.value("requestPid").toInt(-1);

    if (n < 1 || n > 20 ||
        m < 1 || m > 10 ||
        algorithmMode < 0 || algorithmMode > 1 ||
        pid < 0 || pid >= n) {
        return false;
    }

    if (!validMatrix(inputData.value("allocation"), n, m) ||
        !validMatrix(inputData.value("second"), n, m) ||
        !validMatrix(inputData.value("available"), 1, m) ||
        !validMatrix(inputData.value("requestTable"), 1, m)) {
        return false;
    }

    if (!outputData.contains("statusText") ||
        !outputData.value("statusText").isString() ||
        !outputData.contains("explanationText") ||
        !outputData.value("explanationText").isString()) {
        return false;
    }

    {
        QSignalBlocker modeBlocker(mode);
        QSignalBlocker processBlocker(processCount);
        QSignalBlocker resourceBlocker(resourceCount);

        processCount->setValue(n);
        resourceCount->setValue(m);
    }

    resizeTables();

    mode->setCurrentIndex(algorithmMode);

    restoreMatrix(
        allocation,
        inputData.value("allocation").toArray()
    );

    restoreMatrix(
        second,
        inputData.value("second").toArray()
    );

    restoreVector(
        available,
        inputData.value("available")
            .toArray().at(0).toArray()
    );

    restoreVector(
        requestTable,
        inputData.value("requestTable")
            .toArray().at(0).toArray()
    );

    requestPid->setValue(pid);

    status->setText(
        outputData.value("statusText").toString()
    );

    explanation->setPlainText(
        outputData.value("explanationText").toString()
    );

    lastInputData = inputData;
    lastOutputData = outputData;

    hasResult = true;
    saveButton->setEnabled(true);

    return true;
}
