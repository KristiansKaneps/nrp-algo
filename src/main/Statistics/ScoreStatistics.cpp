#include "ScoreStatistics.h"

namespace Statistics {
    void ScoreStatistics::write(IO::StatisticsFile& out) const {
        out << "Time;Strict;Hard;Soft" << '\n';
        for (const auto& [time, score] : m_Points) {
            out << time << ';' << score.strict << ';' << score.hard << ';' << score.soft << '\n';
        }
    }
}
