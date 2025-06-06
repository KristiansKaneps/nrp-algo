#ifndef SCORESTATISTICS_H
#define SCORESTATISTICS_H

#include "Statistics.h"

#include "Score/Score.h"
#include <vector>
#include <chrono>

namespace Statistics {
    class ScoreStatistics : public Statistics {
    public:
        struct Point {
            uint64_t time;
            Score::Score score;
        };

        ScoreStatistics() = default;
        ~ScoreStatistics() override = default;

        [[nodiscard]] const std::vector<Point>& points() const {
            return m_Points;
        }

        [[nodiscard]] Score::Score min() const {
            return m_MinScore;
        }
        [[nodiscard]] Score::Score max() const {
            return m_MaxScore;
        }

        void write(IO::StatisticsFile& out) const override;

        void startRecording(const Score::Score &score) {
            #ifdef ENABLE_SCORE_STATISTICS
            using std::chrono_literals::operator ""ms;
            m_StartTime = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() / 1ms);
            m_MinScore = score;
            m_MaxScore = score;
            m_LastPoint = Point{0, score};
            #endif
        }

        void record(const Score::Score &score) {
            #ifdef ENABLE_SCORE_STATISTICS
            using std::chrono_literals::operator ""ms;
            if (const auto time = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() / 1ms) - m_StartTime; m_LastPoint.time == time) {
                m_LastPoint.score = score;
            } else {
                if (m_MinScore > m_LastPoint.score) m_MinScore = m_LastPoint.score;
                if (m_MaxScore < m_LastPoint.score) m_MaxScore = m_LastPoint.score;
                m_Points.emplace_back(m_LastPoint);
                m_LastPoint = Point{time, score};
            }
            #endif
        }

        void finishRecording() {
            #ifdef ENABLE_SCORE_STATISTICS
            if (m_Points.size() == 0) return;
            if (m_MinScore > m_LastPoint.score) m_MinScore = m_LastPoint.score;
            if (m_MaxScore < m_LastPoint.score) m_MaxScore = m_LastPoint.score;
            m_Points.emplace_back(m_LastPoint);
            #endif
        }

    protected:
        Point m_LastPoint {};

        uint64_t m_StartTime {};
        Score::Score m_MinScore {};
        Score::Score m_MaxScore {};
        std::vector<Point> m_Points {};

        void record(const Point &point) {
            using std::chrono_literals::operator ""ms;
            if (m_MinScore > point.score) m_MinScore = point.score;
            if (m_MaxScore < point.score) m_MaxScore = point.score;
            m_Points.push_back(point);
        }
    };
}

#endif //SCORESTATISTICS_H
