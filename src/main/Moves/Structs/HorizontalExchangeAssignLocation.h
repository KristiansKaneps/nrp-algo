#ifndef HORIZONTALEXCHANGEASSIGNLOCATION_H
#define HORIZONTALEXCHANGEASSIGNLOCATION_H

#include <functional>

#include "State/Size.h"

namespace Moves::Structs {
    using axis_size_t = ::State::axis_size_t;

    struct HorizontalExchangeAssignLocation {
        axis_size_t x, z, w;

        bool operator==(const HorizontalExchangeAssignLocation& other) const noexcept {
            return x == other.x && z == other.z && w == other.w;
        }
    };
}

template <>
struct std::hash<Moves::Structs::HorizontalExchangeAssignLocation> { // NOLINT(*-dcl58-cpp)
    std::size_t operator()(const Moves::Structs::HorizontalExchangeAssignLocation& loc) const noexcept {
        const std::size_t h1 = std::hash<::State::axis_size_t>()(loc.x);
        const std::size_t h2 = std::hash<::State::axis_size_t>()(loc.z);
        const std::size_t h3 = std::hash<::State::axis_size_t>()(loc.w);
        return h1 * 73856093 ^ h2 * 19349663 ^ h3 * 83492791;
    }
};

#endif //HORIZONTALEXCHANGEASSIGNLOCATION_H
