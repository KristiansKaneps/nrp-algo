#ifndef SCORE_H
#define SCORE_H

#include <cstdint>

namespace Score {
    typedef int64_t score_t;

    struct Score {
        score_t hard;
        score_t soft;

        Score operator+(const Score& rhs) const { return {hard + rhs.hard, soft + rhs.soft}; }
        Score operator-(const Score& rhs) const { return {hard - rhs.hard, soft - rhs.soft}; }

        Score& operator+=(const Score& rhs) {
            hard += rhs.hard;
            soft += rhs.soft;
            return *this;
        }

        Score& operator-=(const Score& rhs) {
            hard -= rhs.hard;
            soft -= rhs.soft;
            return *this;
        }
    };
}

#endif //SCORE_H
