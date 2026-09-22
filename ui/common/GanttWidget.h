#ifndef OSIMX_GANTT_WIDGET_H
#define OSIMX_GANTT_WIDGET_H

#include <QWidget>
#include <vector>
#include "models/SchedulingResult.h"

class GanttWidget : public QWidget {
    Q_OBJECT
public:
    explicit GanttWidget(QWidget* parent = nullptr);
    void setGanttData(const std::vector<GanttChart>& data);
    void clear();
    void setCurrentStep(int step);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    std::vector<GanttChart> ganttData;
    int currentStep = -1;
};

#endif
