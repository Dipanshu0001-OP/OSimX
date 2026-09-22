#ifndef OSIMX_DISK_TRACK_WIDGET_H
#define OSIMX_DISK_TRACK_WIDGET_H

#include <QWidget>
#include <vector>
#include "models/DiskResult.h"

class DiskTrackWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiskTrackWidget(QWidget* parent = nullptr);
    void setResult(const DiskResult& result, int maxCylinder, int head, const std::vector<int>& requests);
    void setCurrentStep(int step);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    DiskResult m_result;
    int m_maxCylinder = 199;
    int m_head = 0;
    int m_currentStep = -1;
    std::vector<int> m_requests;
};

#endif
