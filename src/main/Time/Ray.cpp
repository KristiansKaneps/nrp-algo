#include "Time/Ray.h"

#include "Time/Range.h"
#include "Time/RangeCollection.h"

namespace Time {
    inline Range Ray::rangeTo(const Instant& end) const { return Range {m_Start, end}; }

    inline Range Ray::rangeFrom(const Instant& start) const { return Range {start, m_Start}; }

    bool Ray::intersects(const RangeCollection& other) const { return other.intersects(*this); }
}
