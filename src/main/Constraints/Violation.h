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
        Score score;

        Violation(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w, const uint8_t flags, const Score& score) : Area(x, y, z, w, flags), score(score) {}

        static Violation x(const axis_size_t x, const Score& score) { return {x, 0, 0, 0, X, score}; }

        static Violation y(const axis_size_t y, const Score& score) { return {0, y, 0, 0, Y, score}; }

        static Violation z(const axis_size_t z, const Score& score) { return {0, 0, z, 0, Z, score}; }

        static Violation w(const axis_size_t w, const Score& score) { return {0, 0, 0, w, W, score}; }

        static Violation xy(const axis_size_t x, const axis_size_t y, const Score& score) {
            return {x, y, 0, 0, XY, score};
        }

        static Violation xz(const axis_size_t x, const axis_size_t z, const Score& score) {
            return {x, 0, z, 0, XZ, score};
        }

        static Violation xw(const axis_size_t x, const axis_size_t w, const Score& score) {
            return {x, 0, 0, w, XW, score};
        }

        static Violation yz(const axis_size_t y, const axis_size_t z, const Score& score) {
            return {0, y, z, 0, YZ, score};
        }

        static Violation yw(const axis_size_t y, const axis_size_t w, const Score& score) {
            return {0, y, 0, w, YW, score};
        }

        static Violation zw(const axis_size_t z, const axis_size_t w, const Score& score) {
            return {0, 0, z, w, ZW, score};
        }

        static Violation
        xyz(const axis_size_t x, const axis_size_t y, const axis_size_t z, const Score& score) {
            return {x, y, z, 0, XYZ, score};
        }

        static Violation
        xyw(const axis_size_t x, const axis_size_t y, const axis_size_t w, const Score& score) {
            return {x, y, 0, w, XYW, score};
        }

        static Violation
        xzw(const axis_size_t x, const axis_size_t z, const axis_size_t w, const Score& score) {
            return {x, 0, z, w, XZW, score};
        }

        static Violation
        yzw(const axis_size_t y, const axis_size_t z, const axis_size_t w, const Score& score) {
            return {0, y, z, w, YZW, score};
        }

        static Violation xyzw(const axis_size_t x, const axis_size_t y, const axis_size_t z,
                                      const axis_size_t w, const Score& score) { return {x, y, z, w, XYZW, score}; }

        static Violation xyzw(const Location& location, const Score& score) {
            return {location.x, location.y, location.z, location.w, XYZW, score};
        }
    };
}

#endif //VIOLATION_H
