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

        [[nodiscard]] state_size_t volume() const { return width * height * depth * concepts; }

        [[nodiscard]] constexpr state_size_t offset(const axis_size_t x, const axis_size_t y) const {
            assert(x < width && "X must be less than the width.");
            assert(y < height && "Y must be less than the height.");
            return x * height * depth * concepts + y * depth * concepts;
        }

        [[nodiscard]] constexpr state_size_t offset(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            assert(z < depth && "Z must be less than the depth.");
            return offset(x, y) + z * concepts;
        }

        [[nodiscard]] constexpr state_size_t index(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w) const {
            assert(w < concepts && "W must be less than the total concept count.");
            return offset(x, y, z) + w;
        }

        [[nodiscard]] constexpr state_size_t offsetX(const axis_size_t y, const axis_size_t z, const axis_size_t w) const {
            return y * depth * concepts + z * concepts + w;
        }

        [[nodiscard]] constexpr state_size_t offsetY(const axis_size_t x, const axis_size_t z, const axis_size_t w) const {
            return x * height * depth * concepts + z * concepts + w;
        }

        [[nodiscard]] constexpr state_size_t offsetZ(const axis_size_t x, const axis_size_t y, const axis_size_t w) const {
            return x * height * depth * concepts + y * depth * concepts + w;
        }

        [[nodiscard]] constexpr state_size_t offsetW(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            return x * height * depth * concepts + y * depth * concepts + z * concepts;
        }
    };

    struct Location {
        axis_size_t x;
        axis_size_t y;
        axis_size_t z;
        axis_size_t w;

        [[nodiscard]] constexpr bool operator==(const Location &other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        [[nodiscard]] constexpr state_size_t index(const Size &size) const {
            return size.index(x, y, z, w);
        }
    };
}

#endif //SIZE_H
