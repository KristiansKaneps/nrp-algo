#ifndef SIZE_H
#define SIZE_H

#include <cassert>
#include <cstdint>

namespace State {
    typedef uint32_t axis_size_t;
    typedef uint64_t state_size_t;

    struct Size {
        axis_size_t width;
        axis_size_t height;
        axis_size_t depth;
        axis_size_t concepts;

        [[nodiscard]] bool isValid() const noexcept { return width > 0 && height > 0 && depth > 0 && concepts > 0; }

        [[nodiscard]] state_size_t volume() const noexcept { return width * height * depth * concepts; }

        [[nodiscard]] constexpr state_size_t offset(const axis_size_t x, const axis_size_t y) const noexcept {
            assert(x < width && "X must be less than the width.");
            assert(y < height && "Y must be less than the height.");
            return x * height * depth * concepts + y * depth * concepts;
        }

        [[nodiscard]] constexpr state_size_t offset(const axis_size_t x, const axis_size_t y, const axis_size_t z) const noexcept {
            assert(z < depth && "Z must be less than the depth.");
            return offset(x, y) + z * concepts;
        }

        [[nodiscard]] constexpr state_size_t index(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w) const noexcept {
            assert(w < concepts && "W must be less than the total concept count.");
            return offset(x, y, z) + w;
        }

        [[nodiscard]] constexpr state_size_t offsetX(const axis_size_t y, const axis_size_t z, const axis_size_t w) const noexcept {
            return y * depth * concepts + z * concepts + w;
        }

        [[nodiscard]] constexpr state_size_t offsetY(const axis_size_t x, const axis_size_t z, const axis_size_t w) const noexcept {
            return x * height * depth * concepts + z * concepts + w;
        }

        [[nodiscard]] constexpr state_size_t offsetZ(const axis_size_t x, const axis_size_t y, const axis_size_t w) const noexcept {
            return x * height * depth * concepts + y * depth * concepts + w;
        }

        [[nodiscard]] constexpr state_size_t offsetW(const axis_size_t x, const axis_size_t y, const axis_size_t z) const noexcept {
            return x * height * depth * concepts + y * depth * concepts + z * concepts;
        }
    };
}

#endif //SIZE_H
