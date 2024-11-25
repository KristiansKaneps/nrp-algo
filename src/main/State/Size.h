#ifndef SIZE_H
#define SIZE_H

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
    };
}

#endif //SIZE_H
