#ifndef STATISTICS_H
#define STATISTICS_H

#include "IO/StatisticsFile.h"

namespace Statistics {
    class Statistics {
    public:
        virtual ~Statistics() noexcept = default;

        virtual void write(IO::StatisticsFile& out) const = 0;
    };
}

#endif //STATISTICS_H
