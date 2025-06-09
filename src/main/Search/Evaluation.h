#ifndef EVALUATION_H
#define EVALUATION_H

#ifndef NDEBUG
#define PRINT_CONSTRAINT_DEBUG_INFO
#endif

#include "Constraints/Constraint.h"
#include "Score/Score.h"
#include "State/State.h"

#include <cassert>

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class HeuristicProvider;
}

namespace Evaluation {
    template<typename X, typename Y, typename Z, typename W>
    // ReSharper disable once CppRedundantQualifier
    Score::Score evaluateState(const ::State::State<X, Y, Z, W>& state,
                               const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints) {
        Score::Score score {};
        // ReSharper disable once CppRedundantQualifier
        for (::Constraints::Constraint<X, Y, Z, W> *constraint : constraints)
            score += constraint->evaluate(state);
        return score;
    }

    template<typename X, typename Y, typename Z, typename W>
    class Evaluator {
    public:
        explicit Evaluator(const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints) :
            m_Constraints(constraints),
            m_ConstraintScores(constraints.size(), ::Constraints::ConstraintScore{}) {
            #ifdef PRINT_CONSTRAINT_DEBUG_INFO
            m_ConstraintNameLength.reserve(constraints.size());
            for (auto *constraint : constraints) {
                m_AnyConstraintPrintsInfo = m_AnyConstraintPrintsInfo || constraint->printsInfo();
                const auto length = constraint->name().size();
                m_ConstraintNameLength.push_back(length);
                if (m_MaxConstraintNameLength < length)
                    m_MaxConstraintNameLength = length;
            }
            #endif
        }

        ~Evaluator() = default;
        Evaluator(const Evaluator&) = default;

        const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints() const { return m_Constraints; }
        [[nodiscard]] size_t constraintCount() const { return m_Constraints.size(); }

        void printConstraintInfo() const {
            #ifdef PRINT_CONSTRAINT_DEBUG_INFO
            if (!m_AnyConstraintPrintsInfo) [[likely]] return;
            size_t i = 0;
            static const std::string header = "Constraint info";
            int32_t diff = 10 + m_MaxConstraintNameLength - header.size() - 2;
            int32_t paddingLengthStart = diff >> 1;
            int32_t paddingLengthEnd = (diff & 1) != 0 ? paddingLengthStart + 1 : paddingLengthStart;
            for (size_t j = 0; j < paddingLengthStart; ++j) std::cout << '=';
            std::cout << ' ' << header << ' ';
            for (size_t j = 0; j < paddingLengthEnd; ++j) std::cout << '=';
            std::cout << std::endl;
            for (auto *constraint : m_Constraints) {
                if (!constraint->printsInfo()) {
                    i += 1;
                    continue;
                }
                diff = 10 + m_MaxConstraintNameLength - m_ConstraintNameLength[i] - 2;
                paddingLengthStart = diff >> 1;
                paddingLengthEnd = (diff & 1) != 0 ? paddingLengthStart + 1 : paddingLengthStart;
                for (size_t j = 0; j < paddingLengthStart; ++j) std::cout << '=';
                std::cout << ' ' << constraint->name() << ' ';
                for (size_t j = 0; j < paddingLengthEnd; ++j) std::cout << '=';
                std::cout << std::endl;
                constraint->printInfo();
                for (size_t j = 0; j < 10 + m_MaxConstraintNameLength; ++j) std::cout << '=';
                std::cout << std::endl;
                i += 1;
            }
            #endif
        }

        Score::Score evaluateState(const ::State::State<X, Y, Z, W>& state) {
            Score::Score score {};
            m_ConstraintScores.clear();
            m_TotalConstraintViolationCount = 0;
            m_ViolatedConstraintCount = 0;
            for (auto it = m_Constraints.begin(); it != m_Constraints.end(); ++it) {
                const auto& constraint = *it;
                const auto constraintScore = constraint->evaluate(state);
                score += constraintScore;
                m_TotalConstraintViolationCount += constraintScore.violations().size();
                m_ViolatedConstraintCount += constraintScore.violations().size() > 0;
                m_ConstraintScores.emplace_back(constraintScore);
            }
            assert(m_ConstraintScores.size() == m_Constraints.size() && "Constraint scores size mismatch");
            return score;
        }

    protected:
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& m_Constraints;
        std::vector<::Constraints::ConstraintScore> m_ConstraintScores;
        size_t m_TotalConstraintViolationCount{}, m_ViolatedConstraintCount{};

        friend class ::Heuristics::HeuristicProvider<X, Y, Z, W>;

    private:
        #ifdef PRINT_CONSTRAINT_DEBUG_INFO
        bool m_AnyConstraintPrintsInfo = false;
        int32_t m_MaxConstraintNameLength = 0;
        std::vector<int32_t> m_ConstraintNameLength;
        #endif
    };
}

#endif //EVALUATION_H
