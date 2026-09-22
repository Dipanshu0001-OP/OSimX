#include "GanttWidget.h"

#include <QPainter>
#include <QVector>
#include <QFont>
#include <cstdlib>

GanttWidget::GanttWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(250);
    setMinimumWidth(900);
}

void GanttWidget::setGanttData(const std::vector<GanttChart>& data)
{
    ganttData = data;
    currentStep = ganttData.empty() ? -1 : 0;

    int total = 0;
    for (const auto& segment : ganttData)
        total = qMax(total, segment.endTime);

    setMinimumWidth(qMax(900, 80 * qMax(1, total)));
    update();
}

void GanttWidget::clear()
{
    ganttData.clear();
    currentStep = -1;
    setMinimumWidth(900);
    update();
}

void GanttWidget::setCurrentStep(int step)
{
    currentStep = step;
    update();
}

void GanttWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f7f9fc"));

    if (ganttData.empty()) {
        painter.setPen(QColor("#7b8496"));
        painter.setFont(QFont("Segoe UI", 10));
        painter.drawText(rect(), Qt::AlignCenter, "Run a simulation to reveal the CPU timeline");
        return;
    }

    const int left = 36;
    const int right = 36;
    const int titleY = 26;
    const int subtitleY = 46;
    const int stepY = 67;
    const int top = 88;
    const int barHeight = 72;
    const int usable = qMax(200, width() - left - right);

    int total = 1;
    for (const auto& segment : ganttData)
        total = qMax(total, segment.endTime);

    const QVector<QColor> colors = {
        QColor("#2aa56a"), QColor("#4578d4"), QColor("#da7a2d"),
        QColor("#8a64c7"), QColor("#d45f85"), QColor("#219a9a")
    };

    painter.setPen(QColor("#344054"));
    painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    painter.drawText(left, titleY, "Execution timeline");

    painter.setPen(QColor("#667085"));
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(left, subtitleY, "Future decisions stay hidden until playback reaches them.");

    if (currentStep >= 0 && currentStep < static_cast<int>(ganttData.size())) {
        const auto& current = ganttData[currentStep];
        painter.setPen(QColor("#172033"));
        painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        painter.drawText(left, stepY,
                         QString("Step %1 of %2  ·  t=%3 → %4")
                             .arg(currentStep + 1)
                             .arg(ganttData.size())
                             .arg(current.startTime)
                             .arg(current.endTime));
    }

    for (int i = 0; i < static_cast<int>(ganttData.size()); ++i) {
        const auto& segment = ganttData[i];
        const int x1 = left + segment.startTime * usable / total;
        const int x2 = left + segment.endTime * usable / total;
        const int blockWidth = qMax(8, x2 - x1);

        if (i > currentStep) {
            painter.setPen(QPen(QColor("#d9dee6"), 1, Qt::DashLine));
            painter.setBrush(QColor("#f1f3f6"));
            painter.drawRoundedRect(QRect(x1, top, blockWidth, barHeight), 12, 12);
            continue;
        }

        QColor fill = segment.pid < 0
            ? QColor("#98a2b3")
            : colors[std::abs(segment.pid) % colors.size()];

        painter.setPen(Qt::NoPen);
        painter.setBrush(fill);
        painter.drawRoundedRect(QRect(x1, top, blockWidth, barHeight), 12, 12);

        if (blockWidth > 36) {
            painter.setPen(Qt::white);
            painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
            painter.drawText(QRect(x1, top, blockWidth, barHeight), Qt::AlignCenter,
                             segment.pid < 0 ? "IDLE" : QString("P%1").arg(segment.pid));
        }
    }

    painter.setPen(QColor("#a0a7b5"));
    painter.setFont(QFont("Segoe UI", 8));
    for (int t = 0; t <= total; ++t) {
        const int x = left + t * usable / total;
        painter.drawLine(x, top + barHeight + 8, x, top + barHeight + 18);
        painter.drawText(x - 10, top + barHeight + 38, QString::number(t));
    }

    if (currentStep >= 0 && currentStep < static_cast<int>(ganttData.size())) {
        const auto& current = ganttData[currentStep];
        const int x = left + current.startTime * usable / total;
        painter.setPen(QPen(QColor("#172033"), 2));
        painter.drawLine(x, top - 8, x, top + barHeight + 12);
    }
}
