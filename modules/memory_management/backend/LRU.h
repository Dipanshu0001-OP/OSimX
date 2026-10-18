#ifndef OSIMX_LRU_H
#define OSIMX_LRU_H
#include "PageReplacer.h"
class LRU : public PageReplacer {
public:
    PageReplacementResult replace(std::vector<int> referenceString, int frameCount) override;
    std::string getAlgorithmName() const override;
};
#endif
