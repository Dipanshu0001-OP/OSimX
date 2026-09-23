#ifndef OSIMX_DISK_SCHEDULING_WINDOW_H
#define OSIMX_DISK_SCHEDULING_WINDOW_H

#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>
#include <QPushButton>
#include "models/DiskResult.h"
#include "ui/common/PlaybackBar.h"
#include "ui/common/SimulationLogWidget.h"
#include "DiskTrackWidget.h"

class DiskSchedulingWindow : public QWidget {
    Q_OBJECT
public:
    explicit DiskSchedulingWindow(QWidget* parent = nullptr);
signals:
    void backRequested();
private slots:
    void addRequest();
    void removeSelectedRequests();
    void clearRequests();
    void runSimulation();
    void onPlaybackIndex(int index);
    void onMaxCylinderChanged(int value);
    void saveSimulation();
private:
    QTableWidget* requestsTable = nullptr;
    QComboBox* algorithmCombo = nullptr;
    QSpinBox* headSpinBox = nullptr;
    QSpinBox* maxCylinderSpinBox = nullptr;
    QComboBox* directionCombo = nullptr;
    QLabel* algorithmInfo = nullptr;
    QLabel* stepLabel = nullptr;
    QLabel* statusLabel = nullptr;
    QLabel* metricsLabel = nullptr;
    QPushButton* saveButton = nullptr;
    PlaybackBar* playback = nullptr;
    SimulationLogWidget* logs = nullptr;
    DiskTrackWidget* track = nullptr;
    DiskResult result;
    QStringList resultLogs;

    void setupUI();
    void setupRequestTable();
    void updateAlgorithmInfo();
    void updateView(int index);
    QStringList buildLogs() const;
};

#endif
