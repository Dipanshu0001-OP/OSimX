#include "BestFit.h"

// Best Fit: check all blocks and pick the SMALLEST one that still fits.
AllocationResult BestFit::allocate(std::vector<int> blocks,
                                   std::vector<MemoryProcess> processes)
{
    AllocationResult result;
    result.algorithm = getAlgorithmName();
    result.initialBlocks = blocks;

    for (int i = 0; i < (int)processes.size(); i++) {
        int chosen = -1;

        for (int j = 0; j < (int)blocks.size(); j++) {
            if (blocks[j] >= processes[i].size) {
                if (chosen == -1 || blocks[j] < blocks[chosen]) {
                    chosen = j;     // smaller than the best one found so far
                }
            }
        }

        if (chosen != -1) {
            blocks[chosen] -= processes[i].size;
        }

        AllocationStep step;
        step.pid = processes[i].pid;
        step.size = processes[i].size;
        step.blockIndex = chosen;
        step.blocksAfter = blocks;
        result.steps.push_back(step);
    }

    calculateValues(result);
    return result;
}

std::string BestFit::getAlgorithmName() const { return "Best Fit"; }
