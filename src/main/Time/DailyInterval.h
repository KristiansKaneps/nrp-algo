#ifndef DAILYINTERVAL_H
#define DAILYINTERVAL_H

#include <cstdint>
#include <chrono>

#include "Time/Range.h"

namespace Time {
    typedef int16_t day_minutes_t;

    class DailyInterval {
    public:
        static constexpr day_minutes_t MINUTES_IN_A_DAY = 24 * 60;
        static constexpr day_minutes_t MINUTES_IN_TWO_DAYS = 2 * MINUTES_IN_A_DAY;

        DailyInterval(const day_minutes_t startInMinutes, const day_minutes_t durationInMinutes) :
            m_StartInMinutes(startInMinutes),
            m_DurationInMinutes(durationInMinutes) { }

        DailyInterval(const char *startAsString, const day_minutes_t durationInMinutes) : DailyInterval(
            parseString(startAsString), durationInMinutes) { }

        DailyInterval(const char *startAsString, const char *endAsString) : DailyInterval(
            parseString(startAsString),
            static_cast<day_minutes_t>(parseString(endAsString) - parseString(startAsString))) { }

        DailyInterval(const DailyInterval& other) : DailyInterval(other.m_StartInMinutes, other.m_DurationInMinutes) { }

        ~DailyInterval() = default;

        bool operator==(const DailyInterval& other) const {
            return m_StartInMinutes == other.m_StartInMinutes && m_DurationInMinutes == other.m_DurationInMinutes;
        }

        template<typename TimeZone = const std::chrono::time_zone *>
        static DailyInterval fromRange(const Range& range, TimeZone timeZone) {
            using std::chrono_literals::operator ""min;
            const day_minutes_t durationMinutes = range.duration(timeZone) / 1min;
            const auto dayStart = range.getDayStartAt(0, timeZone);
            const day_minutes_t startMinutes = (range.start() - dayStart) / 1min;
            return {startMinutes, durationMinutes};
        }

        [[nodiscard]] day_minutes_t startInMinutes() const { return m_StartInMinutes; }

        [[nodiscard]] day_minutes_t endInMinutes() const {
            return static_cast<day_minutes_t>(m_StartInMinutes + m_DurationInMinutes);
        }

        [[nodiscard]] day_minutes_t durationInMinutes() const { return m_DurationInMinutes; }

        [[nodiscard]] std::chrono::duration<uint16_t, std::ratio<60>> start() const {
            return std::chrono::duration<uint16_t, std::ratio<60>>(m_StartInMinutes);
        }

        [[nodiscard]] std::chrono::duration<uint16_t, std::ratio<60>> end() const {
            return std::chrono::duration<uint16_t, std::ratio<60>>(m_StartInMinutes + m_DurationInMinutes);
        }

        [[nodiscard]] std::chrono::duration<uint16_t, std::ratio<60>> duration() const {
            return std::chrono::duration<uint16_t, std::ratio<60>>(m_DurationInMinutes);
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
         * @param other Interval that is in the offset day with respect to this interval.
         * @param offset Day offset number in respect to this interval's day.
         * @return true if intersects, false otherwise
         */
        [[nodiscard]] bool intersectsOtherInOffsetDay(const DailyInterval& other, const int32_t offset) const {
            return static_cast<int32_t>(m_StartInMinutes) < other.m_StartInMinutes + other.m_DurationInMinutes + offset
                * MINUTES_IN_A_DAY && static_cast<int32_t>(m_StartInMinutes) +
                static_cast<int32_t>(m_DurationInMinutes) > other.m_StartInMinutes + offset * MINUTES_IN_A_DAY;
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
        [[nodiscard]] bool intersectsOtherInPrevDay(const DailyInterval& other) const {
            return other.m_StartInMinutes + other.m_DurationInMinutes - MINUTES_IN_A_DAY > m_StartInMinutes;
        }

        /**
         * @param other Interval that is in the next day with respect to this interval.
         * @return true if intersects, false otherwise
         */
        [[nodiscard]] bool intersectsOtherInNextDay(const DailyInterval& other) const {
            return m_StartInMinutes + m_DurationInMinutes > other.m_StartInMinutes + MINUTES_IN_A_DAY;
        }

        [[nodiscard]] DailyInterval inPreviousDay() const {
            return {
                static_cast<day_minutes_t>(m_StartInMinutes - MINUTES_IN_A_DAY),
                m_DurationInMinutes,
            };
        }

        [[nodiscard]] DailyInterval inNextDay() const {
            return {
                static_cast<day_minutes_t>(m_StartInMinutes + MINUTES_IN_A_DAY),
                m_DurationInMinutes,
            };
        }

        /**
         * Applies padding to this daily interval.
         * @param padding Padding to apply to each side (start and end).
         * @return New daily interval with applied padding.
         */
        [[nodiscard]] DailyInterval withPadding(const day_minutes_t padding) const {
            return {
                static_cast<day_minutes_t>(m_StartInMinutes - padding),
                static_cast<day_minutes_t>(m_DurationInMinutes + 2 * padding),
            };
        }

        /**
         * Applies padding to this daily interval.
         * @param startPadding Padding to apply to start.
         * @param endPadding Padding to apply to end.
         * @return New daily interval with applied padding.
         */
        [[nodiscard]] DailyInterval
        withPadding(const day_minutes_t startPadding, const day_minutes_t endPadding) const {
            return {
                static_cast<day_minutes_t>(m_StartInMinutes - startPadding),
                static_cast<day_minutes_t>(m_DurationInMinutes + startPadding + endPadding),
            };
        }

    protected:
        const day_minutes_t m_StartInMinutes, m_DurationInMinutes;

        /**
         * @param str Time formatted as `H:i` (for example, `13:30`).
         * @return Parsed daily interval minutes.
         */
        static day_minutes_t parseString(const char *str) {
            return static_cast<day_minutes_t>((
                    static_cast<day_minutes_t>(str[0] - '0') * 10 +
                    static_cast<day_minutes_t>(str[1] - '0')) * 60 +
                (
                    static_cast<day_minutes_t>(str[3] - '0') * 10 +
                    static_cast<day_minutes_t>(str[4] - '0')
                ));
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const DailyInterval& dailyInterval) {
        out << '[' << dailyInterval.startAsString() << "; " << dailyInterval.endAsString() << ']';
        return out;
    }
}

template<>
struct std::hash<Time::DailyInterval> {
    std::size_t operator()(const Time::DailyInterval& k) const noexcept {
        using std::size_t;
        using std::hash;
        using std::string;
        return hash<Time::day_minutes_t>()(k.startInMinutes())
            ^ hash<Time::day_minutes_t>()(k.durationInMinutes());
    }
};

#endif //DAILYINTERVAL_H
