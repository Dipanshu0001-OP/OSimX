#ifndef OSIMX_PAGE_REPLACER_H
#define OSIMX_PAGE_REPLACER_H

#include <string>
#include <vector>
#include "models/MemoryResult.h"

// Pages are non-negative numbers. -1 means an empty frame.
class PageReplacer {
public:
    virtual ~PageReplacer() = default;
    virtual PageReplacementResult replace(std::vector<int> referenceString,
                                          int frameCount) = 0;
    virtual std::string getAlgorithmName() const = 0;

protected:
    void calculateValues(PageReplacementResult& result);
};

#endif
