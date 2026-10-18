#include "FirstFit.h"

// First Fit: give the process the FIRST block that is big enough.
AllocationResult FirstFit::allocate(std::vector<int> blocks,
                                    std::vector<MemoryProcess> processes)
{
    AllocationResult result;
    result.algorithm = getAlgorithmName();
    result.initialBlocks = blocks;

    for (int i = 0; i < (int)processes.size(); i++) {
        int chosen = -1;

        for (int j = 0; j < (int)blocks.size(); j++) {
            if (blocks[j] >= processes[i].size) {
                chosen = j;
                break;              // first one found, stop looking
            }
        }

        if (chosen != -1) {
            blocks[chosen] -= processes[i].size;
        }

        AllocationStep step;
        step.pid = processes[i].pid;
        step.size = processes[i].size;
        step.blockIndex = chosen;   // -1 means it did not fit anywhere
        step.blocksAfter = blocks;
        result.steps.push_back(step);
    }

    calculateValues(result);
    return result;
}

std::string FirstFit::getAlgorithmName() const { return "First Fit"; }
