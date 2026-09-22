#include "RoundRobin.h"
#include <algorithm>
#include <queue>

RoundRobin::RoundRobin(int quantum) : timeQuantum(std::max(1, quantum)) {}

SchedulingResult RoundRobin::schedule(std::vector<Process> processes)
{
    SchedulingResult result;
    result.processes = processes;
    std::stable_sort(result.processes.begin(), result.processes.end(), [](const Process& a, const Process& b) {
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.id < b.id;
    });

    std::queue<Process*> ready;
    const int n = static_cast<int>(result.processes.size());
    int next = 0;
    int completed = 0;
    int currentTime = 0;

    while (completed < n) {
        while (next < n && result.processes[next].arrivalTime <= currentTime) {
            result.processes[next].remainingTime = result.processes[next].burstTime;
            ready.push(&result.processes[next]);
            ++next;
        }

        if (ready.empty()) {
            const int jump = result.processes[next].arrivalTime;
            if (jump > currentTime)
                result.ganttChart.push_back({-1, currentTime, jump});
            currentTime = jump;
            continue;
        }

        Process* p = ready.front();
        ready.pop();
        if (p->startTime < 0) p->startTime = currentTime;

        const int runFor = std::min(timeQuantum, p->remainingTime);
        result.ganttChart.push_back({p->id, currentTime, currentTime + runFor});
        p->remainingTime -= runFor;
        currentTime += runFor;

        while (next < n && result.processes[next].arrivalTime <= currentTime) {
            result.processes[next].remainingTime = result.processes[next].burstTime;
            ready.push(&result.processes[next]);
            ++next;
        }

        if (p->remainingTime > 0) {
            ready.push(p);
        } else {
            p->completionTime = currentTime;
            ++completed;
        }
    }

    calculateValues(result);
    return result;
}

std::string RoundRobin::getAlgorithmName() const
{
    return "Round Robin (preemptive, quantum = " + std::to_string(timeQuantum) + ")";
}
