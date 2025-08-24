#include "StepsPerSecondStatistics.h"

namespace Statistics {
    void StepsPerSecondStatistics::write(IO::StatisticsFile& out) const {
        out << "Time;StepsPerSecond;AverageStepsPerSecond" << '\n';
        for (const auto& p : m_Points) {
            out << p.time << ';' << static_cast<int64_t>(p.stepsPerSecond) << ';'
                << static_cast<int64_t>(p.averageStepsPerSecond) << '\n';
        }
    }
}
