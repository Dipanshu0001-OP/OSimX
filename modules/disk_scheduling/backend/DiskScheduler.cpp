#include "DiskScheduler.h"

#include <algorithm>
#include <cmath>
#include <set>

const std::vector<std::string>& DiskScheduler::algorithms()
{
    static const std::vector<std::string> names = {"FCFS", "SSTF", "SCAN", "CSCAN", "LOOK", "CLOOK"};
    return names;
}

DiskResult DiskScheduler::pack(const std::string& algorithm, int head, const std::vector<Move>& path)
{
    DiskResult result;
    result.algorithm = algorithm;
    result.sequence.push_back(head);

    // Keep only actual, not-yet-served requests in the queue shown during playback.
    std::vector<int> remaining;
    for (const auto& move : path)
        if (move.servesRequest) remaining.push_back(move.to);

    std::vector<int> visited;
    int from = head;
    int cumulative = 0;
    int returnDistance = 0;
    int served = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        const auto& move = path[i];
        const int distance = std::abs(move.to - from);
        cumulative += distance;
        if (move.circularReturn) returnDistance += distance;
        if (move.servesRequest) {
            auto pending = std::find(remaining.begin(), remaining.end(), move.to);
            if (pending != remaining.end()) remaining.erase(pending);
            visited.push_back(move.to);
            ++served;
        }

        DiskStep step;
        step.index = static_cast<int>(i);
        step.from = from;
        step.to = move.to;
        step.distance = distance;
        step.cumulative = cumulative;
        step.remaining = remaining;
        step.visited = visited;
        step.reason = move.reason;
        step.servesRequest = move.servesRequest;
        step.circularReturn = move.circularReturn;
        result.steps.push_back(step);
        result.sequence.push_back(move.to);
        from = move.to;
    }

    result.totalSeek = cumulative;
    result.returnDistance = returnDistance;
    result.requestsServed = served;
    result.averageSeek = served == 0 ? 0.0 : static_cast<double>(cumulative) / served;
    return result;
}

DiskResult DiskScheduler::schedule(const std::string& algorithm,
                                   int head,
                                   const std::vector<int>& requests,
                                   int maxCylinder,
                                   bool upward)
{
    maxCylinder = std::max(1, maxCylinder);
    head = std::max(0, std::min(head, maxCylinder));
    std::vector<int> req;
    req.reserve(requests.size());
    for (int request : requests)
        if (request >= 0 && request <= maxCylinder) req.push_back(request);

    if (std::find(algorithms().begin(), algorithms().end(), algorithm) == algorithms().end()) {
        DiskResult invalid;
        invalid.algorithm = "Unsupported algorithm: " + algorithm;
        invalid.sequence.push_back(head);
        return invalid;
    }
    if (req.empty()) return pack(algorithm, head, {});

    std::vector<Move> path;
    const auto serve = [&path](int cylinder, const std::string& reason) {
        path.push_back({cylinder, reason, true, false});
    };
    const auto travel = [&path](int cylinder, const std::string& reason, bool circular = false) {
        path.push_back({cylinder, reason, false, circular});
    };

    if (algorithm == "FCFS") {
        for (std::size_t i = 0; i < req.size(); ++i)
            serve(req[i], i == 0 ? "FCFS serves the first request in arrival order."
                                 : "FCFS preserves the original request order.");
        return pack(algorithm, head, path);
    }
    if (algorithm == "SSTF") {
        std::vector<int> pending = req;
        int position = head;
        while (!pending.empty()) {
            auto best = std::min_element(pending.begin(), pending.end(), [position](int a, int b) {
                const int da = std::abs(a - position), db = std::abs(b - position);
                return da == db ? a < b : da < db;
            });
            const int target = *best;
            serve(target, "SSTF selects the pending request with the shortest seek distance; ties choose the lower cylinder.");
            position = target;
            pending.erase(best);
        }
        return pack(algorithm, head, path);
    }

    std::vector<int> atHead, lower, upper;
    for (int request : req) {
        if (request == head) atHead.push_back(request);
        else if (request < head) lower.push_back(request);
        else upper.push_back(request);
    }
    std::sort(lower.begin(), lower.end());
    std::sort(upper.begin(), upper.end());
    for (int request : atHead)
        serve(request, "The head is already at this cylinder, so the request is served with zero movement.");

    const auto currentPosition = [&]() { return path.empty() ? head : path.back().to; };
    if (algorithm == "SCAN") {
        if (upward) {
            for (int request : upper) serve(request, "SCAN serves requests on the upward sweep.");
            if (!lower.empty()) {
                if (currentPosition() != maxCylinder)
                    travel(maxCylinder, "SCAN reaches the upper disk boundary before reversing.");
                for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                    serve(*it, "SCAN serves requests after reversing downward.");
            }
        } else {
            for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                serve(*it, "SCAN serves requests on the downward sweep.");
            if (!upper.empty()) {
                if (currentPosition() != 0)
                    travel(0, "SCAN reaches cylinder 0 before reversing.");
                for (int request : upper)
                    serve(request, "SCAN serves requests after reversing upward.");
            }
        }
    } else if (algorithm == "CSCAN") {
        if (upward) {
            for (int request : upper) serve(request, "C-SCAN serves requests on the upward sweep.");
            if (!lower.empty()) {
                if (currentPosition() != maxCylinder)
                    travel(maxCylinder, "C-SCAN reaches the upper disk boundary.");
                travel(0, "C-SCAN returns to cylinder 0 without serving requests during the wrap.", true);
                for (int request : lower)
                    serve(request, "C-SCAN resumes the upward sweep after wrapping.");
            }
        } else {
            for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                serve(*it, "C-SCAN serves requests on the downward sweep.");
            if (!upper.empty()) {
                if (currentPosition() != 0)
                    travel(0, "C-SCAN reaches cylinder 0.");
                travel(maxCylinder, "C-SCAN wraps to the upper disk boundary.", true);
                for (auto it = upper.rbegin(); it != upper.rend(); ++it)
                    serve(*it, "C-SCAN resumes the downward sweep after wrapping.");
            }
        }
    } else if (algorithm == "LOOK") {
        if (upward) {
            for (int request : upper) serve(request, "LOOK sweeps upward only as far as the last pending request.");
            for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                serve(*it, "LOOK reverses at the last request rather than travelling to the disk boundary.");
        } else {
            for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                serve(*it, "LOOK sweeps downward only as far as the last pending request.");
            for (int request : upper)
                serve(request, "LOOK reverses at the last request rather than travelling to the disk boundary.");
        }
    } else { // CLOOK
        if (upward) {
            for (int request : upper) serve(request, "C-LOOK sweeps upward through pending requests.");
            for (int request : lower) serve(request, "C-LOOK wraps to the lowest pending request and continues upward.");
        } else {
            for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                serve(*it, "C-LOOK sweeps downward through pending requests.");
            for (auto it = upper.rbegin(); it != upper.rend(); ++it)
                serve(*it, "C-LOOK wraps to the highest pending request and continues downward.");
        }
    }
    return pack(algorithm, head, path);
}
