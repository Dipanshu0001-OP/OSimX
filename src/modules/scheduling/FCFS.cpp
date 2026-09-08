#ifndef FCFS_H
#define FCFS_H

#include "CPUScheduler.h"
#include <algorithm>

using namespace std;

// FCFS Class - First Come First Served

class FCFS : public CPUScheduler {
public:
    SchedulingResult schedule(vector<Process> processes) override {
        
        // Step 1: Sorting of the arrival time
        sort(processes.begin(), processes.end(),
             [](const Process& a, const Process& b) {
                 return a.arrivalTime < b.arrivalTime;
             });
        
        SchedulingResult result;
        result.processes = processes;
        
        int currentTime = 0;
        
        // Step 2: Executing each process one by one
        for (auto& p : result.processes) {
            
            if (currentTime < p.arrivalTime) {
                currentTime = p.arrivalTime;
            }

            p.startTime = currentTime;
            p.completionTime = currentTime + p.burstTime;
            p.waitingTime = p.startTime - p.arrivalTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.responseTime = p.waitingTime;  // For FCFS, response = waiting
            
            GanttChart entry;
            entry.pid = p.id;
            entry.startTime = currentTime;
            entry.endTime = p.completionTime;
            result.ganttChart.push_back(entry);
            
            currentTime = p.completionTime;
        }
        
        calculateValues(result);
        return result;
    }
    
    string getAlgorithmName() const override {
        return "First Come First Served (FCFS)";
    }
};

#endif