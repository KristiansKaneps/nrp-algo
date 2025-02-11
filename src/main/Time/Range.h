#ifndef TIMERANGE_H
#define TIMERANGE_H

#include "Time/Ray.h"

namespace Time {
    class Range : public Ray {
    public:
        Range(const Instant& start, const Instant& end) : Ray(start),
                                                          m_End(end) { }

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
        std::chrono::zoned_time<std::chrono::system_clock::duration, TimeZone> getDayAt(const size_t dayIndex, TimeZone zone) const {
            using namespace std::chrono_literals;
            const auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            const auto localDay = floor<std::chrono::days>(zonedStart.get_local_time()) + std::chrono::days(dayIndex);
            const std::chrono::zoned_time<std::chrono::system_clock::duration, TimeZone> zonedDay = std::chrono::zoned_time(zone, zone->to_sys(localDay));
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
            return std::chrono::round<Duration>(m_End - m_Start);
        }

        template<typename Duration = std::chrono::minutes, typename TimeZone = const std::chrono::time_zone *>
        Duration duration(TimeZone zone) const {
            if (m_End == MIN_INSTANT && m_Start == MAX_INSTANT) [[unlikely]] return Duration::zero();
            const auto start = std::chrono::zoned_time(zone, m_Start);
            const auto end = std::chrono::zoned_time(zone, m_End);
            return std::chrono::round<Duration>(std::chrono::floor<Duration>(end.get_local_time()) - std::chrono::floor<Duration>(start.get_local_time()));
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

    protected:
        Instant m_End;

        friend class RangeCollection;
    };

    inline std::ostream& operator<<(std::ostream& out, const Instant& instant) {
        out << InstantToString(instant);
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const Range& range) {
        out << '[' << range.start() << "; " << range.end() << ']';
        return out;
    }
}

#endif //TIMERANGE_H
