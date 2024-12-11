#include "Time/Range.h"

#include "Time/RangeCollection.h"

namespace Time {
    bool Range::intersects(const RangeCollection& other) const { return other.intersects(*this); }
}
