#ifndef CPU_SCHEDULER_H
#define CPU_SCHEDULER_H

#include <vector>
#include <string>

using namespace std;

// Process Structure
struct Process {
    // Input Fields
    int id;              
    int arrivalTime;     
    int burstTime;       
    int priority;        
    
    // Calculated Fields
    int startTime;       
    int completionTime;  
    int waitingTime;     
    int turnaroundTime;  
    int responseTime;     
    int remainingTime;   
    
    // Constructor
    Process(int i = 0, int a = 0, int b = 0, int p = 0)
        : id(i), arrivalTime(a), burstTime(b), priority(p),
          startTime(-1), completionTime(0), waitingTime(0),
          turnaroundTime(0), responseTime(0), remainingTime(b) {}
};


// GanttChart Structure

struct GanttChart {
    int pid;            
    int startTime;      
    int endTime;        
};

// SchedulingResult Structure

struct SchedulingResult {
    vector<Process> processes;       
    vector<GanttChart> ganttChart;   
    double avgWaitingTime;           
    double avgTurnaroundTime;        
    double avgResponseTime;          
    double cpuUtilization;           
    double throughput;               
};

// CPUScheduler Class (Abstract Base Class)

class CPUScheduler {
public:
    // Virtual destructor for proper cleanup so that the scheduler of the using case is cleaned not the base scheduler
    virtual ~CPUScheduler() = default;
    
    virtual SchedulingResult schedule(vector<Process> processes) = 0;
    virtual string getAlgorithmName() const = 0;
    
protected:
    void calculateValues(SchedulingResult& result);
};

#endif