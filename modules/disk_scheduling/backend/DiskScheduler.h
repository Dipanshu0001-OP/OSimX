#ifndef OSIMX_DISK_SCHEDULER_H
#define OSIMX_DISK_SCHEDULER_H

#include <string>
#include <vector>
#include "models/DiskResult.h"

class DiskScheduler {
public:
    static DiskResult schedule(const std::string& algorithm,
                               int head,
                               const std::vector<int>& requests,
                               int maxCylinder,
                               bool upward);
private:
    struct Move { int to; std::string reason; };
    static DiskResult pack(const std::string& algorithm, int head, const std::vector<Move>& path);
};

#endif
