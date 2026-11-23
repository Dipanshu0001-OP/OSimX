#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "modules/cpu_scheduling/backend/FCFS.h"
#include "modules/cpu_scheduling/backend/SJF.h"
#include "modules/cpu_scheduling/backend/SRTF.h"
#include "modules/cpu_scheduling/backend/PriorityScheduling.h"
#include "modules/cpu_scheduling/backend/RoundRobin.h"
#include "modules/disk_scheduling/backend/DiskScheduler.h"
#include "modules/memory_management/backend/FirstFit.h"
#include "modules/memory_management/backend/BestFit.h"
#include "modules/memory_management/backend/WorstFit.h"
#include "modules/memory_management/backend/FIFO.h"
#include "modules/memory_management/backend/LRU.h"
#include "modules/memory_management/backend/Optimal.h"

int main()
{
    int checks = 0;
    auto check = [&checks](bool condition, const char* message) {
        ++checks;
        if (!condition) {
            std::cerr << "FAIL: " << message << '\n';
            std::exit(1);
        }
    };

    // CPU scheduling textbook/reference checks.
    {
        FCFS scheduler;
        const auto result = scheduler.schedule({Process(1, 0, 8), Process(2, 1, 4),
                                                Process(3, 2, 9), Process(4, 3, 5)});
        check(result.processes.size() == 4, "FCFS returns every process");
        check(std::abs(result.avgWaitingTime - 8.75) < 0.001, "FCFS average waiting time");
    }
    {
        SJF scheduler;
        const auto result = scheduler.schedule({Process(1, 0, 7), Process(2, 2, 4),
                                                Process(3, 4, 1), Process(4, 5, 4)});
        check(result.ganttChart.size() == 4, "SJF segment count");
        check(result.ganttChart[1].pid == 3 && result.ganttChart[2].pid == 2,
              "SJF selects shortest currently-ready process");
    }
    {
        SRTF scheduler;
        const auto result = scheduler.schedule({Process(5, 0, 4), Process(2, 1, 3)});
        check(result.ganttChart.size() == 2, "SRTF does not preempt on equal remaining time");
        check(result.ganttChart[0].pid == 5 && result.ganttChart[1].pid == 2,
              "SRTF tie keeps running process");
        check(result.ganttChart[0].endTime == 4 && result.ganttChart[1].endTime == 7,
              "SRTF tie timeline");
    }
    {
        SRTF scheduler;
        const auto result = scheduler.schedule({Process(1, 0, 8), Process(2, 1, 4)});
        check(result.ganttChart.size() >= 3 && result.ganttChart[0].pid == 1 &&
              result.ganttChart[1].pid == 2, "SRTF preempts for strictly shorter remaining time");
    }
    {
        PriorityScheduling scheduler;
        const auto result = scheduler.schedule({Process(4, 0, 3, 2), Process(1, 1, 3, 2)});
        check(result.ganttChart.size() == 2, "equal priority does not cause pointless preemption");
    }
    {
        PriorityScheduling scheduler;
        const auto result = scheduler.schedule({Process(1, 0, 5, 3), Process(2, 1, 2, 1)});
        check(result.ganttChart.size() >= 3 && result.ganttChart[1].pid == 2,
              "higher-priority process preempts");
    }
    {
        RoundRobin scheduler(2);
        const auto result = scheduler.schedule({Process(1, 0, 5), Process(2, 1, 3)});
        check(result.processes.size() == 2 && result.processes[0].completionTime > 0 &&
              result.processes[1].completionTime > 0, "Round Robin completes all processes");
    }

    // Classic disk request queue: head 53, cylinders 0..199.
    const std::vector<int> requests = {98, 183, 37, 122, 14, 124, 65, 67};
    check(DiskScheduler::schedule("FCFS", 53, requests, 199, true).totalSeek == 640,
          "FCFS textbook seek total");
    check(DiskScheduler::schedule("SSTF", 53, requests, 199, true).totalSeek == 236,
          "SSTF textbook seek total");
    check(DiskScheduler::schedule("SCAN", 53, requests, 199, true).totalSeek == 331,
          "SCAN textbook seek total");
    const auto cscan = DiskScheduler::schedule("CSCAN", 53, requests, 199, true);
    check(cscan.totalSeek == 382, "C-SCAN seek total including wrap movement");
    check(cscan.returnDistance == 199, "C-SCAN return movement is reported separately");
    check(DiskScheduler::schedule("LOOK", 53, requests, 199, true).totalSeek == 299,
          "LOOK seek total");
    check(DiskScheduler::schedule("CLOOK", 53, requests, 199, true).totalSeek == 322,
          "C-LOOK seek total including circular jump");
    {
        const auto result = DiskScheduler::schedule("SCAN", 199, {100, 50}, 199, true);
        check(result.sequence.size() == 3 && result.sequence[1] == 100,
              "SCAN at boundary does not add duplicate boundary movement");
    }
    {
        const auto result = DiskScheduler::schedule("BOGUS", 53, {10}, 199, true);
        check(result.steps.empty() && result.algorithm.find("Unsupported") != std::string::npos,
              "unknown disk algorithm is rejected instead of falling through to C-LOOK");
    }
    {
        const auto result = DiskScheduler::schedule("SCAN", 53, {98, 183, 37}, 199, true);
        check(result.steps[0].remaining.size() == 2 && result.steps[0].remaining[0] == 183,
              "disk playback remaining queue lists requests, not boundary movements");
    }

    // Memory allocation policies.
    {
        FirstFit allocator;
        const auto result = allocator.allocate({100, 500, 200},
            {MemoryProcess{1, 90}, MemoryProcess{2, 417}, MemoryProcess{3, 112}});
        check(result.steps[0].blockIndex == 0 && result.steps[1].blockIndex == 1 &&
              result.steps[2].blockIndex == 2, "First Fit placements");
    }
    {
        BestFit allocator;
        const auto result = allocator.allocate({100, 500, 200, 300},
            {MemoryProcess{1, 190}, MemoryProcess{2, 80}});
        check(result.steps[0].blockIndex == 2 && result.steps[1].blockIndex == 0,
              "Best Fit placements");
    }
    {
        WorstFit allocator;
        const auto result = allocator.allocate({100, 500, 200, 300},
            {MemoryProcess{1, 190}, MemoryProcess{2, 80}});
        check(result.steps[0].blockIndex == 1 && result.steps[1].blockIndex == 1,
              "Worst Fit placements");
    }

    // Page-replacement reference string.
    const std::vector<int> refs = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    {
        FIFO policy;
        check(policy.replace(refs, 3).faults == 10, "FIFO page-fault count");
    }
    {
        LRU policy;
        check(policy.replace(refs, 3).faults == 9, "LRU page-fault count");
    }
    {
        Optimal policy;
        check(policy.replace(refs, 3).faults == 7, "Optimal page-fault count");
    }

    std::cout << checks << " backend checks passed\n";
    return 0;
}
