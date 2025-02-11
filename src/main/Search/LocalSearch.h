#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Search/LocalSearchTask.h"

#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Score/Score.h"

namespace Search {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearch {
    public:
        explicit LocalSearch(const State::State<X, Y, Z, W> *initialState,
                             const std::vector<Constraints::Constraint<X, Y, Z, W> *> &constraints) :
            mp_InitialState(initialState),
            m_Constraints(constraints),
            m_Task(*initialState, m_Constraints) { }

        LocalSearch(const LocalSearch& other) = default;

        ~LocalSearch() = default;

        /**
         * @return `true` if new best state is found, `false` otherwise
         */
        bool step() {
            if (shouldStep()) [[likely]] {
                m_Task.step();
                return m_Task.m_NewBestFound;
                // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
            } else {
                m_Done = true;
                // printBestScore();
                return false;
            }
        }

        [[nodiscard]] bool isDone() const { return m_Done; }

        State::State<X, Y, Z, W> getBestState() const { return m_Task.getOutputState(); }
        [[nodiscard]] Score::Score getBestScore() const { return m_Task.getOutputScore(); }

        [[nodiscard]] Score::Score evaluateCurrentBestState() const {
            Score::Score score {};
            for (Constraints::Constraint<X, Y, Z, W> *constraint : m_Constraints)
                score += constraint->evaluate(m_Task.getOutputState());
            return score;
        }

        void printBestScore() const {
            std::cout << "Best score: " << m_Task.getOutputScore() << "; iterations=" << m_Task.m_Iterations << " (idle=" << m_Task.m_IdleIterations <<
                "); delta=" << (m_Task.getOutputScore() - m_Task.m_InitScore) << std::endl;
        }

    protected:
        /**
         * Defines termination criteria.
         * @return `true` if should keep searching for local optima, `false` otherwise
         */
        [[nodiscard]] bool shouldStep() const {
            // return m_Task.m_Iterations <= 20000 && (m_Task.m_Iterations <= 2000 || m_Task.m_IdleIterations <= m_Task.m_Iterations * 0.02);
            // return m_Task.m_Iterations <= 2000 || m_Task.m_IdleIterations <= m_Task.m_Iterations * 0.02;
            return !m_Task.getOutputScore().isZero();
        }

    private:
        bool m_Done = false;
        const State::State<X, Y, Z, W> *mp_InitialState;
        const std::vector<Constraints::Constraint<X, Y, Z, W> *> m_Constraints;
        Task::LocalSearchTask<X, Y, Z, W> m_Task;
    };
}

#endif //LOCALSEARCH_H
