#ifndef TIMERANGE_H
#define TIMERANGE_H

#include "Time/Ray.h"

namespace Time {
    class Range;
    std::ostream& operator<<(std::ostream& out, const Range& range);

    class Range : public Ray {
    public:
        Range(const Instant& start, const Instant& end) : Ray(start),
                                                          m_End(end) { }

        template<class Duration = std::chrono::system_clock::duration>
        Range(const std::chrono::time_point<std::chrono::system_clock, Duration>& start,
              const std::chrono::time_point<std::chrono::system_clock, Duration>& end) : Ray(start),
            m_End(std::chrono::time_point_cast<INSTANT_PRECISION>(end)) { }

        Range(const Range& other) : Range(other.m_Start, other.m_End) { }

        [[nodiscard]] PeriodType type() const override { return RANGE; }

        [[nodiscard]] const Instant& end() const { return m_End; }

        [[nodiscard]] Ray asRayFromStart() const { return {m_Start}; }
        [[nodiscard]] Ray asRayFromEnd() const { return {m_End}; }
        [[nodiscard]] Ray asRay() const { return asRayFromStart(); }

        bool operator==(const Ray& other) const override {
            return (other.type() == RANGE && other.m_Start == m_Start
                    && static_cast<const Range&>(other).m_End == m_End) // NOLINT(*-pro-type-static-cast-downcast)
                || (other.type() == RAY && other.m_Start == m_Start && other.m_Start == m_End);
        }

        bool operator!=(const Ray& other) const override {
            return (other.type() == RANGE && (other.m_Start != m_Start
                    || static_cast<const Range&>(other).m_End != m_End)) // NOLINT(*-pro-type-static-cast-downcast)
                || (other.type() == RAY && (other.m_Start != m_Start || other.m_Start != m_End));
        }

        bool operator<=(const Ray& other) const override {
            if (other.type() == RAY) [[unlikely]] return other.m_Start >= m_End;
            if (other.type() == RANGE) [[likely]] { return m_Start <= other.m_Start; }
            return false;
        }

        bool operator>=(const Ray& other) const override {
            if (other.type() == RAY) [[unlikely]] return other.m_Start == m_End;
            if (other.type() == RANGE) [[likely]] { return m_Start >= other.m_Start; }
            return false;
        }

        bool operator<(const Ray& other) const override {
            if (other.type() == RAY) [[unlikely]] return other.m_Start > m_End;
            if (other.type() == RANGE) [[likely]] { return m_Start < other.m_Start; }
            return false;
        }

        bool operator>(const Ray& other) const override {
            if (other.type() == RAY) [[unlikely]] return false;
            if (other.type() == RANGE) [[likely]] { return m_Start > other.m_Start; }
            return false;
        }

        template<typename TimeZone = const std::chrono::time_zone *>
        std::chrono::zoned_time<std::chrono::system_clock::duration, TimeZone> getDayAt(
            const size_t dayIndex, TimeZone zone) const {
            using namespace std::chrono_literals;
            const auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            const auto localDay = floor<std::chrono::days>(zonedStart.get_local_time()) + std::chrono::days(dayIndex);
            const std::chrono::zoned_time<std::chrono::system_clock::duration, TimeZone> zonedDay =
                std::chrono::zoned_time(zone, zone->to_sys(localDay));
            return zonedDay;
        }

        template<typename TimeZone = const std::chrono::time_zone *>
        Range getDayRangeAt(const size_t dayIndex, TimeZone zone) const {
            using namespace std::chrono_literals;
            const auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            const auto localRef = std::chrono::floor<std::chrono::days>(zonedStart.get_local_time()) +
                std::chrono::days(dayIndex);
            const auto start = zone->to_sys(localRef);
            const auto end = zone->to_sys(localRef + 24h);
            return {start < m_Start ? m_Start : start, end > m_End ? m_End : end};
        }

