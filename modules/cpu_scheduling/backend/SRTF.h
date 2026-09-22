#ifndef OSIMX_SRTF_H
#define OSIMX_SRTF_H
#include "CPUScheduler.h"
class SRTF : public CPUScheduler {
public:
    SchedulingResult schedule(std::vector<Process> processes) override;
    std::string getAlgorithmName() const override;
};
#endif
