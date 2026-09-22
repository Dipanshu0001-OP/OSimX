#ifndef OSIMX_SCHEDULING_RESULT_H
#define OSIMX_SCHEDULING_RESULT_H

#include <vector>
#include "Process.h"

struct GanttChart {
    int pid = -1;
    int startTime = 0;
    int endTime = 0;
};

struct SchedulingResult {
    std::vector<Process> processes;
    std::vector<GanttChart> ganttChart;
    double avgWaitingTime = 0.0;
    double avgTurnaroundTime = 0.0;
    double avgResponseTime = 0.0;
    double cpuUtilization = 0.0;
    double throughput = 0.0;
};

#endif
