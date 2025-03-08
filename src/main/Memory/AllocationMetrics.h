#ifndef ALLOCATIONMETRICS_H
#define ALLOCATIONMETRICS_H

#include <iostream>

namespace Memory {
    struct AllocationMetrics {
        size_t totalAllocated = 0;
        size_t totalFreed = 0;

        [[nodiscard]] size_t usage() const { return totalAllocated - totalFreed; }
    };

    static AllocationMetrics s_AllocationMetrics;

    inline void printMemoryUsage() {
        std::cout << "Memory usage: " << s_AllocationMetrics.usage() << " bytes." << std::endl;
    }
}

#endif //ALLOCATIONMETRICS_H
