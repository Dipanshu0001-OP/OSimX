#include "CPUScheduler.h"
#include <algorithm>

void CPUScheduler::calculateValues(SchedulingResult& result)
{
    if (result.processes.empty()) {
        result = SchedulingResult();
        return;
    }

    double totalWaiting = 0.0;
    double totalTurnaround = 0.0;
    double totalResponse = 0.0;
    int elapsed = 0;
    int busy = 0;

    for (auto& process : result.processes) {
        if (process.completionTime > 0) {
            process.turnaroundTime = process.completionTime - process.arrivalTime;
            process.waitingTime = std::max(0, process.turnaroundTime - process.burstTime);
        }
        process.responseTime = process.startTime >= 0
            ? std::max(0, process.startTime - process.arrivalTime)
            : 0;

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
