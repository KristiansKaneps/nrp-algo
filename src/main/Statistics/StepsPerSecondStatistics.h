#ifndef STEPSPERSECONDSTATISTICS_H
#define STEPSPERSECONDSTATISTICS_H

#include "Statistics.h"
#include <vector>
#include <chrono>

namespace Statistics {
    class StepsPerSecondStatistics : public Statistics {
    public:
        struct Point {
            uint64_t time; // milliseconds since start
            double stepsPerSecond;
            double averageStepsPerSecond;
        };

        StepsPerSecondStatistics() noexcept = default;
        ~StepsPerSecondStatistics() noexcept override = default;

        void write(IO::StatisticsFile& out) const override;

        void startRecording() noexcept {
            using std::chrono_literals::operator ""ms;
            m_StartTime = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() / 1ms);
            m_Points.clear();
        }

        void record(const double currentStepsPerSecond, const double averageStepsPerSecond) noexcept {
            using std::chrono_literals::operator ""ms;
            const uint64_t now = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() / 1ms);
            const uint64_t rel = now - m_StartTime;
            m_Points.push_back(Point{rel, currentStepsPerSecond, averageStepsPerSecond});
        }

        void finishRecording() noexcept { }

        [[nodiscard]] const std::vector<Point>& points() const noexcept { return m_Points; }

    private:
        uint64_t m_StartTime {};
        std::vector<Point> m_Points {};
    };
}

#endif //STEPSPERSECONDSTATISTICS_H


