#ifndef OSIMX_ROUND_ROBIN_H
#define OSIMX_ROUND_ROBIN_H
#include "CPUScheduler.h"
class RoundRobin : public CPUScheduler {
public:
    explicit RoundRobin(int quantum = 2);
    SchedulingResult schedule(std::vector<Process> processes) override;
    std::string getAlgorithmName() const override;
private:
    int timeQuantum;
};
#endif
