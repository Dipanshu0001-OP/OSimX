#ifndef OSIMX_MEMORY_FRAME_WIDGET_H
#define OSIMX_MEMORY_FRAME_WIDGET_H

#include <QWidget>
#include "models/MemoryResult.h"

// Draws the page frames as a row of boxes. The current page reference
// is shown above the frames, a hit is outlined green, a fault is
// outlined orange, and the evicted page (if any) is named below.
class MemoryFrameWidget : public QWidget {
    Q_OBJECT
public:
    explicit MemoryFrameWidget(QWidget* parent = nullptr);
    void setResult(const PageReplacementResult& result);
    void setCurrentStep(int step);
    void clear();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    PageReplacementResult m_result;
    int m_currentStep = -1;
};

#endif
