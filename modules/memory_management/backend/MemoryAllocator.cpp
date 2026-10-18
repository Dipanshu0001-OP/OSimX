#include "MemoryAllocator.h"

// Fills the summary numbers after all the steps are done.
void MemoryAllocator::calculateValues(AllocationResult& result)
{
    result.allocatedCount = 0;
    result.failedCount = 0;

    for (int i = 0; i < (int)result.steps.size(); i++) {
        if (result.steps[i].blockIndex == -1) {
            result.failedCount++;
        } else {
            result.allocatedCount++;
        }
    }

    // memory that is left after the last step
    std::vector<int> leftover = result.initialBlocks;
    if (!result.steps.empty()) {
        leftover = result.steps.back().blocksAfter;
    }

    int total = 0;
    int largest = 0;
    for (int i = 0; i < (int)leftover.size(); i++) {
        total += leftover[i];
        if (leftover[i] > largest) {
            largest = leftover[i];
        }
    }

    result.totalFreeMemory = total;
    result.largestFreeBlock = largest;
    // free memory that is scattered in small holes instead of one usable block
    result.externalFragmentation = total - largest;
}
