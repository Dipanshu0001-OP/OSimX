#ifndef OSIMX_FIFO_H
#define OSIMX_FIFO_H
#include "PageReplacer.h"
class FIFO : public PageReplacer {
public:
    PageReplacementResult replace(std::vector<int> referenceString, int frameCount) override;
    std::string getAlgorithmName() const override;
};
#endif
