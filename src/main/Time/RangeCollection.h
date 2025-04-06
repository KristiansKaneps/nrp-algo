#ifndef RANGECOLLECTION_H
#define RANGECOLLECTION_H

#include "Time/Range.h"

#include <algorithm>
#include <vector>
#include <utility>

namespace Time {
    class RangeCollection {
    public:
        RangeCollection() = default;

        explicit RangeCollection(const size_t capacity) : RangeCollection() { m_Ranges.reserve(capacity); }

        RangeCollection(const RangeCollection& other) = default;

        virtual ~RangeCollection() = default;

        virtual void clear() {
            m_Ranges.clear();
            m_ResolveBoundsOnGet = false;
            m_MinBound = MAX_INSTANT;
            m_MaxBound = MIN_INSTANT;
        }

        [[nodiscard]] virtual size_t size() const { return m_Ranges.size(); }

        [[nodiscard]] virtual const std::vector<Range>& ranges() const { return m_Ranges; }

        [[nodiscard]] Range bounds() {
            if (m_ResolveBoundsOnGet) {
                m_MinBound = MAX_INSTANT, m_MaxBound = MIN_INSTANT;
                for (const auto& range : m_Ranges) {
                    if (range.m_Start < m_MinBound) m_MinBound = range.m_Start;
                    if (range.m_End > m_MaxBound) m_MaxBound = range.m_End;
                }
                m_ResolveBoundsOnGet = false;
            }
            return Range {m_MinBound, m_MaxBound};
        }

        virtual void add(const Range& range) {
            if (!m_ResolveBoundsOnGet) {
                if (range.m_Start < m_MinBound) m_MinBound = range.m_Start;
                if (range.m_End > m_MaxBound) m_MaxBound = range.m_End;
            }
            insert(range);
        }

        virtual void addAll(const RangeCollection& other) {
            for (const auto& range : other.m_Ranges) {
                add(range);
            }
        }

        virtual void remove(const Range& range) {
            m_ResolveBoundsOnGet = range.m_Start <= m_MinBound || range.m_End >= m_MaxBound;
            for (auto it = m_Ranges.cbegin(); it != m_Ranges.cend(); ++it) {
                if (*it == range) [[unlikely]] {
                    m_Ranges.erase(it);
                    return;
                }
            }
        }

        virtual void removeAll(const RangeCollection& other) {
            for (const auto& range : other.m_Ranges) {
                remove(range);
            }
        }

        virtual void subtract(const Range& range) {
            m_ResolveBoundsOnGet = false;
            size_t additionalRangeCollectionCapacity = 0;
            RangeCollection additionalRangeCollection(0);
            additionalRangeCollection.m_ResolveBoundsOnGet = false;
            for (auto it = m_Ranges.begin(); it != m_Ranges.end();) {
                const auto &r1 = *it;
                const RangeCollection subtracted = r1 - range;
                if (subtracted.m_Ranges.size() == 0) {
                    it += 1;
                    continue;
                }
                if (subtracted.m_Ranges.size() == 1) {
                    const auto &r2 = subtracted.m_Ranges[0];
                    *it = r2;
                    it += 1;
                    m_MinBound = r2.m_Start < m_MinBound ? r2.m_Start : m_MinBound;
                    m_MaxBound = r2.m_End < m_MaxBound ? m_MaxBound : r2.m_End;
                    continue;
                }
                const auto &r2 = subtracted.m_Ranges[0];
                *it = r2;
                it += 1;
                m_MinBound = r2.m_Start < m_MinBound ? r2.m_Start : m_MinBound;
                m_MaxBound = r2.m_End < m_MaxBound ? m_MaxBound : r2.m_End;
                additionalRangeCollectionCapacity += subtracted.m_Ranges.size() - 1;
                additionalRangeCollection.reserve(additionalRangeCollectionCapacity);
                for (auto it2 = subtracted.m_Ranges.begin() + 1; it2 != subtracted.m_Ranges.end(); ++it2) {
                    const auto &r3 = *it2;
                    additionalRangeCollection.m_Ranges.emplace_back(r3);
                    additionalRangeCollection.m_MinBound = r3.m_Start < additionalRangeCollection.m_MinBound ? r3.m_Start : additionalRangeCollection.m_MinBound;
                    additionalRangeCollection.m_MaxBound = r3.m_End < additionalRangeCollection.m_MaxBound ? additionalRangeCollection.m_MaxBound : r3.m_End;
                }
            }
            if (additionalRangeCollection.m_Ranges.size() > 0) {
                m_Ranges.reserve(m_Ranges.size() + additionalRangeCollection.m_Ranges.size());
                for (const auto &r : additionalRangeCollection.m_Ranges) {
                    insert(r);
                }
                m_MinBound = additionalRangeCollection.m_MinBound < m_MinBound ? additionalRangeCollection.m_MinBound : m_MinBound;
                m_MaxBound = additionalRangeCollection.m_MaxBound < m_MaxBound ? m_MaxBound : additionalRangeCollection.m_MaxBound;
            }
            m_Ranges.shrink_to_fit();
        }

