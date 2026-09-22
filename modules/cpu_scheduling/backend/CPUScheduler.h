#ifndef OSIMX_CPU_SCHEDULER_H
#define OSIMX_CPU_SCHEDULER_H

#include <string>
#include <vector>
#include "models/SchedulingResult.h"

class CPUScheduler {
public:
    virtual ~CPUScheduler() = default;
    virtual SchedulingResult schedule(std::vector<Process> processes) = 0;
    virtual std::string getAlgorithmName() const = 0;

protected:
    void calculateValues(SchedulingResult& result);
};

#endif
