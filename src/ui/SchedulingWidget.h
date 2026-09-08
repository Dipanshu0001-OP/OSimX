#ifndef SCHEDULING_WIDGET_H
#define SCHEDULING_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QVector>
#include <QString>
#include <QDebug>
#include <QMessageBox>

#include "modules/scheduling/CPUScheduler.h"
#include "GanttWidget.h"

using namespace std;

class SchedulingWidget : public QWidget {
    Q_OBJECT

public:
    explicit SchedulingWidget(QWidget *parent = nullptr);
    ~SchedulingWidget();

    SchedulingResult getLastResult() const { return lastResult; }
    vector<Process> getProcesses() const { return processes; }
    QString getAlgorithmName() const { return algorithmCombo->currentText(); }
    int getQuantum() const { return quantumSpinBox->value(); }
    void setUserId(int id) { currentUserId = id; }

signals:
    void simulationRun();
    void saveRequested();

private slots:
    void addProcess();
    void clearProcesses();
    void runSimulation();
    void onAlgorithmChange(const QString& algo);

private:
    QTableWidget* processTable;
    QComboBox* algorithmCombo;
    QSpinBox* quantumSpinBox;
    QPushButton* runButton;
    QPushButton* addButton;
    QPushButton* clearButton;
    QPushButton* saveButton;
    QLabel* metricsLabel;
    GanttWidget* ganttWidget;
    
    vector<Process> processes;
    SchedulingResult lastResult;
    int currentUserId;
    
    void setupUI();
    void setupProcessTable();
    void updateMetrics();
};

#endif