        virtual void subtractAll(const RangeCollection& other) {
            for (const auto& range : other.m_Ranges) {
                subtract(range);
            }
        }

        template<typename Duration = std::chrono::minutes>
        Duration duration() const {
            Duration duration = Duration::zero();
            for (const auto& range : m_Ranges) duration += range.duration<Duration>();
            return duration;
        }

        template<typename Duration = std::chrono::minutes, typename TimeZone = const std::chrono::time_zone *>
        Duration duration(TimeZone zone) const {
            Duration duration = Duration::zero();
            for (const auto& range : m_Ranges) duration += range.duration<Duration>(zone);
            return duration;
        }

        [[nodiscard]] bool fullyContains(const Ray& other) const {
            if (other.type() == RAY) [[unlikely]] {
                return std::ranges::all_of(m_Ranges, [other](const Range& range) -> bool {
                    return range.fullyContains(other);
                });
            }
            const auto &r = static_cast<const Range&>(other); // NOLINT(*-pro-type-static-cast-downcast)
            return std::ranges::all_of(m_Ranges.cbegin(), m_Ranges.cend(), [r](const Range& range) -> bool {
                return range.fullyContains(r);
            });
        }

        [[nodiscard]] bool fullyContains(const RangeCollection& other) const {
            return std::ranges::all_of(m_Ranges.cbegin(), m_Ranges.cend(), [other](const Range& range) -> bool {
                return other.isFullyContainedBy(range);
            });
        }

        [[nodiscard]] bool isFullyContainedBy(const Ray& other) const {
            if (other.type() == RAY) [[unlikely]] {
                return std::ranges::all_of(m_Ranges, [other](const Range& range) -> bool {
                    return other.fullyContains(range);
                });
            }
            const auto &r = static_cast<const Range&>(other); // NOLINT(*-pro-type-static-cast-downcast)
            return std::ranges::all_of(m_Ranges.cbegin(), m_Ranges.cend(), [r](const Range& range) -> bool {
                return r.fullyContains(range);
            });
        }

        [[nodiscard]] bool isFullyContainedBy(const RangeCollection& other) const {
            return std::ranges::all_of(m_Ranges.cbegin(), m_Ranges.cend(), [other](const Range& range) -> bool {
                return other.fullyContains(range);
            });
        }

        [[nodiscard]] bool intersects(const Ray& ray) const {
            if (ray.type() == RAY) [[unlikely]] {
                return std::ranges::any_of(m_Ranges, [ray](const Range& range) -> bool {
                    return range.m_End > ray.m_Start;
                });
            }
            const auto &r = static_cast<const Range&>(ray); // NOLINT(*-pro-type-static-cast-downcast)
            return std::ranges::any_of(m_Ranges.cbegin(), m_Ranges.cend(), [r](const Range& range) -> bool {
                return r.intersects(range);
            });
        }

        [[nodiscard]] bool intersects(const RangeCollection& other) const {
            return std::ranges::any_of(m_Ranges.cbegin(), m_Ranges.cend(), [other](const Range& range) -> bool {
                return other.intersects(range);
            });
        }

        RangeCollection getIntersection(const Range& other) {
            if (!bounds().intersects(other)) return RangeCollection(0);
            RangeCollection result(m_Ranges.size());
            for (const auto& range : m_Ranges) {
                if (!range.intersects(other)) continue;
                const auto intersection = range.getIntersectionUnsafe(other);
                result.add(intersection);
            }
            return result;
        }

        RangeCollection getIntersection(RangeCollection& other) {
            if (!bounds().intersects(other.bounds())) return RangeCollection(0);
            RangeCollection result(m_Ranges.size());
            for(const auto &range1 : m_Ranges) {
                for (const auto &range2 : other.m_Ranges) {
                    if (!range1.intersects(range2)) continue;
                    result.add(range1.getIntersectionUnsafe(range2));
                }
            }
            return result;
        }

