#ifndef VIOLATION_H
#define VIOLATION_H

#include "Score/Score.h"
#include "State/Location.h"

namespace Constraints {
    using Area = ::State::Area;
    using axis_size_t = ::State::axis_size_t;
    using Score = ::Score::Score;
    using score_t = ::Score::score_t;

    struct Violation : Area {
        typedef uint8_t info_t;

        Score score;
        info_t info {};

        Violation(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w, const uint8_t flags, const Score& score, const info_t info = 0) : Area(x, y, z, w, flags), score(score), info(info) {}

        static Violation x(const axis_size_t x, const Score& score, const info_t info = 0) { return {x, 0, 0, 0, X, score, info}; }

        static Violation y(const axis_size_t y, const Score& score, const info_t info = 0) { return {0, y, 0, 0, Y, score, info}; }

        static Violation z(const axis_size_t z, const Score& score, const info_t info = 0) { return {0, 0, z, 0, Z, score, info}; }

        static Violation w(const axis_size_t w, const Score& score, const info_t info = 0) { return {0, 0, 0, w, W, score, info}; }

        static Violation xy(const axis_size_t x, const axis_size_t y, const Score& score, const info_t info = 0) {
            return {x, y, 0, 0, XY, score, info};
        }

        static Violation xz(const axis_size_t x, const axis_size_t z, const Score& score, const info_t info = 0) {
            return {x, 0, z, 0, XZ, score, info};
        }

        static Violation xw(const axis_size_t x, const axis_size_t w, const Score& score, const info_t info = 0) {
            return {x, 0, 0, w, XW, score, info};
        }

        static Violation yz(const axis_size_t y, const axis_size_t z, const Score& score, const info_t info = 0) {
            return {0, y, z, 0, YZ, score, info};
        }

        static Violation yw(const axis_size_t y, const axis_size_t w, const Score& score, const info_t info = 0) {
            return {0, y, 0, w, YW, score, info};
        }

        static Violation zw(const axis_size_t z, const axis_size_t w, const Score& score, const info_t info = 0) {
            return {0, 0, z, w, ZW, score, info};
        }

        static Violation
        xyz(const axis_size_t x, const axis_size_t y, const axis_size_t z, const Score& score, const info_t info = 0) {
            return {x, y, z, 0, XYZ, score, info};
        }

        static Violation
        xyw(const axis_size_t x, const axis_size_t y, const axis_size_t w, const Score& score, const info_t info = 0) {
            return {x, y, 0, w, XYW, score, info};
        }

        static Violation
        xzw(const axis_size_t x, const axis_size_t z, const axis_size_t w, const Score& score, const info_t info = 0) {
            return {x, 0, z, w, XZW, score, info};
        }

        static Violation
        yzw(const axis_size_t y, const axis_size_t z, const axis_size_t w, const Score& score, const info_t info = 0) {
            return {0, y, z, w, YZW, score, info};
        }

        static Violation xyzw(const axis_size_t x, const axis_size_t y, const axis_size_t z,
                                      const axis_size_t w, const Score& score, const info_t info = 0) {
            return {x, y, z, w, XYZW, score, info};
        }

        static Violation xyzw(const Location& location, const Score& score, const info_t info = 0) {
            return {location.x, location.y, location.z, location.w, XYZW, score, info};
        }
    };
}

#endif //VIOLATION_H
