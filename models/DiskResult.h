#ifndef OSIMX_DISK_RESULT_H
#define OSIMX_DISK_RESULT_H

#include <string>
#include <vector>

struct DiskStep {
    int index = 0;
    int from = 0;
    int to = 0;
    int distance = 0;
    int cumulative = 0;
    std::vector<int> remaining;
    std::vector<int> visited;
    std::string reason;
    bool servesRequest = true;
    bool circularReturn = false;
};

struct DiskResult {
    std::string algorithm;
    std::vector<int> sequence;
    std::vector<DiskStep> steps;
    int totalSeek = 0;
    int returnDistance = 0;
    int requestsServed = 0;
    double averageSeek = 0.0;
};

#endif
