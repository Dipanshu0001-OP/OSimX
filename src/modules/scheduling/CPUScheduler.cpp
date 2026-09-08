#include "CPUScheduler.h"

using namespace std;

void CPUScheduler::calculateValues(SchedulingResult& result) {
    // If no processes, set all metrics to 0 and return
    if (result.processes.empty()) {
        result.avgWaitingTime = 0;
        result.avgTurnaroundTime = 0;
        result.avgResponseTime = 0;
        result.cpuUtilization = 0;
        result.throughput = 0;
        return;
    }
    
    double totalWaiting = 0;
    double totalTurnaround = 0;
    double totalResponse = 0;
    int totalTime = 0;
    
    for (const auto& p : result.processes) {
        totalWaiting += p.waitingTime;
        totalTurnaround += p.turnaroundTime;
        totalResponse += p.responseTime;
        
        //Total time calculation after every process
        if (p.completionTime > totalTime) {
            totalTime = p.completionTime;
        }
    }
    
    // Number of processes
    int n = result.processes.size();
    
    // Calculate averages
    result.avgWaitingTime = totalWaiting / n;
    result.avgTurnaroundTime = totalTurnaround / n;
    result.avgResponseTime = totalResponse / n;
    
    // CPU Utilization
    result.cpuUtilization = totalTime > 0 ? 100.0 : 0;
    
    // Throughput
    result.throughput = totalTime > 0 ? (double)n / totalTime : 0;
}
