#ifndef OSIMX_WORST_FIT_H
#define OSIMX_WORST_FIT_H
#include "MemoryAllocator.h"
class WorstFit : public MemoryAllocator {
public:
    AllocationResult allocate(std::vector<int> blocks,
                              std::vector<MemoryProcess> processes) override;
    std::string getAlgorithmName() const override;
};
#endif
