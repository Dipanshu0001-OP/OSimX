#ifndef OSIMX_SCHEDULING_WINDOW_H
#define OSIMX_SCHEDULING_WINDOW_H

#include <QLabel>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QWidget>
#include <QJsonObject>
#include "models/SchedulingResult.h"
#include "ui/common/GanttWidget.h"
#include "ui/common/PlaybackBar.h"
#include "ui/common/SimulationLogWidget.h"

class SchedulingWindow : public QWidget {
    Q_OBJECT
public:
    explicit SchedulingWindow(QWidget* parent = nullptr);
    bool restoreSimulation(const QJsonObject& saved);

signals:
    void backRequested();
    void simulationSaved();

private slots:
    void addProcess();
    void removeSelectedProcesses();
    void clearProcesses();
    void runSimulation();
    void onAlgorithmChanged(const QString& algorithm);
    void onPlaybackIndex(int index);
    void saveSimulation();
    void compareAlgorithms();

private:
    QTableWidget* processTable = nullptr;
    QComboBox* algorithmCombo = nullptr;
    QSpinBox* quantumSpinBox = nullptr;
    QPushButton* runButton = nullptr;
    QPushButton* addButton = nullptr;
    QPushButton* removeButton = nullptr;
    QPushButton* clearButton = nullptr;
    QLabel* algorithmInfo = nullptr;
    QLabel* currentStepLabel = nullptr;
    QLabel* metricsLabel = nullptr;
    QLabel* statusLabel = nullptr;
    GanttWidget* ganttWidget = nullptr;
    PlaybackBar* playback = nullptr;
    SimulationLogWidget* logs = nullptr;
    QTableWidget* resultsTable = nullptr;
    QPushButton* saveButton = nullptr;
    QTableWidget* comparisonTable = nullptr;
    QLabel* comparisonInsight = nullptr;

    QString lastRunAlgorithm;
    int lastRunQuantum = 1;
    SchedulingResult result;
    QStringList resultLogs;

    void setupUI();
    void setupProcessTable();
    void updateAlgorithmInfo();
    void updateMetrics();
    void updatePlaybackState(int index);
    void populateResultTable();
    QStringList buildLogs() const;
    QString algorithmKey() const;
    bool readProcesses(std::vector<Process>& processes);
};

#endif
