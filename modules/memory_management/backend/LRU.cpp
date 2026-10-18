#include "LRU.h"

// LRU: remove the page that was used longest ago.
// lastUsed[f] stores the reference-string index when frame f was last touched.
PageReplacementResult LRU::replace(std::vector<int> refs, int frameCount)
{
    PageReplacementResult result;
    result.algorithm = getAlgorithmName();
    result.frameCount = frameCount;
    if (frameCount <= 0) return result;

    std::vector<int> frames(frameCount, -1);     // -1 = empty
    std::vector<int> lastUsed(frameCount, -1);

    for (int time = 0; time < (int)refs.size(); time++) {
        int page = refs[time];

        int foundAt = -1;
        for (int f = 0; f < frameCount; f++) {
            if (frames[f] == page) {
                foundAt = f;
                break;
            }
        }

        bool hit = (foundAt != -1);
        int victim = -1;

        if (hit) {
            lastUsed[foundAt] = time;            // used again just now
        } else {
            int slot = -1;
            for (int f = 0; f < frameCount; f++) {
                if (frames[f] == -1) {
                    slot = f;
                    break;
                }
            }

            if (slot == -1) {
                slot = 0;
                for (int f = 1; f < frameCount; f++) {
                    if (lastUsed[f] < lastUsed[slot]) {
                        slot = f;
                    }
                }
                victim = frames[slot];
            }

            frames[slot] = page;
            lastUsed[slot] = time;
        }

        PageStep step;
        step.page = page;
        step.frames = frames;
        step.hit = hit;
        step.victim = victim;
        result.steps.push_back(step);
    }

    calculateValues(result);
    return result;
}

std::string LRU::getAlgorithmName() const { return "LRU"; }
