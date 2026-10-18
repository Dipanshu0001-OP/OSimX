#ifndef OSIMX_OPTIMAL_H
#define OSIMX_OPTIMAL_H
#include "PageReplacer.h"
class Optimal : public PageReplacer {
public:
    PageReplacementResult replace(std::vector<int> referenceString, int frameCount) override;
    std::string getAlgorithmName() const override;
};
#endif