        template<typename Duration = std::chrono::minutes>
        Duration duration() const {
            if (m_End == MIN_INSTANT && m_Start == MAX_INSTANT) [[unlikely]] return Duration::zero();
            return std::chrono::floor<Duration>(m_End - m_Start);
        }

        template<typename Duration = std::chrono::minutes, typename TimeZone = const std::chrono::time_zone *>
        Duration duration(TimeZone zone) const {
            if (m_End == MIN_INSTANT && m_Start == MAX_INSTANT) [[unlikely]] return Duration::zero();
            const auto start = std::chrono::zoned_time(zone, m_Start);
            const auto end = std::chrono::zoned_time(zone, m_End);
            return std::chrono::round<Duration>(
                std::chrono::floor<Duration>(end.get_local_time()) - std::chrono::floor<Duration>(
                    start.get_local_time()));
        }

        template<typename TimeZone = const std::chrono::time_zone *>
        int32_t getWorkdayCount(TimeZone zone) const { return getDayCount(zone, 0x1f); }

        template<typename TimeZone = const std::chrono::time_zone *>
        int32_t getDayCount(TimeZone zone, const uint8_t weekdayBitMask) const {
            if ((m_End == MIN_INSTANT && m_Start == MAX_INSTANT) || m_End == m_Start) [[unlikely]] return 0;
            auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            auto zonedEnd = std::chrono::zoned_time(zone, m_End);
            int32_t sign = 1;
            if (zonedStart.get_sys_time() > zonedEnd.get_sys_time()) [[unlikely]] {
                std::swap(zonedStart, zonedEnd);
                sign = -1;
            }

            int32_t dayCount = 0;

            constexpr auto oneDay = std::chrono::days(1);

            for (
                auto zonedInstant = std::chrono::zoned_time(
                    zone, std::chrono::floor<std::chrono::days>(zonedStart.get_local_time()));
                zonedInstant.get_sys_time() < zonedEnd.get_sys_time();
                zonedInstant = std::chrono::zoned_time(
                    zone, std::chrono::floor<std::chrono::days>(zonedInstant.get_local_time() + oneDay))
            ) {
                if (!(weekdayBitMask >> InstantToWeekday(zonedInstant) & 1)) continue;
                dayCount += 1;
            }

            return dayCount * sign;
        }

        template<typename TimeZone = const std::chrono::time_zone *>
        float getPartialWorkdayCount(TimeZone zone) const { return getPartialDayCount(zone, 0x1f); }

        template<typename TimeZone = const std::chrono::time_zone *>
        float getPartialDayCount(TimeZone zone, const uint8_t weekdayBitMask) const {
            if ((m_End == MIN_INSTANT && m_Start == MAX_INSTANT) || m_End == m_Start) [[unlikely]] return 0;
            auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            auto zonedEnd = std::chrono::zoned_time(zone, m_End);
            int32_t sign = 1;
            if (zonedStart.get_sys_time() > zonedEnd.get_sys_time()) [[unlikely]] {
                std::swap(zonedStart, zonedEnd);
                sign = -1;
            }

            float dayCount = 0.0f;

            constexpr auto oneDay = std::chrono::days(1);

            const auto localStartDayTimePoint = std::chrono::floor<std::chrono::days>(zonedStart.get_local_time());
            const auto localEndDayTimePoint = std::chrono::floor<std::chrono::days>(zonedEnd.get_local_time());
            auto startDay = std::chrono::zoned_time(zone, localStartDayTimePoint);
            auto endDay = std::chrono::zoned_time(zone, localEndDayTimePoint);
            auto nextStartDay = std::chrono::zoned_time(
                zone, std::chrono::floor<std::chrono::days>(localStartDayTimePoint + oneDay));

            if (weekdayBitMask >> InstantToWeekday(zonedStart) & 1) {
                const auto fullDayDuration = std::chrono::duration<float>(
                    nextStartDay.get_sys_time() - startDay.get_sys_time());
                if (fullDayDuration > fullDayDuration.zero()) {
                    dayCount += (nextStartDay.get_sys_time() - zonedStart.get_sys_time()) / fullDayDuration;
                }
            }

            for (
                auto day = nextStartDay;
                day.get_sys_time() < endDay.get_sys_time();
                day = std::chrono::zoned_time(
                    zone, std::chrono::floor<std::chrono::days>(day.get_local_time() + oneDay))
            ) {
                if (!(weekdayBitMask >> InstantToWeekday(day) & 1)) continue;
                dayCount += 1.0f;
            }

            if (weekdayBitMask >> InstantToWeekday(zonedEnd) & 1) {
                const auto fullDayDuration = std::chrono::duration<float>(
                    std::chrono::zoned_time(zone, std::chrono::ceil<std::chrono::days>(zonedEnd.get_local_time())).
                    get_sys_time() - endDay.get_sys_time());
                if (fullDayDuration > fullDayDuration.zero()) {
                    dayCount += (zonedEnd.get_sys_time() - endDay.get_sys_time()) / fullDayDuration;
                }
            }

            return dayCount * static_cast<float>(sign);
        }

