#include "Time/Ray.h"

#include "Time/Range.h"
#include "Time/RangeCollection.h"

namespace Time {
    template<class Duration>
    Range Ray::rangeTo(const std::chrono::time_point<std::chrono::system_clock, Duration>& end) const { return Range {m_Start, end}; }

    template<class Duration>
    Range Ray::rangeFrom(const std::chrono::time_point<std::chrono::system_clock, Duration>& start) const { return Range {start, m_Start}; }

    bool Ray::intersects(const RangeCollection& other) const { return other.intersects(*this); }
}
