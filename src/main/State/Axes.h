#ifndef AXES_H
#define AXES_H

#include <type_traits>
#include <cassert>

#include "State/Size.h"

namespace Axes {
    using axis_size_t = State::axis_size_t;

    class AxisEntity {
    public:
        AxisEntity() noexcept = default;
        virtual ~AxisEntity() noexcept = default;
    };

    template<typename T>
    class Axis {
        static_assert(std::is_base_of_v<AxisEntity, T>, "T should inherit from AxisEntity");

    public:
        explicit Axis(const T *entities, const axis_size_t size) noexcept : m_Entities(entities),
                                                                   m_Size(size) { }

        ~Axis() noexcept = default;

        [[nodiscard]] const T *entities() const noexcept { return m_Entities; }
        [[nodiscard]] axis_size_t size() const noexcept { return m_Size; }

        const T& operator[](const axis_size_t index) const noexcept {
            assert(index < m_Size && "Index should not exceed axis size");
            return *(m_Entities + index);
        }

    private:
        const T *m_Entities;
        const axis_size_t m_Size {};
    };
}

#endif //AXES_H
