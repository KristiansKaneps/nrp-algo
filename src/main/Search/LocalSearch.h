#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Search/LocalSearchTask.h"

#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Heuristics/HeuristicProvider.h"
#include "Score/Score.h"
#include "Statistics/ScoreStatistics.h"

namespace Search {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearch {
    public:
        // ReSharper disable CppRedundantQualifier
        explicit LocalSearch(const ::State::State<X, Y, Z, W> *initialState,
                             const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints) :
            mp_InitialState(initialState),
            m_Constraints(constraints),
            m_HeuristicProvider(::Heuristics::HeuristicProvider<X, Y, Z, W>(constraints.size())),
            m_Task(*initialState, m_Constraints, m_ScoreStatistics) { }
        // ReSharper restore CppRedundantQualifier

        LocalSearch(const LocalSearch& other) = default;

        ~LocalSearch() = default;

        [[nodiscard]] Statistics::ScoreStatistics scoreStatistics() const { return m_ScoreStatistics; }

        void reset() {
            m_Done = false;
            resetInternalTerminationCriteriaState();
        }

        void startStatistics() {
            m_ScoreStatistics.startRecording(m_Task.getInitialScore());
        }

        void endStatistics() {
            m_ScoreStatistics.finishRecording();
        }

        /**
         * @return `true` if new best state is found, `false` otherwise
         */
        bool step() {
            // Finalizer
            if (shouldStep()) [[likely]] {
                m_Task.step(m_HeuristicProvider);
                return m_Task.newBestFound();
                // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
            } else {
                m_Done = true;
                // printBestScore();
                return false;
            }
        }

        [[nodiscard]] bool isDone() const { return m_Done; }

        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> getBestState() const { return m_Task.getOutputState(); }
        [[nodiscard]] Score::Score getBestScore() const { return m_Task.getOutputScore(); }

        [[nodiscard]] Score::Score evaluateCurrentBestState() const {
            Score::Score score {};
            // ReSharper disable once CppRedundantQualifier
            for (::Constraints::Constraint<X, Y, Z, W> *constraint : m_Constraints)
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
        [[nodiscard]] bool shouldStep() {
            if (m_Task.getOutputScore().isZero()) [[unlikely]] {
                if (m_IterationCountAtZeroScore >= 2000000) [[unlikely]] return m_Task.getIdleIterationCount() < 3000000;
                m_IterationCountAtZeroScore += 1;
                return true;
            }
            if (m_Task.getOutputScore().isFeasible()) [[unlikely]] {
                if (m_IterationCountAtFeasibleScore >= 4000000) [[unlikely]] return m_Task.getIdleIterationCount() < 3000000;
                m_IterationCountAtFeasibleScore += 1;
                return true;
            }
            // return m_Task.getIterationCount() <= 20000 && (m_Task.getIterationCount() <= 2000 || m_Task.getIdleIterationCount() <= m_Task.getIterationCount() * 0.02);
            // return m_Task.getIterationCount() <= 2000 || m_Task.getIdleIterationCount() <= m_Task.getIterationCount() * 0.02;
            return m_Task.getIdleIterationCount() < 10000000;
            // return !m_Task.getOutputScore().isZero();
        }

        void resetInternalTerminationCriteriaState() {
            m_IterationCountAtZeroScore = 0;
            m_IterationCountAtFeasibleScore = 0;
        }

    private:
        bool m_Done = false;
        // ReSharper disable once CppRedundantQualifier
        const ::State::State<X, Y, Z, W> *mp_InitialState;
        // ReSharper disable once CppRedundantQualifier
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *> m_Constraints;
        // ReSharper disable once CppRedundantQualifier
        ::Heuristics::HeuristicProvider<X, Y, Z, W> m_HeuristicProvider;
        Statistics::ScoreStatistics m_ScoreStatistics{};
        Task::LocalSearchTask<X, Y, Z, W> m_Task;

        uint64_t m_IterationCountAtZeroScore = 0, m_IterationCountAtFeasibleScore = 0;
    };
}

#endif //LOCALSEARCH_H
