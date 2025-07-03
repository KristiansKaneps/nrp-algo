#ifndef SCORE_H
#define SCORE_H

#include <iostream>
#include <cstdint>

namespace Score {
    typedef int64_t score_t;

    static constexpr unsigned int SCORE_BITS = sizeof(score_t) * 8;
    static constexpr unsigned int SCORE_SIGN_BIT_OFFSET = SCORE_BITS - 1;
    static constexpr score_t SCORE_SIGN = static_cast<score_t>(1) << SCORE_SIGN_BIT_OFFSET;

    struct Score {
        score_t strict{};
        score_t hard{};
        score_t soft{};

        [[nodiscard]] bool isFeasible() const noexcept { return strict >= 0 && hard >= 0; }
        [[nodiscard]] bool isZero() const noexcept { return strict >= 0 && hard >= 0 && soft >= 0; }

        Score& operator=(const Score& rhs) noexcept = default;

        Score operator+(const Score& rhs) const noexcept { return {strict + rhs.strict, hard + rhs.hard, soft + rhs.soft}; }
        Score operator-(const Score& rhs) const noexcept { return {strict - rhs.strict, hard - rhs.hard, soft - rhs.soft}; }

        Score& operator+=(const Score& rhs) noexcept {
            strict += rhs.strict;
            hard += rhs.hard;
            soft += rhs.soft;
            return *this;
        }

        Score& operator-=(const Score& rhs) noexcept {
            strict -= rhs.strict;
            hard -= rhs.hard;
            soft -= rhs.soft;
            return *this;
        }

        bool operator==(const Score& rhs) const noexcept { return strict == rhs.strict && hard == rhs.hard && soft == rhs.soft; }

        bool operator>(const Score& rhs) const noexcept {
            return strict > rhs.strict || (strict == rhs.strict && hard > rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft > rhs.soft);
        }

        bool operator<(const Score& rhs) const noexcept {
            return strict < rhs.strict || (strict == rhs.strict && hard < rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft < rhs.soft);
        }

        bool operator>=(const Score& rhs) const noexcept {
            return strict > rhs.strict || (strict == rhs.strict && hard > rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft >= rhs.soft);
        }

        bool operator<=(const Score& rhs) const noexcept {
            return strict < rhs.strict || (strict == rhs.strict && hard < rhs.hard) || (strict == rhs.strict && hard == rhs.hard && soft <= rhs.soft);
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const Score& score) noexcept {
        out << "Score(strict=" << score.strict << "; hard=" << score.hard << "; soft=" << score.soft << ')';
        return out;
    }
}

#endif //SCORE_H
