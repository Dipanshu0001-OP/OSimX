#include "MemoryBlockWidget.h"
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <numeric>

MemoryBlockWidget::MemoryBlockWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(900, 220);
}

void MemoryBlockWidget::setResult(const AllocationResult& result)
{
    m_result = result;
    m_currentStep = result.steps.empty() ? -1 : 0;
    update();
}

void MemoryBlockWidget::setCurrentStep(int step)
{
    m_currentStep = step;
    update();
}

void MemoryBlockWidget::clear()
{
    m_result = AllocationResult();
    m_currentStep = -1;
    update();
}

void MemoryBlockWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f7f9fc"));

    painter.setPen(QColor("#667085"));
    painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    painter.drawText(16, 26, "Memory blocks");
    painter.setFont(QFont("Segoe UI", 9));
    painter.drawText(16, 42, "Dark fill is used memory. Light fill is still free. The block touched this step has a green outline.");

    if (m_result.initialBlocks.empty()) {
        painter.setPen(QColor("#9aa4b2"));
        painter.drawText(rect(), Qt::AlignCenter, "Run a simulation to see the memory blocks.");
        return;
    }

    const std::vector<int>& freeNow = (m_currentStep >= 0 && m_currentStep < (int)m_result.steps.size())
        ? m_result.steps[m_currentStep].blocksAfter
        : m_result.initialBlocks;

    const int total = std::accumulate(m_result.initialBlocks.begin(), m_result.initialBlocks.end(), 0);
    const int area_left = 16;
    const int area_right = width() - 16;
    const int area_top = 64;
    const int barHeight = 70;
    const int gap = 6;
    const int usableWidth = std::max(1, area_right - area_left - gap * ((int)m_result.initialBlocks.size() - 1));

    const int touchedBlock = (m_currentStep >= 0 && m_currentStep < (int)m_result.steps.size())
        ? m_result.steps[m_currentStep].blockIndex
        : -1;

    int x = area_left;
    for (int i = 0; i < (int)m_result.initialBlocks.size(); i++) {
        const int blockSize = m_result.initialBlocks[i];
        const int freeSize = (i < (int)freeNow.size()) ? freeNow[i] : blockSize;
        const int usedSize = blockSize - freeSize;

        const int blockWidth = total > 0
            ? std::max(40, blockSize * usableWidth / total)
            : usableWidth / (int)m_result.initialBlocks.size();

        QRect blockRect(x, area_top, blockWidth, barHeight);
        const int usedWidth = blockSize > 0 ? blockWidth * usedSize / blockSize : 0;

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#dfe4ec"));
        painter.drawRect(blockRect);

        if (usedWidth > 0) {
            QRect usedRect(x, area_top, usedWidth, barHeight);
            painter.setBrush(QColor("#2aa56a"));
            painter.drawRect(usedRect);
        }

        painter.setPen(QPen(i == touchedBlock ? QColor("#17864a") : QColor("#9aa4b2"),
                             i == touchedBlock ? 3 : 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(blockRect);

        painter.setPen(QColor("#172033"));
        painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        painter.drawText(blockRect.adjusted(4, barHeight + 4, 0, 0), Qt::AlignLeft, QString("Block %1").arg(i));
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(blockRect.adjusted(4, barHeight + 20, 0, 0), Qt::AlignLeft,
                          QString("%1 free / %2").arg(freeSize).arg(blockSize));

        x += blockWidth + gap;
    }

    if (touchedBlock == -1 && m_currentStep >= 0 && m_currentStep < (int)m_result.steps.size()) {
        const auto& step = m_result.steps[m_currentStep];
        painter.setPen(QColor("#b3261e"));
        painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        painter.drawText(area_left, area_top + barHeight + 50,
                          QString("P%1 (size %2) did not fit in any block.").arg(step.pid).arg(step.size));
    }
}
