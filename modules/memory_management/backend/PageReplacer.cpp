#include "PageReplacer.h"

// Counts hits and faults from the steps and works out the ratios.
void PageReplacer::calculateValues(PageReplacementResult& result)
{
    result.hits = 0;
    result.faults = 0;

    for (int i = 0; i < (int)result.steps.size(); i++) {
        if (result.steps[i].hit) {
            result.hits++;
        } else {
            result.faults++;
        }
    }

    int total = result.hits + result.faults;
    if (total > 0) {
        result.hitRatio = (double)result.hits / total;
        result.faultRatio = (double)result.faults / total;
    } else {
        result.hitRatio = 0.0;
        result.faultRatio = 0.0;
    }
}
