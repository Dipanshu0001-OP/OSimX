#ifndef OSIMX_MEMORY_BLOCK_WIDGET_H
#define OSIMX_MEMORY_BLOCK_WIDGET_H

#include <QWidget>
#include "models/MemoryResult.h"

// Draws the memory blocks as a row of bars. Each bar's width is
// proportional to the block's size. The used portion and the free
// portion of a block are shaded differently, and the block chosen
// on the current step is outlined.
class MemoryBlockWidget : public QWidget {
    Q_OBJECT
public:
    explicit MemoryBlockWidget(QWidget* parent = nullptr);
    void setResult(const AllocationResult& result);
    void setCurrentStep(int step);
    void clear();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    AllocationResult m_result;
    int m_currentStep = -1;
};

#endif
