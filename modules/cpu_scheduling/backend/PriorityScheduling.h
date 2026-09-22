#ifndef OSIMX_PRIORITY_SCHEDULING_H
#define OSIMX_PRIORITY_SCHEDULING_H
#include "CPUScheduler.h"
class PriorityScheduling : public CPUScheduler {
public:
    SchedulingResult schedule(std::vector<Process> processes) override;
    std::string getAlgorithmName() const override;
};
#endif
