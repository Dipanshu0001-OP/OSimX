#ifndef GANTT_WIDGET_H
#define GANTT_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QColor>
#include "modules/scheduling/CPUScheduler.h"

using namespace std;

class GanttWidget : public QWidget {
    Q_OBJECT

public:
    explicit GanttWidget(QWidget *parent = nullptr);
    void setGanttData(const vector<GanttChart>& data);
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    vector<GanttChart> ganttData;
    bool hasData = false;
};

#endif