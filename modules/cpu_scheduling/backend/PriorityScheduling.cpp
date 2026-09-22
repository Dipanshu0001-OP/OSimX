#include "PriorityScheduling.h"
#include <algorithm>
#include <climits>

SchedulingResult PriorityScheduling::schedule(std::vector<Process> processes)
{
    SchedulingResult result;
    result.processes = processes;
    for (auto& p : result.processes) {
        p.remainingTime = p.burstTime;
        p.startTime = -1;
    }

    const int n = static_cast<int>(result.processes.size());
    int completed = 0;
    int currentTime = 0;
    int previousPid = -2;

    while (completed < n) {
        int selected = -1;
        for (int i = 0; i < n; ++i) {
            const auto& p = result.processes[i];
            if (p.remainingTime <= 0 || p.arrivalTime > currentTime) continue;
            if (selected == -1 ||
                p.priority < result.processes[selected].priority ||
                (p.priority == result.processes[selected].priority && p.id < result.processes[selected].id)) {
                selected = i;
            }
        }

        if (selected == -1) {
            int nextArrival = INT_MAX;
            for (const auto& p : result.processes)
                if (p.remainingTime > 0) nextArrival = std::min(nextArrival, p.arrivalTime);
            if (nextArrival > currentTime)
                result.ganttChart.push_back({-1, currentTime, nextArrival});
            currentTime = nextArrival;
            previousPid = -2;
            continue;
        }

        auto& p = result.processes[selected];
        if (p.startTime < 0) p.startTime = currentTime;
        if (previousPid != p.id) {
            result.ganttChart.push_back({p.id, currentTime, currentTime});
            previousPid = p.id;
        }

        --p.remainingTime;
        ++currentTime;
        result.ganttChart.back().endTime = currentTime;

        if (p.remainingTime == 0) {
            p.completionTime = currentTime;
            ++completed;
        }
    }

    calculateValues(result);
    return result;
}

std::string PriorityScheduling::getAlgorithmName() const
{
    return "Priority Scheduling (preemptive, lower number = higher priority)";
}
