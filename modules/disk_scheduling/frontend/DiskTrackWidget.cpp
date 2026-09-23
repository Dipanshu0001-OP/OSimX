#include "DiskTrackWidget.h"
#include <QPainter>
#include <QFont>

DiskTrackWidget::DiskTrackWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(900, 260);
}

void DiskTrackWidget::setResult(const DiskResult& result, int maxCylinder, int head, const std::vector<int>& requests)
{
    m_result = result;
    m_requests = requests;
    m_maxCylinder = qMax(1, maxCylinder);
    m_head = qBound(0, head, m_maxCylinder);
    m_currentStep = result.steps.empty() ? -1 : 0;
    update();
}

void DiskTrackWidget::setCurrentStep(int step)
{
    m_currentStep = step;
    update();
}

void DiskTrackWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f7f9fc"));

    const QRect area = rect().adjusted(38, 55, -38, -70);
    const int y = area.center().y();
    const int span = qMax(1, m_maxCylinder);
    auto x = [&](int cylinder) {
        return area.left() + cylinder * area.width() / span;
    };

    painter.setPen(QColor("#667085"));
    painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    painter.drawText(area.left(), 30, "Disk head path");
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(area.left(), 46, "Dots are requests. The line shows the movement revealed by playback.");

    painter.setPen(QPen(QColor("#ccd3df"), 6));
    painter.drawLine(area.left(), y, area.right(), y);

    for (int marker : {0, span / 2, span}) {
        const int px = x(marker);
        painter.setPen(QColor("#7b8496"));
        painter.drawLine(px, y - 8, px, y + 8);
        painter.drawText(px - 15, y + 36, QString::number(marker));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#172033"));
    for (int request : m_requests)
        painter.drawEllipse(QPointF(x(request), y), 5, 5);

    if (!m_result.steps.empty() && m_currentStep >= 0) {
        const int limit = qBound(0, m_currentStep, static_cast<int>(m_result.steps.size()) - 1);
        int from = m_head;
        painter.setPen(QPen(QColor("#2aa56a"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        for (int i = 0; i <= limit; ++i) {
            const int to = m_result.steps[i].to;
            painter.drawLine(QPointF(x(from), y), QPointF(x(to), y));
            from = to;
        }

        const int current = m_result.steps[limit].to;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#2aa56a"));
        painter.drawEllipse(QPointF(x(current), y), 9, 9);
        painter.setPen(QColor("#2aa56a"));
        painter.drawText(x(current) - 26, y - 18, QString("head %1").arg(current));
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#2aa56a"));
        painter.drawEllipse(QPointF(x(m_head), y), 9, 9);
        painter.setPen(QColor("#2aa56a"));
        painter.drawText(x(m_head) - 26, y - 18, QString("head %1").arg(m_head));
    }
}
