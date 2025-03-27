#ifndef CONSTRAINTSCORE_H
#define CONSTRAINTSCORE_H

#include "Score/Score.h"
#include "State/State.h"

namespace Constraints {
    using Score = ::Score::Score;
    using axis_size_t = ::State::axis_size_t;
    using state_size_t = ::State::state_size_t;
    using score_t = ::Score::score_t;

    class ConstraintScore {
    public:
        explicit ConstraintScore(const Score& score) : m_Score(score) {

        }

        [[nodiscard]] const Score &score() const {
            return m_Score;
        }

    private:
        Score m_Score;
    };

    inline Score operator+(const Score& lhs, const ConstraintScore& rhs) { return lhs + rhs.score(); }
    inline Score operator-(const Score& lhs, const ConstraintScore& rhs) { return lhs + rhs.score(); }

    inline Score& operator+=(Score& lhs, const ConstraintScore& rhs) {
        return lhs += rhs.score();
    }

    inline Score& operator-=(Score& lhs, const ConstraintScore& rhs) {
        return lhs -= rhs.score();
    }

    inline bool operator==(const Score& lhs, const ConstraintScore& rhs) { return lhs == rhs.score(); }

    inline bool operator>(const Score& lhs, const ConstraintScore& rhs) {
        return lhs > rhs.score();
    }

    inline bool operator<(const Score& lhs, const ConstraintScore& rhs) {
        return lhs < rhs.score();
    }

    inline bool operator>=(const Score& lhs, const ConstraintScore& rhs) {
        return lhs >= rhs.score();
    }

    inline bool operator<=(const Score& lhs, const ConstraintScore& rhs) {
        return lhs <= rhs.score();
    }
}

#endif //CONSTRAINTSCORE_H
