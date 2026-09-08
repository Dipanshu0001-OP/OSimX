#include "GanttWidget.h"
#include <QDebug>

GanttWidget::GanttWidget(QWidget *parent)
    : QWidget(parent), hasData(false) {
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: white; border: 1px solid #ccc; border-radius: 5px;");
}

void GanttWidget::setGanttData(const vector<GanttChart>& data) {
    ganttData = data;
    hasData = !data.empty();
    update();  // Trigger repaint
}

void GanttWidget::clear() {
    ganttData.clear();
    hasData = false;
    update();
}

void GanttWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = this->width() - 20;
    int h = this->height() - 20;
    int y = 10;
    int barHeight = 40;
    
    if (!hasData || ganttData.empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(10, 50, "Run a simulation to see Gantt chart");
        return;
    }
    
    qDebug() << "Drawing Gantt chart with" << ganttData.size() << "entries";
    
    // Calculate total time
    int totalTime = 0;
    for (const auto& g : ganttData) {
        if (g.endTime > totalTime) totalTime = g.endTime;
    }
    if (totalTime == 0) totalTime = 1;
    
    QVector<QColor> colors = {
        QColor(52, 152, 219),   // Blue
        QColor(46, 204, 113),   // Green
        QColor(231, 76, 60),    // Red
        QColor(241, 196, 15),   // Yellow
        QColor(155, 89, 182),   // Purple
        QColor(26, 188, 156)    // Teal
    };
    
    // Draw bars
    for (int i = 0; i < (int)ganttData.size(); i++) {
        const auto& g = ganttData[i];
        int x = 10 + (g.startTime * w) / totalTime;
        int barWidth = ((g.endTime - g.startTime) * w) / totalTime;
        if (barWidth < 2) barWidth = 2;
        
        QColor color = colors[g.pid % colors.size()];
        painter.fillRect(x, y, barWidth, barHeight, color);
        painter.setPen(Qt::black);
        painter.drawRect(x, y, barWidth, barHeight);
        
        if (barWidth > 15) {
            painter.setPen(Qt::white);
            painter.drawText(x + 5, y + barHeight - 10, 
                            "P" + QString::number(g.pid));
        }
    }
    
    // Time axis
    painter.setPen(Qt::black);
    for (int t = 0; t <= totalTime; t += 1) {
        int x = 10 + (t * w) / totalTime;
        painter.drawLine(x, y + barHeight + 5, x, y + barHeight + 10);
        painter.drawText(x - 8, y + barHeight + 25, QString::number(t));
    }
    
    qDebug() << "Gantt chart drawn successfully!";
}
