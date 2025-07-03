#include "Time/Range.h"

#include "Time/RangeCollection.h"

namespace Time {
    bool Range::fullyContains(const RangeCollection& other) const noexcept { return other.isFullyContainedBy(*this); }

    bool Range::isFullyContainedBy(const RangeCollection& other) const noexcept { return other.fullyContains(*this); }

    bool Range::intersects(const RangeCollection& other) const noexcept { return other.intersects(*this); }

    RangeCollection Range::getIntersection(RangeCollection& other) const noexcept {
        return other.getIntersection(*this);
    }

    RangeCollection Range::getSymmetricDifference(const Range& other) const noexcept {
        return RangeCollection::getSymmetricDifference(*this, other);
    }

    RangeCollection Range::operator+(const Range& rhs) const noexcept { return RangeCollection::add(*this, rhs); }

    RangeCollection Range::operator+(const RangeCollection& rhs) const noexcept { return rhs + *this; }

    RangeCollection Range::operator-(const Range& rhs) const noexcept { return RangeCollection::subtract(*this, rhs); }

    RangeCollection Range::operator-(const RangeCollection& rhs) const noexcept { return RangeCollection::subtract(*this, rhs); }
}
