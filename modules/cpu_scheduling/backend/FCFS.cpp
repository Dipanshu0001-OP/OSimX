#include "FCFS.h"
#include <algorithm>

SchedulingResult FCFS::schedule(std::vector<Process> processes)
{
    std::stable_sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });

    SchedulingResult result;
    result.processes = processes;
    int currentTime = 0;

    for (auto& p : result.processes) {
        if (currentTime < p.arrivalTime) {
            result.ganttChart.push_back({-1, currentTime, p.arrivalTime});
            currentTime = p.arrivalTime;
        }
        p.startTime = currentTime;
        p.remainingTime = p.burstTime;
        p.completionTime = currentTime + p.burstTime;
        p.remainingTime = 0;
        result.ganttChart.push_back({p.id, currentTime, p.completionTime});
        currentTime = p.completionTime;
    }

    calculateValues(result);
    return result;
}

std::string FCFS::getAlgorithmName() const
{
    return "First Come, First Served (FCFS)";
}
