#include "WorstFit.h"

// Worst Fit: check all blocks and pick the LARGEST one.
AllocationResult WorstFit::allocate(std::vector<int> blocks,
                                    std::vector<MemoryProcess> processes)
{
    AllocationResult result;
    result.algorithm = getAlgorithmName();
    result.initialBlocks = blocks;

    for (int i = 0; i < (int)processes.size(); i++) {
        int chosen = -1;

        for (int j = 0; j < (int)blocks.size(); j++) {
            if (blocks[j] >= processes[i].size) {
                if (chosen == -1 || blocks[j] > blocks[chosen]) {
                    chosen = j;     // bigger than the biggest one found so far
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

std::string WorstFit::getAlgorithmName() const { return "Worst Fit"; }
