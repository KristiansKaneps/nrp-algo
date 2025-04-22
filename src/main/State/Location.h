#ifndef LOCATION_H
#define LOCATION_H

#include "Size.h"

namespace State {
    using axis_size_t = ::State::axis_size_t;

    struct Location {
        axis_size_t x, y, z, w;

        [[nodiscard]] axis_size_t getX() const { return x; }
        [[nodiscard]] axis_size_t getY() const { return y; }
        [[nodiscard]] axis_size_t getZ() const { return z; }
        [[nodiscard]] axis_size_t getW() const { return w; }

        [[nodiscard]] constexpr bool operator==(const Location& other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        [[nodiscard]] constexpr state_size_t index(const Size& size) const { return size.index(x, y, z, w); }
    };

    struct Area : Location {
        constexpr static uint8_t X = 0b00000001;
        constexpr static uint8_t Y = 0b00000010;
        constexpr static uint8_t Z = 0b00000100;
        constexpr static uint8_t W = 0b00001000;
        constexpr static uint8_t XY = X | Y;
        constexpr static uint8_t XZ = X | Z;
        constexpr static uint8_t XW = X | W;
        constexpr static uint8_t YZ = Y | Z;
        constexpr static uint8_t YW = Y | W;
        constexpr static uint8_t ZW = Z | W;
        constexpr static uint8_t XYZ = X | Y | Z;
        constexpr static uint8_t XYW = X | Y | W;
        constexpr static uint8_t XZW = X | Z | W;
        constexpr static uint8_t YZW = Y | Z | W;
        constexpr static uint8_t XYZW = X | Y | Z | W;

        uint8_t flags;

        Area(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w, const uint8_t flags) : Location{x, y, z, w}, flags(flags) {}

        [[nodiscard]] bool hasX() const { return (flags & X) != 0; }
        [[nodiscard]] bool hasY() const { return (flags & Y) != 0; }
        [[nodiscard]] bool hasZ() const { return (flags & Z) != 0; }
        [[nodiscard]] bool hasW() const { return (flags & W) != 0; }

        static Area x(const axis_size_t x) { return {x, 0, 0, 0, X}; }

        static Area y(const axis_size_t y) { return {0, y, 0, 0, Y}; }

        static Area z(const axis_size_t z) { return {0, 0, z, 0, Z}; }

        static Area w(const axis_size_t w) { return {0, 0, 0, w, W}; }

        static Area xy(const axis_size_t x, const axis_size_t y) { return {x, y, 0, 0, XY}; }

        static Area xz(const axis_size_t x, const axis_size_t z) { return {x, 0, z, 0, XZ}; }

        static Area xw(const axis_size_t x, const axis_size_t w) { return {x, 0, 0, w, XW}; }

        static Area yz(const axis_size_t y, const axis_size_t z) { return {0, y, z, 0, YZ}; }

        static Area yw(const axis_size_t y, const axis_size_t w) { return {0, y, 0, w, YW}; }

        static Area zw(const axis_size_t z, const axis_size_t w) { return {0, 0, z, w, ZW}; }

        static Area xyz(const axis_size_t x, const axis_size_t y, const axis_size_t z) { return {x, y, z, 0, XYZ}; }

        static Area xyw(const axis_size_t x, const axis_size_t y, const axis_size_t w) { return {x, y, 0, w, XYW}; }

        static Area xzw(const axis_size_t x, const axis_size_t z, const axis_size_t w) { return {x, 0, z, w, XZW}; }

        static Area yzw(const axis_size_t y, const axis_size_t z, const axis_size_t w) { return {0, y, z, w, YZW}; }

        static Area xyzw(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w) {
            return {x, y, z, w, XYZW};
        }

        static Area xyzw(const Location& location) { return {location.x, location.y, location.z, location.w, XYZW}; }
    };
}

#endif //LOCATION_H
