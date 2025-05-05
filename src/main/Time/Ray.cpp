#include "Time/Ray.h"

#include "Time/Range.h"
#include "Time/RangeCollection.h"

namespace Time {
    template<class Duration>
    Range Ray::rangeTo(const std::chrono::time_point<std::chrono::system_clock, Duration>& end) const { return Range {m_Start, end}; }

    template<class Duration>
    Range Ray::rangeFrom(const std::chrono::time_point<std::chrono::system_clock, Duration>& start) const { return Range {start, m_Start}; }

    bool Ray::fullyContains(const RangeCollection& other) const { return other.isFullyContainedBy(*this); }

    bool Ray::isFullyContainedBy(const RangeCollection& other) const { return other.fullyContains(*this); }

    bool Ray::intersects(const Ray& other) const {
        return other.type() == RAY || static_cast<const Range&>(other).intersects(*this); // NOLINT(*-pro-type-static-cast-downcast)
    }

    bool Ray::intersects(const RangeCollection& other) const { return other.intersects(*this); }
}
