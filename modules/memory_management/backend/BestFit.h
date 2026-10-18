#ifndef OSIMX_BEST_FIT_H
#define OSIMX_BEST_FIT_H
#include "MemoryAllocator.h"
class BestFit : public MemoryAllocator {
public:
    AllocationResult allocate(std::vector<int> blocks,
                              std::vector<MemoryProcess> processes) override;
    std::string getAlgorithmName() const override;
};
#endif
