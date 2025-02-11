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

        DailyInterval(const char *startAsString, const day_minutes_t durationInMinutes) : DailyInterval(
            parseString(startAsString), durationInMinutes) { }

        /**
         * Unsafe, because `day_minutes_t` is unsigned and duration could be negative, in which case duration will
         * overflow.
         */
        DailyInterval(const char *startAsString, const char *endAsString) : DailyInterval(
            parseString(startAsString), parseString(endAsString) - parseString(startAsString)) { }

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

        [[nodiscard]] std::string startAsString() const {
            const auto hours = static_cast<uint8_t>(m_StartInMinutes / 60);
            const auto minutes = static_cast<uint8_t>(m_StartInMinutes % 60);
            char cStr[5] = {'0', '0', ':', '0', '0'};
            if (hours >= 10) cStr[0] = static_cast<char>(hours / 10) + '0'; // NOLINT(*-narrowing-conversions)
            cStr[1] = static_cast<char>(hours % 10) + '0'; // NOLINT(*-narrowing-conversions)
            if (minutes >= 10) cStr[3] = static_cast<char>(minutes / 10) + '0'; // NOLINT(*-narrowing-conversions)
            cStr[4] = static_cast<char>(minutes % 10) + '0'; // NOLINT(*-narrowing-conversions)
            return {cStr, 5};
        }

        [[nodiscard]] std::string endAsString() const {
            const auto hours = static_cast<uint8_t>((m_StartInMinutes + m_DurationInMinutes) / 60);
            const auto minutes = static_cast<uint8_t>((m_StartInMinutes + m_DurationInMinutes) % 60);
            char cStr[5] = {'0', '0', ':', '0', '0'};
            if (hours >= 10) cStr[0] = static_cast<char>(hours / 10) + '0'; // NOLINT(*-narrowing-conversions)
            cStr[1] = static_cast<char>(hours % 10) + '0'; // NOLINT(*-narrowing-conversions)
            if (minutes >= 10) cStr[3] = static_cast<char>(minutes / 10) + '0'; // NOLINT(*-narrowing-conversions)
            cStr[4] = static_cast<char>(minutes % 10) + '0'; // NOLINT(*-narrowing-conversions)
            return {cStr, 5};
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

        /**
         * @param str Time formatted as `H:i` (for example, `13:30`).
         * @return Parsed daily interval minutes.
         */
        static day_minutes_t parseString(const char *str) {
            return (static_cast<day_minutes_t>(str[0] - '0') * 10 + static_cast<day_minutes_t>(str[1] - '0')) * 60 + (
                static_cast<day_minutes_t>(str[3] - '0') * 10 + static_cast<day_minutes_t>(str[4] - '0'));
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const DailyInterval& dailyInterval) {
        out << '[' << dailyInterval.startAsString() << "; " << dailyInterval.endAsString() << ']';
        return out;
    }
}

#endif //DAILYINTERVAL_H
