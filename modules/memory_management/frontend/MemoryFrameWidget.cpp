#include "MemoryFrameWidget.h"
#include <QPainter>
#include <QFont>
#include <algorithm>

MemoryFrameWidget::MemoryFrameWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(900, 220);
}

void MemoryFrameWidget::setResult(const PageReplacementResult& result)
{
    m_result = result;
    m_currentStep = result.steps.empty() ? -1 : 0;
    update();
}

void MemoryFrameWidget::setCurrentStep(int step)
{
    m_currentStep = step;
    update();
}

void MemoryFrameWidget::clear()
{
    m_result = PageReplacementResult();
    m_currentStep = -1;
    update();
}

void MemoryFrameWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f7f9fc"));

    painter.setPen(QColor("#667085"));
    painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    painter.drawText(16, 26, "Page frames");
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(16, 42, "Green outline is a hit. Orange outline is a fault. The evicted page, if any, is named below.");

    if (m_result.frameCount <= 0 || m_currentStep < 0 || m_currentStep >= (int)m_result.steps.size()) {
        painter.setPen(QColor("#9aa4b2"));
        painter.drawText(rect(), Qt::AlignCenter, "Run a simulation to see the page frames.");
        return;
    }

    const auto& step = m_result.steps[m_currentStep];
    const int frameCount = m_result.frameCount;

    const int area_left = 16;
    const int area_right = width() - 16;
    const int area_top = 70;
    const int boxSize = 70;
    const int gap = 12;
    const int totalWidth = frameCount * boxSize + (frameCount - 1) * gap;
    int x = std::max(area_left, area_left + (area_right - area_left - totalWidth) / 2);

    painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    painter.setPen(step.hit ? QColor("#17864a") : QColor("#b3261e"));
    painter.drawText(area_left, 56,
                      QString("Reference: page %1  —  %2").arg(step.page).arg(step.hit ? "HIT" : "FAULT"));

    for (int f = 0; f < frameCount; f++) {
        QRect box(x, area_top, boxSize, boxSize);
        const bool holdsCurrent = (f < (int)step.frames.size()) && (step.frames[f] == step.page);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(f < (int)step.frames.size() && step.frames[f] != -1 ? "#dfe4ec" : "#eef2f6"));
        painter.drawRect(box);

        if (holdsCurrent) {
            painter.setPen(QPen(step.hit ? QColor("#2aa56a") : QColor("#e08a00"), 3));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(box);
        } else {
            painter.setPen(QPen(QColor("#c3cad6"), 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(box);
        }

        painter.setPen(QColor("#172033"));
        painter.setFont(QFont("Segoe UI", 13, QFont::DemiBold));
        const QString label = (f < (int)step.frames.size() && step.frames[f] != -1)
            ? QString::number(step.frames[f])
            : "—";
        painter.drawText(box, Qt::AlignCenter, label);

        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor("#9aa4b2"));
        painter.drawText(box.adjusted(0, boxSize + 4, 0, 0), Qt::AlignHCenter | Qt::AlignTop, QString("frame %1").arg(f));

        x += boxSize + gap;
    }

    if (step.victim != -1) {
        painter.setPen(QColor("#9aa4b2"));
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(area_left, area_top + boxSize + 42, QString("Page %1 was evicted to make room.").arg(step.victim));
    }
}
