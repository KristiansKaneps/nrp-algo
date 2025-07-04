#ifndef LOCALSEARCHTASK_H
#define LOCALSEARCHTASK_H

#include "Evaluation.h"
#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Score/Score.h"
#include "Heuristics/HeuristicProvider.h"
#include "Statistics/ScoreStatistics.h"

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearchTask {
    public:
        LocalSearchTask(const LocalSearchTask&) = delete;

        // ReSharper disable CppRedundantQualifier
        explicit LocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                 const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints,
                                 Statistics::ScoreStatistics &scoreStatistics) noexcept :
            m_OutputState(inputState),
            m_Evaluator(constraints),
            m_ScoreStatistics(scoreStatistics),
            m_InitScore(Evaluation::evaluateState(inputState, constraints)),
            m_CurrentState(inputState) {
            m_CurrentScore = m_InitScore;
            m_OutputScore = m_InitScore;
        }
        // ReSharper restore CppRedundantQualifier

        virtual ~LocalSearchTask() noexcept = default;

        [[nodiscard]] bool newBestFound() const noexcept { return m_NewBestFound; }

        // ReSharper disable once CppRedundantQualifier
        virtual void reset(const ::State::State<X, Y, Z, W> inputState) noexcept { m_OutputState = inputState; }

        // ReSharper disable CppRedundantQualifier
        virtual void step(::Heuristics::HeuristicProvider<X, Y, Z, W> &heuristicProvider) noexcept = 0;
        // ReSharper restore CppRedundantQualifier

        /**
         * Defines termination criteria.
         * @return `true` if should keep searching for local optima, `false` otherwise
         */
        [[nodiscard]] virtual bool shouldStep() noexcept = 0;

        [[nodiscard]] Score::Score getInitialScore() const noexcept { return m_InitScore; }

        // ReSharper disable once CppRedundantQualifier
        [[nodiscard]] ::State::State<X, Y, Z, W> getOutputState() const noexcept { return m_OutputState; }
        [[nodiscard]] Score::Score getOutputScore() const noexcept { return m_OutputScore; }

    protected:
        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> m_OutputState;
        Score::Score m_OutputScore;
        Evaluation::Evaluator<X, Y, Z, W> m_Evaluator;
        Statistics::ScoreStatistics& m_ScoreStatistics;

        const Score::Score m_InitScore;
        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> m_CurrentState;
        Score::Score m_CurrentScore;

        bool m_NewBestFound = false;
    };
}

#endif //LOCALSEARCHTASK_H
