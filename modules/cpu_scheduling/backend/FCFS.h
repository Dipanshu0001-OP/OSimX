#ifndef OSIMX_FCFS_H
#define OSIMX_FCFS_H
#include "CPUScheduler.h"
class FCFS : public CPUScheduler {
public:
    SchedulingResult schedule(std::vector<Process> processes) override;
    std::string getAlgorithmName() const override;
};
#endif
