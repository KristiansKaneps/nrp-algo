#ifndef DAY_H
#define DAY_H

#include "Time/Range.h"
#include "State/Axes.h"

namespace Domain {
    class Day : public Axes::AxisEntity {
    public:
        explicit Day(const uint32_t index, const Time::Range& range) : m_Index(index),
                                                                       m_Range(range) { }

        [[nodiscard]] uint32_t index() const { return m_Index; }
        [[nodiscard]] const Time::Range& range() const { return m_Range; }

    protected:
        const uint32_t m_Index;
        const Time::Range m_Range;
    };
}

#endif //DAY_H
