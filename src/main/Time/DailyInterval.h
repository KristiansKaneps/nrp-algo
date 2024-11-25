#ifndef DAILYINTERVAL_H
#define DAILYINTERVAL_H

#include <cassert>
#include <cstdint>
#include <chrono>

#include "Time/Range.h"

namespace Time {
    typedef uint16_t day_minutes_t;

    class DailyInterval {
    public:
        static constexpr day_minutes_t MINUTES_IN_A_DAY = 24 * 60;

        DailyInterval(const day_minutes_t startInMinutes, const day_minutes_t durationInMinutes) :
            m_StartInMinutes(startInMinutes),
            m_DurationInMinutes(durationInMinutes) {
            assert(
                startInMinutes < MINUTES_IN_A_DAY &&
                "Start [minutes] should be in range 0 <= startInMinutes < MINUTES_IN_A_DAY.");
            assert(
                durationInMinutes > 0 && durationInMinutes <= MINUTES_IN_A_DAY &&
                "Duration [minutes] should be in range 0 < durationInMinutes <= MINUTES_IN_A_DAY.");
        }

        DailyInterval(const DailyInterval& other) : DailyInterval(other.m_StartInMinutes, other.m_DurationInMinutes) { }

        ~DailyInterval() = default;

        [[nodiscard]] day_minutes_t startInMinutes() const { return m_StartInMinutes; }
        [[nodiscard]] day_minutes_t durationInMinutes() const { return m_DurationInMinutes; }
        [[nodiscard]] day_minutes_t endInMinutes() const { return m_StartInMinutes + m_DurationInMinutes; }

        [[nodiscard]] std::chrono::duration<uint16_t, std::ratio<60>> start() const {
            return std::chrono::duration<uint16_t, std::ratio<60>>(m_StartInMinutes);
        }

        [[nodiscard]] std::chrono::duration<uint16_t, std::ratio<60>> duration() const {
            return std::chrono::duration<uint16_t, std::ratio<60>>(m_DurationInMinutes);
        }

        [[nodiscard]] std::chrono::duration<uint16_t, std::ratio<60>> end() const {
            return std::chrono::duration<uint16_t, std::ratio<60>>(m_StartInMinutes + m_DurationInMinutes);
        }

        template<typename DurationType = std::chrono::minutes, typename TimeZone = const std::chrono::time_zone *>
        Range toRange(const std::chrono::zoned_time<DurationType, TimeZone>& day) const {
            auto zone = day.get_time_zone();
            auto localRef = std::chrono::floor<std::chrono::days>(day.get_local_time());
            auto start = zone->to_sys(localRef + DailyInterval::start());
            auto end = zone->to_sys(localRef + DailyInterval::end());
            return {start, end};
        }

        [[nodiscard]] bool isStartAdjacentTo(const DailyInterval& other) const {
            return m_StartInMinutes == other.m_StartInMinutes + other.m_DurationInMinutes;
        }

        [[nodiscard]] bool isEndAdjacentTo(const DailyInterval& other) const {
            return m_StartInMinutes + m_DurationInMinutes == other.m_StartInMinutes;
        }

        [[nodiscard]] bool isAdjacentTo(const DailyInterval& other) const {
            return isStartAdjacentTo(other) || isEndAdjacentTo(other);
        }

        /**
         * @param other Interval that is in the same day as this interval.
         * @return true if intersects, false otherwise
         */
        [[nodiscard]] bool intersectsInSameDay(const DailyInterval& other) const {
            return m_StartInMinutes < other.m_StartInMinutes + other.m_DurationInMinutes && m_StartInMinutes +
                m_DurationInMinutes > other.m_StartInMinutes;
        }

        /**
         * @param other Interval that is in the previous day with respect to this interval.
         * @return true if intersects, false otherwise
         */
        [[nodiscard]] bool intersectsOtherFromPrevDay(const DailyInterval& other) const {
            return other.m_StartInMinutes + other.m_DurationInMinutes - MINUTES_IN_A_DAY > m_StartInMinutes;
        }

        /**
         * @param other Interval that is in the next day with respect to this interval.
         * @return true if intersects, false otherwise
         */
        [[nodiscard]] bool intersectsOtherFromNextDay(const DailyInterval& other) const {
            return m_StartInMinutes + m_DurationInMinutes > other.m_StartInMinutes + MINUTES_IN_A_DAY;
        }

    protected:
        const day_minutes_t m_StartInMinutes, m_DurationInMinutes;
    };
}

#endif //DAILYINTERVAL_H
