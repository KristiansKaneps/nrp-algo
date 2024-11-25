#ifndef AXES_H
#define AXES_H

#include <type_traits>
#include <cassert>

#include "State/Size.h"

namespace Axes {
    using axis_size_t = State::axis_size_t;

    class AxisEntity {
    public:
        AxisEntity() = default;
        virtual ~AxisEntity() = default;
    };

    template<typename T>
    class Axis {
        static_assert(std::is_base_of_v<AxisEntity, T>, "T should inherit from AxisEntity");

    public:
        explicit Axis(const T *entities, const axis_size_t size) : m_Entities(entities),
                                                                   m_Size(size) { }

        ~Axis() = default;

        [[nodiscard]] const T *entities() const { return m_Entities; }
        [[nodiscard]] axis_size_t size() const { return m_Size; }

        const T& operator[](const axis_size_t index) const {
            assert(index < m_Size && "Index should not exceed axis size");
            return *(m_Entities + index);
        }

    private:
        const T *m_Entities;
        const axis_size_t m_Size {};
    };
}

#endif //AXES_H
