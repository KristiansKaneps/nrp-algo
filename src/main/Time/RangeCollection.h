#ifndef RANGECOLLECTION_H
#define RANGECOLLECTION_H

#include "Time/Range.h"

#include <algorithm>
#include <vector>

namespace Time {
    class RangeCollection {
    public:
        RangeCollection() = default;

        explicit RangeCollection(const size_t capacity) : RangeCollection() { m_Ranges.reserve(capacity); }

        RangeCollection(const RangeCollection& other) = default;

        virtual ~RangeCollection() = default;

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
            m_Ranges.insert(std::ranges::upper_bound(std::as_const(m_Ranges), range), range);
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

        template<typename Duration = std::chrono::minutes>
        Duration duration() const {
            Duration duration = Duration::zero();
            for (const auto& range : m_Ranges) duration += range.duration<Duration>();
            return duration;
        }

        [[nodiscard]] bool intersects(const Ray& ray) const {
            if (ray.type() == RAY) [[unlikely]] {
                return std::ranges::any_of(m_Ranges, [ray](const Range& range) -> bool {
                    return range.m_End > ray.m_Start;
                });
            }
            const auto r = static_cast<const Range&>(ray); // NOLINT(*-pro-type-static-cast-downcast)
            return std::ranges::any_of(m_Ranges.cbegin(), m_Ranges.cend(), [r](const Range& range) -> bool {
                return r.intersects(range);
            });
        }

    protected:
        std::vector<Range> m_Ranges;
        bool m_ResolveBoundsOnGet = false;
        Instant m_MinBound = MAX_INSTANT, m_MaxBound = MIN_INSTANT;
    };
}

#endif //RANGECOLLECTION_H
