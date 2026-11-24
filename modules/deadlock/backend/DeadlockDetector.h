#ifndef OSIMX_DEADLOCK_DETECTOR_H
#define OSIMX_DEADLOCK_DETECTOR_H
#include <string>
#include <vector>
namespace deadlock {
using Matrix = std::vector<std::vector<int>>;
struct Result {
    bool valid = false;
    bool safe = false;
    std::string error;
    std::vector<int> sequence;
    std::vector<int> unfinished;
    std::vector<std::string> steps;
};
struct RequestResult { bool granted = false; std::string reason; Result safety; };
Result bankers(const Matrix& allocation, const Matrix& maximum, const std::vector<int>& available);
Result detect(const Matrix& allocation, const Matrix& request, const std::vector<int>& available);
RequestResult requestResources(const Matrix& allocation, const Matrix& maximum, const std::vector<int>& available, int pid, const std::vector<int>& request);
}
class DeadlockDetector {
public:
    using Matrix = deadlock::Matrix;
    using Result = deadlock::Result;
    static Result bankers(const Matrix& a, const Matrix& m, const std::vector<int>& v) { return deadlock::bankers(a,m,v); }
    static Result detect(const Matrix& a, const Matrix& r, const std::vector<int>& v) { return deadlock::detect(a,r,v); }
};
#endif
