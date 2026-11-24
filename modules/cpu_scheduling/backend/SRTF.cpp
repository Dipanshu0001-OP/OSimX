#include "SRTF.h"
#include <algorithm>
#include <climits>

SchedulingResult SRTF::schedule(std::vector<Process> processes)
{
    normalize(processes);
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
            if (selected == -1) {
                selected = i;
            } else {
                const auto& current = result.processes[selected];
                const bool pIsRunning = p.id == previousPid;
                const bool currentIsRunning = current.id == previousPid;
                if (p.remainingTime < current.remainingTime ||
                    (p.remainingTime == current.remainingTime && pIsRunning && !currentIsRunning) ||
                    (p.remainingTime == current.remainingTime && pIsRunning == currentIsRunning &&
                     (p.arrivalTime < current.arrivalTime ||
                      (p.arrivalTime == current.arrivalTime && p.id < current.id)))) {
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            int nextArrival = INT_MAX;
            for (const auto& p : result.processes)
                if (p.remainingTime > 0) nextArrival = std::min(nextArrival, p.arrivalTime);
            if (nextArrival > currentTime) {
                result.ganttChart.push_back({-1, currentTime, nextArrival});
                currentTime = nextArrival;
            }
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

std::string SRTF::getAlgorithmName() const
{
    return "Shortest Remaining Time First (preemptive)";
}
