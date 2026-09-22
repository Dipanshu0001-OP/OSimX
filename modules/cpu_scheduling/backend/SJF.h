#ifndef OSIMX_SJF_H
#define OSIMX_SJF_H
#include "CPUScheduler.h"
class SJF : public CPUScheduler {
public:
    SchedulingResult schedule(std::vector<Process> processes) override;
    std::string getAlgorithmName() const override;
};
#endif
