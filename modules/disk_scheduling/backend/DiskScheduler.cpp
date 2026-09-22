#include "DiskScheduler.h"
#include <algorithm>
#include <cmath>

DiskResult DiskScheduler::pack(const std::string& algorithm, int head, const std::vector<Move>& path)
{
    DiskResult result;
    result.algorithm = algorithm;
    result.sequence.push_back(head);

    std::vector<int> remaining;
    for (const auto& move : path) remaining.push_back(move.to);

    std::vector<int> visited;
    int from = head;
    int cumulative = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        const auto& move = path[i];
        const int distance = std::abs(move.to - from);
        cumulative += distance;
        if (!remaining.empty()) remaining.erase(remaining.begin());
        visited.push_back(move.to);

        DiskStep step;
        step.index = static_cast<int>(i);
        step.from = from;
        step.to = move.to;
        step.distance = distance;
        step.cumulative = cumulative;
        step.remaining = remaining;
        step.visited = visited;
        step.reason = move.reason;
        result.steps.push_back(step);
        result.sequence.push_back(move.to);
        from = move.to;
    }

    result.totalSeek = cumulative;
    result.averageSeek = path.empty() ? 0.0 : static_cast<double>(cumulative) / path.size();
    return result;
}

DiskResult DiskScheduler::schedule(const std::string& algorithm,
                                   int head,
                                   const std::vector<int>& requests,
                                   int maxCylinder,
                                   bool upward)
{
    std::vector<int> req;
    for (int r : requests)
        if (r >= 0 && r <= maxCylinder) req.push_back(r);

    if (req.empty()) return pack(algorithm, head, {});

    std::vector<Move> path;

    if (algorithm == "FCFS") {
        for (std::size_t i = 0; i < req.size(); ++i) {
            path.push_back({req[i], i == 0
                ? "FCFS starts with the first request in the queue."
                : "FCFS continues in arrival order, without reordering the queue."});
        }
    } else if (algorithm == "SSTF") {
        std::vector<int> pending = req;
        int position = head;
        while (!pending.empty()) {
            auto best = std::min_element(pending.begin(), pending.end(), [position](int a, int b) {
                const int da = std::abs(a - position);
                const int db = std::abs(b - position);
                return da == db ? a < b : da < db;
            });
            const int target = *best;
            path.push_back({target, "SSTF chooses the pending request closest to the current head."});
            position = target;
            pending.erase(best);
        }
    } else {
        std::vector<int> lower;
        std::vector<int> upper;
        for (int r : req) (r < head ? lower : upper).push_back(r);
        std::sort(lower.begin(), lower.end());
        std::sort(upper.begin(), upper.end());

        if (algorithm == "SCAN") {
            if (upward) {
                for (int r : upper) path.push_back({r, "SCAN moves upward and serves the next request."});
                if (!lower.empty()) {
                    if (path.empty() || path.back().to != maxCylinder)
                        path.push_back({maxCylinder, "SCAN reaches the disk boundary before reversing direction."});
                    for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                        path.push_back({*it, "SCAN has reversed and now serves requests on the downward sweep."});
                }
            } else {
                for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                    path.push_back({*it, "SCAN moves downward and serves the next request."});
                if (!upper.empty()) {
                    if (path.empty() || path.back().to != 0)
                        path.push_back({0, "SCAN reaches cylinder 0 before reversing direction."});
                    for (int r : upper)
                        path.push_back({r, "SCAN has reversed and now serves requests on the upward sweep."});
                }
            }
        } else if (algorithm == "CSCAN") {
            if (upward) {
                for (int r : upper) path.push_back({r, "C-SCAN moves upward and serves the next request."});
                if (!lower.empty()) {
                    if (path.empty() || path.back().to != maxCylinder)
                        path.push_back({maxCylinder, "C-SCAN reaches the upper boundary."});
                    path.push_back({0, "C-SCAN wraps around to cylinder 0 without serving requests during the return."});
                    for (int r : lower)
                        path.push_back({r, "C-SCAN resumes the upward sweep after the circular wrap."});
                }
            } else {
                for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                    path.push_back({*it, "C-SCAN moves downward and serves the next request."});
                if (!upper.empty()) {
                    if (path.empty() || path.back().to != 0)
                        path.push_back({0, "C-SCAN reaches cylinder 0."});
                    path.push_back({maxCylinder, "C-SCAN wraps to the far end of the disk."});
                    for (auto it = upper.rbegin(); it != upper.rend(); ++it)
                        path.push_back({*it, "C-SCAN resumes the downward sweep after the circular wrap."});
                }
            }
        } else if (algorithm == "LOOK") {
            if (upward) {
                for (int r : upper) path.push_back({r, "LOOK sweeps upward only as far as the last outstanding request."});
                for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                    path.push_back({*it, "LOOK reverses at the last request instead of travelling to the boundary."});
            } else {
                for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                    path.push_back({*it, "LOOK sweeps downward only as far as the last outstanding request."});
                for (int r : upper)
                    path.push_back({r, "LOOK reverses at the last request instead of travelling to the boundary."});
            }
        } else { // CLOOK
            if (upward) {
                for (int r : upper) path.push_back({r, "C-LOOK sweeps upward through the pending requests."});
                for (int r : lower) path.push_back({r, "C-LOOK wraps to the lowest pending request, then continues upward."});
            } else {
                for (auto it = lower.rbegin(); it != lower.rend(); ++it)
                    path.push_back({*it, "C-LOOK sweeps downward through the pending requests."});
                for (auto it = upper.rbegin(); it != upper.rend(); ++it)
                    path.push_back({*it, "C-LOOK wraps to the highest pending request, then continues downward."});
            }
        }
    }

    return pack(algorithm, head, path);
}
