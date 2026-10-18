#include "FIFO.h"

// FIFO: the page that came in first is the first one to go out.
// nextToReplace always points at the frame holding the oldest page.
PageReplacementResult FIFO::replace(std::vector<int> refs, int frameCount)
{
    PageReplacementResult result;
    result.algorithm = getAlgorithmName();
    result.frameCount = frameCount;
    if (frameCount <= 0) return result;

    std::vector<int> frames(frameCount, -1);   // -1 = empty
    int nextToReplace = 0;

    for (int i = 0; i < (int)refs.size(); i++) {
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
            victim = frames[nextToReplace];     // stays -1 if the frame was empty
            frames[nextToReplace] = page;
            nextToReplace = (nextToReplace + 1) % frameCount;
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

std::string FIFO::getAlgorithmName() const { return "FIFO"; }
