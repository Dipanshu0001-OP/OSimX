#ifndef SJF_H
#define SJF_H

#include "CPUScheduler.h"
#include <algorithm>
#include <climits>  // For INT_MAX

using namespace std;


// SJF Class - Shortest Job First (Non-Preemptive)

// 
// Working
//   1.First we are checking all the processes that have arrived
//   2.Then we pick the one with smallest burst time
//   3.Run it to completion
//   4.then do the same for the remaining ones.


class SJF : public CPUScheduler {
public:
    SchedulingResult schedule(vector<Process> processes) override {
        
        SchedulingResult result;
        result.processes = processes;
        
        int n = processes.size();
        vector<bool> isCompleted(n, false);
        int currentTime = 0;
        int completed = 0;
        
        while (completed < n) {
            
            //arrived processes with the smallest burst time to execute it first. 
            int shortestIdx = -1;
            int minBurst = INT_MAX;
            
            for (int i = 0; i < n; i++) {
                // if process is arrived but not completed yet
                if (!isCompleted[i] && processes[i].arrivalTime <= currentTime) {
                    if (processes[i].burstTime < minBurst) {
                        minBurst = processes[i].burstTime;
                        shortestIdx = i;
                    }
                }
            }
            
            // if no process arrived then go to to the arrival of the next process
            if (shortestIdx == -1) {
                int nextArrival = INT_MAX;
                for (int i = 0; i < n; i++) {
                    if (!isCompleted[i] && processes[i].arrivalTime < nextArrival) {
                        nextArrival = processes[i].arrivalTime;
                    }
                }
                currentTime = nextArrival;
                continue;
            }
            
            //execution of the selected process
            Process& p = processes[shortestIdx];
            
            p.startTime = currentTime;
            p.completionTime = currentTime + p.burstTime;
            p.waitingTime = p.startTime - p.arrivalTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.responseTime = p.waitingTime;
            
            GanttChart entry;
            entry.pid = p.id;
            entry.startTime = currentTime;
            entry.endTime = p.completionTime;
            result.ganttChart.push_back(entry);
            
            //updating the values
            currentTime = p.completionTime;
            isCompleted[shortestIdx] = true;
            completed++;
        }
        
        result.processes = processes;
        calculateValues(result);
        return result;
    }
    
    string getAlgorithmName() const override {
        return "Shortest Job First (SJF)";
    }
};

#endif