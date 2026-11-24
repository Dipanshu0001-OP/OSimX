
#ifndef OSIMX_DEADLOCK_WINDOW_H
#define OSIMX_DEADLOCK_WINDOW_H

#include <QJsonObject>
#include <QWidget>

#include "modules/deadlock/backend/DeadlockDetector.h"

class QSpinBox;
class QTableWidget;
class QComboBox;
class QLabel;
class QTextEdit;
class QPushButton;

class DeadlockWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DeadlockWindow(QWidget* parent = nullptr);

    // Restore a saved simulation without rerunning it.
    bool restoreSimulation(
        const QJsonObject& inputData,
        const QJsonObject& outputData
    );

signals:
    void backRequested();

    // Allows MainWindow to refresh Saved Simulations.
    void simulationSaved();

private:
    QSpinBox* processCount = nullptr;
    QSpinBox* resourceCount = nullptr;
    QSpinBox* requestPid = nullptr;

    QComboBox* mode = nullptr;

    QTableWidget* allocation = nullptr;
    QTableWidget* second = nullptr;
    QTableWidget* available = nullptr;
    QTableWidget* requestTable = nullptr;

    QLabel* status = nullptr;
    QTextEdit* explanation = nullptr;

    QPushButton* saveButton = nullptr;

    // Tracks the most recent completed analysis.
    bool hasResult = false;

    QJsonObject lastInputData;
    QJsonObject lastOutputData;

    void resizeTables();
    void loadExample();

    void runSimulation();
    void evaluateRequest();

    void saveSimulation();

    bool readInputs(
        deadlock::Matrix& allocationMatrix,
        deadlock::Matrix& secondMatrix,
        std::vector<int>& availableVector
    );

    bool readTable(
        QTableWidget* table,
        int row,
        int column,
        int& value
    );

    void showResult(
        const deadlock::Result& result,
        const QString& prefix
    );

    QJsonObject captureInputs() const;

    QJsonObject captureResult(
        const deadlock::Result& result,
        const QString& operation,
        const QString& message = QString()
    ) const;

    void invalidateSavedResult();
};

#endif
