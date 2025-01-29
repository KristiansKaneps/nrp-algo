#ifndef SCORE_H
#define SCORE_H

#include <iostream>
#include <cstdint>

namespace Score {
    typedef int64_t score_t;

    struct Score {
        score_t hard;
        score_t soft;

        Score& operator=(const Score& rhs) = default;

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

        bool operator==(const Score& rhs) const { return hard == rhs.hard && soft == rhs.soft; }

        bool operator>(const Score& rhs) const {
            return hard > rhs.hard || (hard == rhs.hard && soft > rhs.soft);
        }

        bool operator<(const Score& rhs) const {
            return hard < rhs.hard || (hard == rhs.hard && soft < rhs.soft);
        }

        bool operator>=(const Score& rhs) const {
            return hard >= rhs.hard || (hard == rhs.hard && soft >= rhs.soft);
        }

        bool operator<=(const Score& rhs) const {
            return hard <= rhs.hard || (hard == rhs.hard && soft <= rhs.soft);
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const Score& score) {
        out << "Score(hard=" << score.hard << "; soft=" << score.soft << ')';
        return out;
    }
}

#endif //SCORE_H