        RangeCollection operator+(const Range& rhs) const {
            RangeCollection result = *this;
            result.reserve(m_Ranges.size() + 1);
            result.add(rhs);
            return result;
        }

        RangeCollection operator-(const Range& rhs) const {
            size_t capacity = 0;
            RangeCollection result(0);
            result.m_ResolveBoundsOnGet = false;
            for (const auto &range : m_Ranges) {
                const RangeCollection subtracted = range - rhs;
                if (subtracted.size() == 0) continue;
                capacity += subtracted.size();
                result.reserve(capacity);
                for (const auto &r : subtracted.m_Ranges) {
                    result.m_Ranges.push_back(r);
                    result.m_MinBound = r.m_Start < result.m_MinBound ? r.m_Start : result.m_MinBound;
                    result.m_MaxBound = r.m_End < result.m_MaxBound ? result.m_MaxBound : r.m_End;
                }
            }
            return result;
        }

    protected:
        std::vector<Range> m_Ranges;
        bool m_ResolveBoundsOnGet = false;
        Instant m_MinBound = MAX_INSTANT, m_MaxBound = MIN_INSTANT;

        constexpr void reserve(const size_t newCapacity) {
            m_Ranges.reserve(newCapacity);
        }

        constexpr void insert(const Range& range) {
            m_Ranges.insert(std::ranges::upper_bound(std::as_const(m_Ranges), range), range);
        }

