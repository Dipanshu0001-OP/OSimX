#ifndef ROUNDROBIN_H
#define ROUNDROBIN_H

#include "CPUScheduler.h"
#include <queue>
#include <algorithm>
#include <climits>

using namespace std;

// Round Robin Process Scheduler
 
// working of round robin
// 1.put all the currently working processes in a queue
// 2.give time quantum to each process
// 3.if the process is not over then it goes back to queue
// 4.process is changes after every time quantum

class RoundRobin : public CPUScheduler {
private:
    int timeQuantum;

public:
    RoundRobin(int quantum = INT_MAX) : timeQuantum(quantum) {}
    
    SchedulingResult schedule(vector<Process> processes) override {
        
        SchedulingResult result;
        
        // Initialize remaining time
        vector<Process> remaining = processes;
        for (auto& p : remaining) {
            p.remainingTime = p.burstTime;
            p.startTime = -1;  // Not started yet
        }
        
        // Sort by arrival time
        sort(remaining.begin(), remaining.end(),
             [](const Process& a, const Process& b) {
                 return a.arrivalTime < b.arrivalTime;
             });
        
        queue<Process*> readyQueue;
        int currentTime = 0;
        int completed = 0;
        int n = processes.size();
        int idx = 0;  
        
        while (completed < n) {
            
            //Add newly arrived processes to queue
            while (idx < n && remaining[idx].arrivalTime <= currentTime) {
                readyQueue.push(&remaining[idx]);
                idx++;
            }
            
            //If queue is empty, jump to next arrival
            if (readyQueue.empty()) {
                currentTime = remaining[idx].arrivalTime;
                continue;
            }
            
            //Get next process from front of queue
            Process* p = readyQueue.front();
            readyQueue.pop();
            
            // Record start time if first time
            if (p->startTime == -1) {
                p->startTime = currentTime;
            }
            
            //Execute for quantum or remaining time (whichever is less)
            int execTime = min(timeQuantum, p->remainingTime);
            
            //Add to Gantt chart
            GanttChart entry;
            entry.pid = p->id;
            entry.startTime = currentTime;
            entry.endTime = currentTime + execTime;
            result.ganttChart.push_back(entry);
            
            //Update time and remaining time
            currentTime += execTime;
            p->remainingTime -= execTime;
            
            //Add newly arrived processes during execution
            while (idx < n && remaining[idx].arrivalTime <= currentTime) {
                readyQueue.push(&remaining[idx]);
                idx++;
            }
            
            //If process not finished, add back to queue
            if (p->remainingTime > 0) {
                readyQueue.push(p);
            } else {
                // Process finished, calculate times
                p->completionTime = currentTime;
                p->waitingTime = p->startTime - p->arrivalTime;
                p->turnaroundTime = p->completionTime - p->arrivalTime;
                p->responseTime = p->startTime - p->arrivalTime;
                completed++;
            }
        }
        
        result.processes = remaining;
        calculateValues(result);
        return result;
    }
    
    string getAlgorithmName() const override {
        return "Round Robin (Quantum = " + to_string(timeQuantum) + ")";
    }
};

#endif