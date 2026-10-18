#ifndef OSIMX_MEMORY_WINDOW_H
#define OSIMX_MEMORY_WINDOW_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>
#include "models/MemoryResult.h"
#include "ui/common/PlaybackBar.h"
#include "ui/common/SimulationLogWidget.h"
#include "MemoryBlockWidget.h"
#include "MemoryFrameWidget.h"

// Memory Management has two algorithm families (README section 4), so
// this window holds two tabs: Allocation (First/Best/Worst Fit) and
// Page Replacement (FIFO/LRU/Optimal). Each tab follows the same
// input -> run -> replayable visualization -> log -> results -> save
// pattern as SchedulingWindow and DiskSchedulingWindow.
class MemoryWindow : public QWidget {
    Q_OBJECT
public:
    explicit MemoryWindow(QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    // allocation tab
    void addProcessRow();
    void removeSelectedProcessRows();
    void clearProcessRows();
    void runAllocation();
    void onAllocationAlgorithmChanged(const QString& algorithm);
    void onAllocationPlaybackIndex(int index);
    void saveAllocationSimulation();

    // page replacement tab
    void runPageReplacement();
    void onPageAlgorithmChanged(const QString& algorithm);
    void onPagePlaybackIndex(int index);
    void savePageSimulation();

private:
    // ----- allocation tab widgets -----
    QTableWidget* processTable = nullptr;
    QPushButton* addProcessButton = nullptr;
    QPushButton* removeProcessButton = nullptr;
    QPushButton* clearProcessButton = nullptr;
    QLineEdit* blockSizesEdit = nullptr;
    QComboBox* allocationAlgorithmCombo = nullptr;
    QLabel* allocationAlgorithmInfo = nullptr;
    QPushButton* runAllocationButton = nullptr;
    MemoryBlockWidget* blockWidget = nullptr;
    PlaybackBar* allocationPlayback = nullptr;
    QLabel* allocationStepLabel = nullptr;
    QLabel* allocationStatusLabel = nullptr;
    QLabel* allocationMetricsLabel = nullptr;
    SimulationLogWidget* allocationLogs = nullptr;
    QTableWidget* allocationResultsTable = nullptr;
    QPushButton* saveAllocationButton = nullptr;

    AllocationResult allocationResult;
    QStringList allocationResultLogs;

    // ----- page replacement tab widgets -----
    QLineEdit* referenceStringEdit = nullptr;
    QSpinBox* frameCountSpinBox = nullptr;
    QComboBox* pageAlgorithmCombo = nullptr;
    QLabel* pageAlgorithmInfo = nullptr;
    QPushButton* runPageButton = nullptr;
    MemoryFrameWidget* frameWidget = nullptr;
    PlaybackBar* pagePlayback = nullptr;
    QLabel* pageStepLabel = nullptr;
    QLabel* pageStatusLabel = nullptr;
    QLabel* pageMetricsLabel = nullptr;
    SimulationLogWidget* pageLogs = nullptr;
    QTableWidget* pageResultsTable = nullptr;
    QPushButton* savePageButton = nullptr;

    PageReplacementResult pageResult;
    QStringList pageResultLogs;

    void setupUI();
    QWidget* buildAllocationTab();
    QWidget* buildPageReplacementTab();
    void setupProcessTable();

    void updateAllocationAlgorithmInfo();
    void updateAllocationMetrics();
    void updateAllocationPlaybackState(int index);
    void populateAllocationResultTable();
    QStringList buildAllocationLogs() const;

    void updatePageAlgorithmInfo();
    void updatePageMetrics();
    void updatePagePlaybackState(int index);
    void populatePageResultTable();
    QStringList buildPageLogs() const;
};

#endif
