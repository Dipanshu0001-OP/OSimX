#include "SJF.h"
#include <algorithm>
#include <climits>

SchedulingResult SJF::schedule(std::vector<Process> processes)
{
    SchedulingResult result;
    result.processes = processes;
    const int n = static_cast<int>(result.processes.size());
    std::vector<bool> done(n, false);
    int completed = 0;
    int currentTime = 0;

    while (completed < n) {
        int selected = -1;
        for (int i = 0; i < n; ++i) {
            const auto& p = result.processes[i];
            if (done[i] || p.arrivalTime > currentTime) continue;
            if (selected == -1 ||
                p.burstTime < result.processes[selected].burstTime ||
                (p.burstTime == result.processes[selected].burstTime && p.id < result.processes[selected].id)) {
                selected = i;
            }
        }

        if (selected == -1) {
            int nextArrival = INT_MAX;
            for (int i = 0; i < n; ++i)
                if (!done[i])
                    nextArrival = std::min(nextArrival, result.processes[i].arrivalTime);
            result.ganttChart.push_back({-1, currentTime, nextArrival});
            currentTime = nextArrival;
            continue;
        }

        auto& p = result.processes[selected];
        p.startTime = currentTime;
        p.remainingTime = 0;
        p.completionTime = currentTime + p.burstTime;
        result.ganttChart.push_back({p.id, currentTime, p.completionTime});
        currentTime = p.completionTime;
        done[selected] = true;
        ++completed;
    }

    calculateValues(result);
    return result;
}

std::string SJF::getAlgorithmName() const
{
    return "Shortest Job First (non-preemptive)";
}
