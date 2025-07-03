#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Heuristics/HeuristicProvider.h"
#include "Score/Score.h"
#include "Statistics/ScoreStatistics.h"

#include "Search/LocalSearchTask.h"

#include "Search/Implementation/LateAcceptanceLocalSearchTask.h"

namespace Search {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearch {
    public:
        // ReSharper disable CppRedundantQualifier
        explicit LocalSearch(const ::State::State<X, Y, Z, W> *initialState,
                             const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints) noexcept :
            mp_InitialState(initialState),
            m_Constraints(constraints),
            m_HeuristicProvider(::Heuristics::HeuristicProvider<X, Y, Z, W>(initialState, constraints)) {
            mp_Task = new Task::LateAcceptanceLocalSearchTask(*initialState, m_Constraints, m_ScoreStatistics);
        }
        // ReSharper restore CppRedundantQualifier

        LocalSearch(const LocalSearch& other) noexcept = default;

        ~LocalSearch() noexcept = default;

        [[nodiscard]] Statistics::ScoreStatistics scoreStatistics() const noexcept { return m_ScoreStatistics; }

        void reset() noexcept {
            m_Done = false;
        }

        void startStatistics() noexcept {
            m_ScoreStatistics.startRecording(mp_Task->getInitialScore());
        }

        void endStatistics() noexcept {
            m_ScoreStatistics.finishRecording();
        }

        /**
         * @return `true` if new best state is found, `false` otherwise
         */
        bool step() noexcept {
            // Finalizer
            if (mp_Task->shouldStep()) [[likely]] {
                mp_Task->step(m_HeuristicProvider);
                return mp_Task->newBestFound();
                // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
            } else {
                m_Done = true;
                // printBestScore();
                return false;
            }
        }

        [[nodiscard]] bool isDone() const noexcept { return m_Done; }

        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> getBestState() const noexcept { return mp_Task->getOutputState(); }
        [[nodiscard]] Score::Score getBestScore() const noexcept { return mp_Task->getOutputScore(); }

        [[nodiscard]] Score::Score evaluateCurrentBestState() const noexcept {
            Score::Score score {};
            // ReSharper disable once CppRedundantQualifier
            for (::Constraints::Constraint<X, Y, Z, W> *constraint : m_Constraints)
                score += constraint->evaluate(mp_Task->getOutputState());
            return score;
        }

        void printBestScore() const noexcept {
            std::cout << "Best score: " << mp_Task->getOutputScore() << "; delta=" << (mp_Task->getOutputScore() - mp_Task->m_InitScore) << std::endl;
        }

    protected:

    private:
        bool m_Done = false;
        // ReSharper disable once CppRedundantQualifier
        const ::State::State<X, Y, Z, W> *mp_InitialState;
        // ReSharper disable once CppRedundantQualifier
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *> m_Constraints;
        // ReSharper disable once CppRedundantQualifier
        ::Heuristics::HeuristicProvider<X, Y, Z, W> m_HeuristicProvider;
        Statistics::ScoreStatistics m_ScoreStatistics{};
        Task::LocalSearchTask<X, Y, Z, W>* mp_Task;
    };
}

#endif //LOCALSEARCH_H
