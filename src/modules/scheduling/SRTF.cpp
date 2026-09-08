#ifndef SRTF_H
#define SRTF_H

#include "CPUScheduler.h"
#include <algorithm>
#include <climits>

using namespace std;

// SRTF - Shortest Remaining Time First (Preemptive SJF)
 
// Working of SRTF:
//   1. we have to check if all the processes have been arrived
//   2. select the process with the smallest remaining time
//   3. solve for the first unit
//   4. evaluate after the next unit
//   5. Preempt if a new process arrives with shorter remaining time

class SRTF : public CPUScheduler {
public:
    SchedulingResult schedule(vector<Process> processes) override {
        
        SchedulingResult result;
        
        // Initialize remaining time = burst time
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
            
            //Find process with shortest remaining time
            int shortestIdx = -1;
            int minRemaining = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                if (remaining[i].remainingTime > 0 && 
                    remaining[i].arrivalTime <= currentTime) {
                    if (remaining[i].remainingTime < minRemaining) {
                        minRemaining = remaining[i].remainingTime;
                        shortestIdx = i;
                    }
                }
            }
            
            //If no process arrived, jump to next arrival
            if (shortestIdx == -1) {
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
            
            Process& p = remaining[shortestIdx];
            
            // Record start time if first time
            if (p.startTime == -1) {
                p.startTime = currentTime;
            }
            
            // Execute for 1 unit (preemptive)
            if (prevPid != p.id) {
                // New process started add to Gantt chart
                GanttChart entry;
                entry.pid = p.id;
                entry.startTime = currentTime;
                // End time will be updated in next iteration
                result.ganttChart.push_back(entry);
                prevPid = p.id;
            }
            
            p.remainingTime--;
            currentTime++;
            
            // Update last Gantt value end time
            if (!result.ganttChart.empty()) {
                result.ganttChart.back().endTime = currentTime;
            }
            
            // If process finished calculate times
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
        return "Shortest Remaining Time First (SRTF)";
    }
};

#endif