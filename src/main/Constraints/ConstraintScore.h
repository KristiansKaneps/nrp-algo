#ifndef CONSTRAINTSCORE_H
#define CONSTRAINTSCORE_H

#include <vector>

#include "Violation.h"
#include "Score/Score.h"

namespace Constraints {
    template<typename X, typename Y, typename Z, typename W>
    class Constraint;

    using Score = ::Score::Score;
    using axis_size_t = ::State::axis_size_t;
    using state_size_t = ::State::state_size_t;

    class ConstraintScore {
    public:
        explicit ConstraintScore(const Score& score, std::vector<Violation>&& violations) noexcept : m_Score(score),
            m_Violations(std::move(violations)) { }

        ConstraintScore() noexcept {
            m_Violations.shrink_to_fit();
        }

        [[nodiscard]] const Score& score() const noexcept { return m_Score; }

        [[nodiscard]] const std::vector<Violation>& violations() const noexcept { return m_Violations; }

        void addScore(const Score& score) noexcept { m_Score += score; }
        void addStrictScore(const score_t strict) noexcept { m_Score.strict += strict; }
        void addHardScore(const score_t hard) noexcept { m_Score.hard += hard; }
        void addSoftScore(const score_t soft) noexcept { m_Score.soft += soft; }

        Score operator+(const Score& rhs) const noexcept { return m_Score + rhs; }
        Score operator-(const Score& rhs) const noexcept { return m_Score + rhs; }

        ConstraintScore& operator+=(const Score& rhs) noexcept {
            m_Score += rhs;
            return *this;
        }

        ConstraintScore& operator-=(const Score& rhs) noexcept {
            m_Score -= rhs;
            return *this;
        }

        bool operator==(const Score& rhs) const noexcept { return m_Score == rhs; }

        bool operator>(const Score& rhs) const noexcept { return m_Score > rhs; }

        bool operator<(const Score& rhs) const noexcept { return m_Score < rhs; }

        bool operator>=(const Score& rhs) const noexcept { return m_Score >= rhs; }

        bool operator<=(const Score& rhs) const noexcept { return m_Score <= rhs; }

        void violate(Violation&& violation) noexcept {
            m_Score += violation.score;
            m_Violations.emplace_back(std::forward<Violation>(violation));
        }

    protected:
        Score m_Score{};
        std::vector<Violation> m_Violations;

        template<typename X, typename Y, typename Z, typename W>
        friend class ::Constraints::Constraint;
    };

    inline Score operator+(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs + rhs.score(); }
    inline Score operator-(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs + rhs.score(); }

    inline Score& operator+=(Score& lhs, const ConstraintScore& rhs) noexcept { return lhs += rhs.score(); }

    inline Score& operator-=(Score& lhs, const ConstraintScore& rhs) noexcept { return lhs -= rhs.score(); }

    inline bool operator==(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs == rhs.score(); }

    inline bool operator>(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs > rhs.score(); }

    inline bool operator<(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs < rhs.score(); }

    inline bool operator>=(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs >= rhs.score(); }

    inline bool operator<=(const Score& lhs, const ConstraintScore& rhs) noexcept { return lhs <= rhs.score(); }
}

#endif //CONSTRAINTSCORE_H
