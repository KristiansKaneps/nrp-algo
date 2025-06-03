#ifndef SCORESTATISTICS_H
#define SCORESTATISTICS_H

#include "Score/Score.h"
#include <vector>
#include <chrono>

namespace Statistics {
    class ScoreStatistics {
    public:
        struct Point {
            uint64_t time;
            Score::Score score;
        };

        ScoreStatistics() = default;
        ~ScoreStatistics() = default;

        [[nodiscard]] std::vector<Point> points() const { return m_Points; }

        [[nodiscard]] Score::Score min() const { return m_MinScore; }
        [[nodiscard]] Score::Score max() const { return m_MaxScore; }

        void recordFirstPoint(const Score::Score &score) {
            using std::chrono_literals::operator ""ms;
            m_StartTime = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() / 1ms);
            m_MinScore = score;
            m_MaxScore = score;
            m_Points.emplace_back(Point{0, score});
        }

        void record(const Score::Score &score) {
            using std::chrono_literals::operator ""ms;
            if (m_MinScore > score) m_MinScore = score;
            if (m_MaxScore < score) m_MaxScore = score;
            m_Points.emplace_back(Point{static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() / 1ms) - m_StartTime, score});
        }

    protected:
        uint64_t m_StartTime {};
        Score::Score m_MinScore {};
        Score::Score m_MaxScore {};
        std::vector<Point> m_Points {};
    };
}

#endif //SCORESTATISTICS_H