        [[nodiscard]] bool isStartAdjacentTo(const Ray& other) const override {
            if (other.type() == RAY) [[unlikely]] return other.m_Start == m_Start;
            if (other.type() == RANGE) [[likely]] {
                const auto r = static_cast<const Range&>(other); // NOLINT(*-pro-type-static-cast-downcast)
                return r.m_End == m_Start;
            }
            return other.isStartAdjacentTo(*this);
        }

        [[nodiscard]] virtual bool isEndAdjacentTo(const Ray& other) const {
            if (other.type() == RAY) [[unlikely]] return other.m_Start == m_End;
            if (other.type() == RANGE) [[likely]] { return other.m_Start == m_End; }
            return other.isStartAdjacentTo(*this);
        }

        [[nodiscard]] bool isAdjacentTo(const Ray& other) const override {
            return isStartAdjacentTo(other) || isEndAdjacentTo(other);
        }

        [[nodiscard]] bool intersects(const Ray& other) const override {
            if (other.type() == RAY) [[unlikely]]
                return other.m_Start < m_End;
            if (other.type() == RANGE) [[likely]] {
                const auto r = static_cast<const Range&>(other); // NOLINT(*-pro-type-static-cast-downcast)
                return r.m_Start < m_End && r.m_End > m_Start;
            }
            return other.intersects(*this);
        }

        [[nodiscard]] bool intersects(const RangeCollection& other) const override;

        [[nodiscard]] std::unique_ptr<const Range> getIntersection(const Ray& other) const {
            if (other.type() == RAY) [[unlikely]] {
                if (m_End <= other.m_Start) return std::unique_ptr<const Range>(nullptr);
                return std::make_unique<const Range>(m_Start > other.m_Start ? m_Start : other.m_Start, m_End);
            }
            if (other.type() == RANGE) [[likely]] {
                const auto r = static_cast<const Range&>(other); // NOLINT(*-pro-type-static-cast-downcast)
                if (r.m_Start >= m_End || m_Start >= r.m_End) return std::unique_ptr<const Range>(nullptr);
                return std::make_unique<const Range>(m_Start > r.m_Start ? m_Start : r.m_Start, m_End > r.m_End ? r.m_End : m_End);
            }
            return std::unique_ptr<const Range>(nullptr);
        }

    protected:
        Instant m_End;

        friend class RangeCollection;
    };

    inline std::ostream& operator<<(std::ostream& out, const Range& range) {
        out << '[' << range.start() << "; " << range.end() << ']';
        return out;
    }
}

template<>
struct std::hash<Time::Range> {
    std::size_t operator()(const Time::Range& k) const noexcept {
        using std::size_t;
        using std::hash;
        using std::string;
        using std::chrono_literals::operator ""s;
        return hash<uint64_t>()(k.start().time_since_epoch().count())
            ^ hash<uint64_t>()(k.end().time_since_epoch().count());
    }
};

#endif //TIMERANGE_H
