#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Search/LocalSearchStep.h"

#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Score/Score.h"

namespace Search {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearch {
    public:
        explicit LocalSearch(const State::State<X, Y, Z, W> *initialState,
                             const std::vector<Constraints::Constraint<X, Y, Z, W> *>& constraints) :
            m_InitialState(initialState),
            m_CurrentState(*initialState),
            m_Constraints(constraints) { }

        LocalSearch(const LocalSearch& other) = default;

        ~LocalSearch() = default;

        [[nodiscard]] size_t stepSize() const { return m_Steps.size(); }

        const std::vector<Step::LocalSearchStep<X, Y, Z, W>>& getSteps() const { return m_Steps; }

        void addStep(Step::LocalSearchStep<X, Y, Z, W> *step) { m_Steps.push_back(step); }

        [[nodiscard]] Score::Score evaluateCurrentState() const {
            Score::Score score {};
            for (Constraints::Constraint<X, Y, Z, W> *constraint : m_Constraints)
                score += constraint->evaluate(m_CurrentState);
            return score;
        }

    private:
        const State::State<X, Y, Z, W> *m_InitialState;
        State::State<X, Y, Z, W> m_CurrentState;
        const std::vector<Constraints::Constraint<X, Y, Z, W> *> m_Constraints;
        std::vector<Step::LocalSearchStep<X, Y, Z, W> *> m_Steps;
    };
}

#endif //LOCALSEARCH_H
