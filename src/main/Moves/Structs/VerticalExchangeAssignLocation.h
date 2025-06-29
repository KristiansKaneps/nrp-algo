#ifndef VERTICALEXCHANGEASSIGNLOCATION_H
#define VERTICALEXCHANGEASSIGNLOCATION_H

#include <functional>

#include "State/Size.h"

namespace Moves::Structs {
    using axis_size_t = ::State::axis_size_t;

    struct VerticalExchangeAssignLocation {
        axis_size_t x, y, w;

        bool operator==(const VerticalExchangeAssignLocation& other) const {
            return x == other.x && y == other.y && w == other.w;
        }
    };
}

template <>
struct std::hash<Moves::Structs::VerticalExchangeAssignLocation> { // NOLINT(*-dcl58-cpp)
    std::size_t operator()(const Moves::Structs::VerticalExchangeAssignLocation& loc) const noexcept {
        const std::size_t h1 = std::hash<::State::axis_size_t>()(loc.x);
        const std::size_t h2 = std::hash<::State::axis_size_t>()(loc.y);
        const std::size_t h3 = std::hash<::State::axis_size_t>()(loc.w);
        return h1 * 73856093 ^ h2 * 19349663 ^ h3 * 83492791;
    }
};

#endif //VERTICALEXCHANGEASSIGNLOCATION_H