        static RangeCollection add(const Range& lhs, const Range& rhs) {
            if (!lhs.intersects(rhs)) {
                RangeCollection result(2);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back(lhs);
                result.m_Ranges.push_back(rhs);
                result.m_MinBound = lhs.m_Start < rhs.m_Start ? lhs.m_Start : rhs.m_Start;
                result.m_MaxBound = lhs.m_End < rhs.m_End ? rhs.m_End : lhs.m_End;
                return result;
            }
            if (lhs.fullyContains(rhs)) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back(lhs);
                result.m_MinBound = lhs.m_Start;
                result.m_MaxBound = lhs.m_End;
            }
            if (rhs.fullyContains(lhs)) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back(rhs);
                result.m_MinBound = rhs.m_Start;
                result.m_MaxBound = rhs.m_End;
            }
            if (lhs.isStartAdjacentTo(rhs)) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back({rhs.m_Start, lhs.m_End});
                result.m_MinBound = rhs.m_Start;
                result.m_MaxBound = lhs.m_End;
                return result;
            }
            if (rhs.isStartAdjacentTo(lhs)) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back({lhs.m_Start, rhs.m_End});
                result.m_MinBound = lhs.m_Start;
                result.m_MaxBound = rhs.m_End;
                return result;
            }
            if (lhs.m_Start < rhs.m_Start) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back({lhs.m_Start, rhs.m_End});
                result.m_MinBound = lhs.m_Start;
                result.m_MaxBound = rhs.m_End;
                return result;
            }
            if (rhs.m_Start < lhs.m_Start) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back({rhs.m_Start, lhs.m_End});
                result.m_MinBound = rhs.m_Start;
                result.m_MaxBound = lhs.m_End;
                return result;
            }
            return RangeCollection(0);
        }

        static RangeCollection subtract(const Range& lhs, const Range& rhs) {
            if (!lhs.intersects(rhs)) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back(lhs);
                result.m_MinBound = lhs.m_Start;
                result.m_MaxBound = lhs.m_End;
                return result;
            }
            if (rhs.fullyContains(lhs)) {
                return RangeCollection(0);
            }
            if (lhs.m_Start < rhs.m_Start) {
                if (lhs.m_End <= rhs.m_End) {
                    RangeCollection result(1);
                    result.m_ResolveBoundsOnGet = false;
                    result.m_Ranges.push_back({lhs.m_Start, rhs.m_Start});
                    result.m_MinBound = lhs.m_Start;
                    result.m_MaxBound = rhs.m_Start;
                    return result;
                }
                RangeCollection result(2);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back({lhs.m_Start, rhs.m_Start});
                result.m_Ranges.push_back({rhs.m_End, lhs.m_End});
                result.m_MinBound = lhs.m_Start;
                result.m_MaxBound = lhs.m_End;
                return result;
            }
            if (rhs.m_Start < lhs.m_Start) {
                RangeCollection result(1);
                result.m_ResolveBoundsOnGet = false;
                result.m_Ranges.push_back({rhs.m_End, lhs.m_End});
                result.m_MinBound = rhs.m_End;
                result.m_MaxBound = lhs.m_End;
                return result;
            }
            return RangeCollection(0);
        }

        static RangeCollection subtract(const Range &lhs, const RangeCollection &rhs) {
            RangeCollection result(1);
            result.m_ResolveBoundsOnGet = false;
            result.m_Ranges.push_back(lhs);
            result.m_MinBound = lhs.m_Start;
            result.m_MaxBound = lhs.m_End;
            result.subtractAll(rhs);
            return result;
        }

        static RangeCollection getSymmetricDifference(const Range& range1, const Range& range2) {
            // auto difference = subtract(range1, range2);
            // difference.addAll(subtract(range2, range1));
            // return difference;
            if (!range1.intersects(range2)) {
                RangeCollection symmetricDifference(2);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back(range1);
                symmetricDifference.m_Ranges.push_back(range2);
                symmetricDifference.m_MinBound = range1.m_Start < range2.m_Start ? range1.m_Start : range2.m_Start;
                symmetricDifference.m_MaxBound = range1.m_End < range2.m_End ? range2.m_End : range1.m_End;
                return symmetricDifference;
            }
            if (range1.m_Start < range2.m_Start && range2.m_End < range1.m_End) {
                RangeCollection symmetricDifference(2);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range1.m_Start, range2.m_Start});
                symmetricDifference.m_Ranges.push_back({range2.m_End, range1.m_End});
                symmetricDifference.m_MinBound = range1.m_Start;
                symmetricDifference.m_MaxBound = range1.m_End;
                return symmetricDifference;
            }
            if (range2.m_Start < range1.m_Start && range1.m_End < range2.m_End) {
                RangeCollection symmetricDifference(2);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range2.m_Start, range1.m_Start});
                symmetricDifference.m_Ranges.push_back({range1.m_End, range2.m_End});
                symmetricDifference.m_MinBound = range2.m_Start;
                symmetricDifference.m_MaxBound = range2.m_End;
                return symmetricDifference;
            }
            if (range1.m_Start < range2.m_Start && range1.m_End < range2.m_End) {
                RangeCollection symmetricDifference(2);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range1.m_Start, range2.m_Start});
                symmetricDifference.m_Ranges.push_back({range1.m_End, range2.m_End});
                symmetricDifference.m_MinBound = range1.m_Start;
                symmetricDifference.m_MaxBound = range2.m_End;
                return symmetricDifference;
            }
            if (range2.m_Start < range1.m_Start && range2.m_End < range1.m_End) {
                RangeCollection symmetricDifference(2);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range2.m_Start, range1.m_Start});
                symmetricDifference.m_Ranges.push_back({range2.m_End, range1.m_End});
                symmetricDifference.m_MinBound = range2.m_Start;
                symmetricDifference.m_MaxBound = range1.m_End;
                return symmetricDifference;
            }
            if (range1.m_Start == range2.m_Start && range1.m_End < range2.m_End) {
                RangeCollection symmetricDifference(1);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range1.m_End, range2.m_End});
                symmetricDifference.m_MinBound = range2.m_Start;
                symmetricDifference.m_MaxBound = range2.m_End;
                return symmetricDifference;
            }
            if (range2.m_Start == range1.m_Start && range2.m_End < range1.m_End) {
                RangeCollection symmetricDifference(1);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range2.m_End, range1.m_End});
                symmetricDifference.m_MinBound = range1.m_Start;
                symmetricDifference.m_MaxBound = range1.m_End;
                return symmetricDifference;
            }
            if (range1.m_End == range2.m_End && range1.m_Start < range2.m_Start) {
                RangeCollection symmetricDifference(1);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range1.m_Start, range2.m_Start});
                symmetricDifference.m_MinBound = range1.m_Start;
                symmetricDifference.m_MaxBound = range1.m_End;
                return symmetricDifference;
            }
            if (range2.m_End == range1.m_End && range2.m_Start < range1.m_Start) {
                RangeCollection symmetricDifference(1);
                symmetricDifference.m_ResolveBoundsOnGet = false;
                symmetricDifference.m_Ranges.push_back({range2.m_Start, range1.m_Start});
                symmetricDifference.m_MinBound = range2.m_Start;
                symmetricDifference.m_MaxBound = range2.m_End;
                return symmetricDifference;
            }
            return RangeCollection(0);
        }

        friend class Range;
    };
}

#endif //RANGECOLLECTION_H
