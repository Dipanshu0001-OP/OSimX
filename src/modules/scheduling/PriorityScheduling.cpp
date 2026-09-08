#ifndef PRIORITY_SCHEDULING_H
#define PRIORITY_SCHEDULING_H

#include "CPUScheduler.h"
#include <algorithm>
#include <climits>

using namespace std;

/* Priority Scheduling Class (Preemptive)
 
How it works:
1. each process has its priority
2. check arrived processes at each unit time
3. pick the process with highest priority
4. if a new process comes with higher priority then it takes over 
    # Starvation is possible so low priority may never run.
*/

class PriorityScheduling : public CPUScheduler {
public:
    SchedulingResult schedule(vector<Process> processes) override {
        
        SchedulingResult result;
        
        // Initialize remaining time
        vector<Process> remaining = processes;
        for (auto& p : remaining) {
            p.remainingTime = p.burstTime;
            p.startTime = -1;
        }
        
        int currentTime = 0;
        int completed = 0;
        int n = processes.size();
        int prevPid = -1;
        
        while (completed < n) {
            
            // find the process with highest priority
            int highestIdx = -1;
            int highestPriority = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                if (remaining[i].remainingTime > 0 && 
                    remaining[i].arrivalTime <= currentTime) {
                    if (remaining[i].priority < highestPriority) {
                        highestPriority = remaining[i].priority;
                        highestIdx = i;
                    }
                }
            }
            
            // if no process is arrived then go to the next arrival
            if (highestIdx == -1) {
                int nextArrival = INT_MAX;
                for (int i = 0; i < n; i++) {
                    if (remaining[i].remainingTime > 0 && 
                        remaining[i].arrivalTime < nextArrival) {
                        nextArrival = remaining[i].arrivalTime;
                    }
                }
                currentTime = nextArrival;
                continue;
            }
            
            Process& p = remaining[highestIdx];
            
            // Record start time if first time
            if (p.startTime == -1) {
                p.startTime = currentTime;
            }
            
            // execute the process for 1 unit each
            if (prevPid != p.id) {
                // if a new process is started then add to gantt chart
                GanttChart entry;
                entry.pid = p.id;
                entry.startTime = currentTime;
                result.ganttChart.push_back(entry);
                prevPid = p.id;
            }
            
            p.remainingTime--;
            currentTime++;
            
            // update the end time of the last gant chart
            if (!result.ganttChart.empty()) {
                result.ganttChart.back().endTime = currentTime;
            }
            
            // if a process is finished then calculate its times 
            if (p.remainingTime == 0) {
                p.completionTime = currentTime;
                p.waitingTime = p.startTime - p.arrivalTime;
                p.turnaroundTime = p.completionTime - p.arrivalTime;
                p.responseTime = p.startTime - p.arrivalTime;
                completed++;
            }
        }
        
        result.processes = remaining;
        calculateValues(result);
        return result;
    }
    
    string getAlgorithmName() const override {
        return "Priority Scheduling (Preemptive)";
    }
};

#endif