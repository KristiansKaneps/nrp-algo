#ifndef RAY_H
#define RAY_H

#include "Time/Constants.h"

namespace Time {
    class Ray;
    class Range;
    class RangeCollection;

    enum PeriodType {
        RAY,
        RANGE
    };

    std::ostream& operator<<(std::ostream& out, const Ray& ray);

    class Ray {
    public:
        Ray(const Instant& start) : m_Start(start) { } // NOLINT(*-explicit-constructor)

        template<class Duration = std::chrono::system_clock::duration>
        Ray(const std::chrono::time_point<std::chrono::system_clock, Duration>& start) : m_Start(
            std::chrono::time_point_cast<INSTANT_PRECISION>(start)) { } // NOLINT(*-explicit-constructor)

        Ray(const Ray& other) : Ray(other.m_Start) { }

        virtual ~Ray() = default;

        [[nodiscard]] virtual PeriodType type() const { return RAY; }

        [[nodiscard]] const Instant& start() const { return m_Start; }

        virtual bool operator==(const Ray& other) const { return other.type() == RAY && other.m_Start == m_Start; }
        virtual bool operator!=(const Ray& other) const { return other.type() != RAY || other.m_Start != m_Start; }
        virtual bool operator<=(const Ray& other) const { return other.type() == RAY && other.m_Start <= m_Start; }
        virtual bool operator>=(const Ray& other) const { return other.type() == RAY && other.m_Start >= m_Start; }
        virtual bool operator<(const Ray& other) const { return other.type() == RAY && other.m_Start < m_Start; }
        virtual bool operator>(const Ray& other) const { return other.type() == RAY && other.m_Start > m_Start; }

        template<typename DurationType = std::chrono::days, typename TimeZone = const std::chrono::time_zone *>
        [[nodiscard]] std::chrono::time_point<std::chrono::system_clock, DurationType> getDayStartAt(
            const size_t dayIndex, TimeZone zone) const {
            const auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            const auto localRef = std::chrono::floor<std::chrono::days>(zonedStart.get_local_time()) +
                std::chrono::days(dayIndex);
            const auto start1 = std::chrono::time_point_cast<DurationType>(zone->to_sys(localRef));
            const auto start2 = std::chrono::time_point_cast<DurationType>(m_Start);
            return start1 < start2 ? start2 : start1;
        }

        template<typename TimeZone = const std::chrono::time_zone *>
        [[nodiscard]] Ray offsetStartByDayCount(const int32_t offsetDayCount, TimeZone zone) const {
            const auto zonedStart = std::chrono::zoned_time(zone, m_Start);
            const auto localRef = zonedStart.get_local_time() + std::chrono::days(offsetDayCount);
            return {std::chrono::time_point_cast<INSTANT_PRECISION>(zone->to_sys(localRef))};
        }

        template<class Duration = std::chrono::system_clock::duration>
        [[nodiscard]] Range rangeTo(const std::chrono::time_point<std::chrono::system_clock, Duration>& end) const;
        template<class Duration = std::chrono::system_clock::duration>
        [[nodiscard]] Range rangeFrom(const std::chrono::time_point<std::chrono::system_clock, Duration>& start) const;

        template<typename Duration = std::chrono::minutes, typename TimePoint = Instant>
        Duration durationTo(const TimePoint& end) const { return std::chrono::round<Duration>(end - m_Start); }

        [[nodiscard]] virtual bool isStartAdjacentTo(const Ray& other) const { // NOLINT(*-no-recursion)
            if (other.type() == RAY) [[unlikely]] return other.m_Start == m_Start;
            return other.isStartAdjacentTo(*this);
        }

        [[nodiscard]] virtual bool isAdjacentTo(const Ray& other) const { return isStartAdjacentTo(other); }

        [[nodiscard]] virtual bool fullyContains(const Ray& other) const {
            return m_Start <= other.m_Start;
        }

        [[nodiscard]] virtual bool fullyContains(const RangeCollection& other) const;

        [[nodiscard]] virtual bool isFullyContainedBy(const Ray& other) const {
            return m_Start >= other.m_Start;
        }

        [[nodiscard]] virtual bool isFullyContainedBy(const RangeCollection& other) const;

        [[nodiscard]] virtual bool intersects(const Ray& other) const;

        [[nodiscard]] virtual bool intersects(const RangeCollection& other) const;

        [[nodiscard]] Ray getRayIntersection(const Ray& other) const {
            return {other.m_Start < m_Start ? m_Start : other.m_Start};
        }

    protected:
        Instant m_Start;

        friend class Range;
        friend class RangeCollection;
    };

    inline std::ostream& operator<<(std::ostream& out, const Ray& ray) {
        out << '[' << ray.start() << ']';
        return out;
    }
}

template<>
struct std::hash<Time::Ray> { // NOLINT(*-dcl58-cpp)
    std::size_t operator()(const Time::Ray& k) const noexcept {
        using std::size_t;
        using std::hash;
        using std::string;
        using std::chrono_literals::operator ""s;
        return hash<uint64_t>()(k.start().time_since_epoch().count());
    }
};

#endif //RAY_H
