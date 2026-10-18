#include "Optimal.h"

// Optimal (Belady's): remove the page that will not be needed for the
// longest time in the future. It needs the whole reference string in
// advance, so it works only as a best-case comparison, not a real policy.
PageReplacementResult Optimal::replace(std::vector<int> refs, int frameCount)
{
    PageReplacementResult result;
    result.algorithm = getAlgorithmName();
    result.frameCount = frameCount;
    if (frameCount <= 0) return result;

    std::vector<int> frames(frameCount, -1);     // -1 = empty
    int n = (int)refs.size();

    for (int i = 0; i < n; i++) {
        int page = refs[i];

        bool hit = false;
        for (int f = 0; f < frameCount; f++) {
            if (frames[f] == page) {
                hit = true;
                break;
            }
        }

        int victim = -1;
        if (!hit) {
            int slot = -1;
            for (int f = 0; f < frameCount; f++) {
                if (frames[f] == -1) {
                    slot = f;
                    break;
                }
            }

            if (slot == -1) {
                int farthest = -1;
                for (int f = 0; f < frameCount; f++) {
                    int nextUse = n;             // n means "never used again"
                    for (int j = i + 1; j < n; j++) {
                        if (refs[j] == frames[f]) {
                            nextUse = j;
                            break;
                        }
                    }
                    if (nextUse > farthest) {
                        farthest = nextUse;
                        slot = f;
                    }
                }
                victim = frames[slot];
            }

            frames[slot] = page;
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

std::string Optimal::getAlgorithmName() const { return "Optimal"; }
