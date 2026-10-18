#ifndef OSIMX_MEMORY_ALLOCATOR_H
#define OSIMX_MEMORY_ALLOCATOR_H

#include <string>
#include <vector>
#include "models/MemoryResult.h"

class MemoryAllocator {
public:
    virtual ~MemoryAllocator() = default;
    virtual AllocationResult allocate(std::vector<int> blocks,
                                      std::vector<MemoryProcess> processes) = 0;
    virtual std::string getAlgorithmName() const = 0;

protected:
    void calculateValues(AllocationResult& result);
};

#endif
