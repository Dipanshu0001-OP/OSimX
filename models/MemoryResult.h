#ifndef OSIMX_MEMORY_RESULT_H
#define OSIMX_MEMORY_RESULT_H

#include <string>
#include <vector>

// ---------- Memory Allocation (First / Best / Worst Fit) ----------

struct MemoryProcess {
    int pid = 0;
    int size = 0;
};

// One step = one process tried to get a block.
// The frontend replays these steps one by one to animate allocation.
struct AllocationStep {
    int pid = 0;
    int size = 0;
    int blockIndex = -1;           // -1 => could not be allocated
    std::vector<int> blocksAfter;  // free size of every block AFTER this step
};

struct AllocationResult {
    std::string algorithm;
    std::vector<int> initialBlocks;
    std::vector<AllocationStep> steps;

    int allocatedCount = 0;
    int failedCount = 0;
    int totalFreeMemory = 0;
    int largestFreeBlock = 0;
    int externalFragmentation = 0;  // total free memory minus the largest free block
};

// ---------- Page Replacement (FIFO / LRU / Optimal) ----------

// One step = one page reference.
struct PageStep {
    int page = 0;
    std::vector<int> frames;  // size == frameCount, -1 => empty frame
    bool hit = false;
    int victim = -1;          // page evicted on this step, -1 => none
};

struct PageReplacementResult {
    std::string algorithm;
    int frameCount = 0;
    std::vector<PageStep> steps;

    int hits = 0;
    int faults = 0;
    double hitRatio = 0.0;
    double faultRatio = 0.0;
};

#endif
