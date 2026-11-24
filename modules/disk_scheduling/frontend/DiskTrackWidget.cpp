#include "DiskTrackWidget.h"
#include <QPainter>
#include <QFont>
#include <QPainterPath>
#include <QPaintEvent>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

DiskTrackWidget::DiskTrackWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(760, 390);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor("#f7f9fc"));

    p.setPen(QColor("#172033"));
    p.setFont(QFont("Segoe UI", 11, QFont::DemiBold));
    p.drawText(QRect(22, 12, width()-44, 26), Qt::AlignLeft | Qt::AlignVCenter,
               "Disk Head Movement Graph");
    p.setPen(QColor("#667085"));
    p.setFont(QFont("Segoe UI", 9));
    p.drawText(QRect(22, 39, width()-44, 22), Qt::AlignLeft | Qt::AlignVCenter,
               "Horizontal: cylinder number   |   Vertical: movement step (top to bottom)");

    const QRectF plot(75, 94, qMax(1, width()-112), qMax(1, height()-144));
    const int total = static_cast<int>(m_result.steps.size());
    const int rowCount = qMax(1, total);
    const auto x = [&](int cylinder) -> qreal {
        return plot.left() + qBound(0, cylinder, m_maxCylinder) * plot.width() / m_maxCylinder;
    };
    const auto y = [&](int index) -> qreal {
        return plot.top() + index * plot.height() / rowCount;
    };

    // Cylinder guides and labels.
    p.setFont(QFont("Segoe UI", 8));
    for (int i = 0; i <= 4; ++i) {
        const int cylinder = static_cast<int>(std::lround(m_maxCylinder * i / 4.0));
        const qreal px = x(cylinder);
        p.setPen(QPen(QColor("#dde4ee"), 1, Qt::DashLine));
        p.drawLine(QPointF(px, plot.top()), QPointF(px, plot.bottom()));
        p.setPen(QColor("#596579"));
        p.drawText(QRectF(px-28, plot.top()-24, 56, 18), Qt::AlignCenter, QString::number(cylinder));
    }

    // Horizontal guides indicate the order in which the head moves.
    const int guideInterval = qMax(1, (rowCount+9)/10);
    for (int i = 0; i <= rowCount; i += guideInterval) {
        const qreal py = y(i);
        p.setPen(QPen(QColor("#e8edf4"), 1));
        p.drawLine(QPointF(plot.left(), py), QPointF(plot.right(), py));
        p.setPen(QColor("#667085"));
        p.drawText(QRectF(10, py-9, 52, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(i));
    }

    // Faint preview of the entire calculated route.
    QPointF previous(x(m_head), y(0));
    p.setPen(QPen(QColor("#cbd5e1"), 1.5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < total; ++i) {
        const QPointF next(x(m_result.steps[i].to), y(i+1));
        p.drawLine(previous, next);
        previous = next;
    }

    // Playback reveals the route one movement at a time. Step 0 is the first movement,
    // matching the previous widget's playback behavior.
    const int visible = m_currentStep < 0 ? 0 : qMin(total, m_currentStep+1);
    previous = QPointF(x(m_head), y(0));
    for (int i = 0; i < visible; ++i) {
        const DiskStep& step = m_result.steps[i];
        const QPointF next(x(step.to), y(i+1));
        const QColor segmentColor = step.circularReturn ? QColor("#e59b36") : QColor("#159a74");
        p.setPen(QPen(segmentColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(previous, next);
        p.setPen(Qt::NoPen);
        p.setBrush(segmentColor);
        p.drawEllipse(next, 4.5, 4.5);
        previous = next;
    }

    // Start marker and current head marker.
    const QPointF start(x(m_head), y(0));
    p.setPen(QPen(QColor("#ffffff"), 2));
    p.setBrush(QColor("#2563eb"));
    p.drawEllipse(start, 6, 6);
    const int currentCylinder = visible > 0 ? m_result.steps[visible-1].to : m_head;
    const QPointF current(x(currentCylinder), y(visible));
    p.setPen(QPen(QColor("#ffffff"), 2));
    p.setBrush(QColor("#0f766e"));
    p.drawEllipse(current, 8, 8);

    // Keep head-position label within the plot's horizontal bounds.
    p.setPen(QColor("#0f766e"));
    p.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    const qreal labelX = qBound(plot.left(), current.x()-42, plot.right()-84);
    const qreal labelY = current.y()+11 < plot.bottom()-16 ? current.y()+10 : current.y()-26;
    p.drawText(QRectF(labelX, labelY, 84, 18), Qt::AlignCenter,
               QString("Head: %1").arg(currentCylinder));

    p.setPen(QColor("#596579"));
    p.setFont(QFont("Segoe UI", 9));
    p.drawText(QRectF(plot.left(), height()-35, plot.width(), 20), Qt::AlignCenter,
               QString("Cylinder position     |     Movements shown: %1 / %2     |     Total seek: %3")
                   .arg(visible).arg(total).arg(m_result.totalSeek));
}
