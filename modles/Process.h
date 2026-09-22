#ifndef OSIMX_PROCESS_H
#define OSIMX_PROCESS_H

struct Process {
    int id = 0;
    int arrivalTime = 0;
    int burstTime = 0;
    int priority = 0;
    int startTime = -1;
    int completionTime = 0;
    int waitingTime = 0;
    int turnaroundTime = 0;
    int responseTime = 0;
    int remainingTime = 0;

    Process(int i = 0, int a = 0, int b = 0, int p = 0)
        : id(i), arrivalTime(a), burstTime(b), priority(p), remainingTime(b) {}
};

#endif
