#include "CPUScheduler.h"
#include <algorithm>

void CPUScheduler::normalize(std::vector<Process>& processes)
{
    for (auto& p : processes) {
        p.arrivalTime = std::max(0, p.arrivalTime);
        p.burstTime = std::max(1, p.burstTime);
        p.remainingTime = p.burstTime;
        p.startTime = -1;
        p.completionTime = 0;
        p.waitingTime = 0;
        p.turnaroundTime = 0;
        p.responseTime = 0;
    }
}

void CPUScheduler::calculateValues(SchedulingResult& result)
{
    if (result.processes.empty()) {
        result.avgWaitingTime = 0.0;
        result.avgTurnaroundTime = 0.0;
        result.avgResponseTime = 0.0;
        result.cpuUtilization = 0.0;
        result.throughput = 0.0;
        return;
    }

    double totalWaiting = 0.0;
    double totalTurnaround = 0.0;
    double totalResponse = 0.0;
    int elapsed = 0;
    int busy = 0;

    for (auto& process : result.processes) {
        if (process.startTime >= 0) {
            process.turnaroundTime = process.completionTime - process.arrivalTime;
            process.waitingTime = std::max(0, process.turnaroundTime - process.burstTime);
            process.responseTime = std::max(0, process.startTime - process.arrivalTime);
        } else {
            process.turnaroundTime = 0;
            process.waitingTime = 0;
            process.responseTime = 0;
        }

        totalWaiting += process.waitingTime;
        totalTurnaround += process.turnaroundTime;
        totalResponse += process.responseTime;
        elapsed = std::max(elapsed, process.completionTime);
    }

    for (const auto& segment : result.ganttChart) {
        if (segment.pid >= 0)
            busy += std::max(0, segment.endTime - segment.startTime);
        elapsed = std::max(elapsed, segment.endTime);
    }

    const double count = static_cast<double>(result.processes.size());
    result.avgWaitingTime = totalWaiting / count;
    result.avgTurnaroundTime = totalTurnaround / count;
    result.avgResponseTime = totalResponse / count;
    result.cpuUtilization = elapsed > 0 ? (100.0 * busy / elapsed) : 0.0;
    result.throughput = elapsed > 0 ? (count / elapsed) : 0.0;
}
