#ifndef SCORE_H
#define SCORE_H

#include <iostream>
#include <cstdint>

namespace Score {
    typedef int64_t score_t;

    struct Score {
        score_t strict{};
        score_t hard{};
        score_t soft{};

        [[nodiscard]] bool isFeasible() const { return strict >= 0 && hard >= 0; }
        [[nodiscard]] bool isZero() const { return strict >= 0 && hard >= 0 && soft >= 0; }

        Score& operator=(const Score& rhs) = default;

        Score operator+(const Score& rhs) const { return {strict + rhs.strict, hard + rhs.hard, soft + rhs.soft}; }
        Score operator-(const Score& rhs) const { return {strict - rhs.strict, hard - rhs.hard, soft - rhs.soft}; }

        Score& operator+=(const Score& rhs) {
            strict += rhs.strict;
            hard += rhs.hard;
            soft += rhs.soft;
            return *this;
        }

        Score& operator-=(const Score& rhs) {
            strict -= rhs.strict;
            hard -= rhs.hard;
            soft -= rhs.soft;
            return *this;
        }

        bool operator==(const Score& rhs) const { return strict == rhs.strict && hard == rhs.hard && soft == rhs.soft; }

        bool operator>(const Score& rhs) const {
            return strict > rhs.strict || (strict == rhs.strict && hard > rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft > rhs.soft);
        }

        bool operator<(const Score& rhs) const {
            return strict < rhs.strict || (strict == rhs.strict && hard < rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft < rhs.soft);
        }

        bool operator>=(const Score& rhs) const {
            return strict > rhs.strict || (strict == rhs.strict && hard > rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft >= rhs.soft);
        }

        bool operator<=(const Score& rhs) const {
            return strict < rhs.strict || (strict == rhs.strict && hard < rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft <= rhs.soft);
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const Score& score) {
        out << "Score(strict=" << score.strict << "; hard=" << score.hard << "; soft=" << score.soft << ')';
        return out;
    }
}

#endif //SCORE_H
