#ifndef LOCALSEARCHTASK_H
#define LOCALSEARCHTASK_H

#include "Evaluation.h"
#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Score/Score.h"

#include <array>

#include "Heuristics/HeuristicProvider.h"

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearchTask {
    public:
        LocalSearchTask(const LocalSearchTask&) = delete;

        // ReSharper disable CppRedundantQualifier
        explicit LocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                 const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints) :
            m_InitScore(Evaluation::evaluateState(inputState, constraints)),
            m_CurrentState(inputState),
            m_CurrentScore(m_InitScore),
            m_OutputState(inputState),
            m_OutputScore(m_InitScore),
            m_Constraints(constraints) {
            m_History.fill(m_InitScore);
        }
        // ReSharper restore CppRedundantQualifier

        virtual ~LocalSearchTask() = default;

        // ReSharper disable once CppRedundantQualifier
        void reset(const ::State::State<X, Y, Z, W> inputState) { m_OutputState = inputState; }

        const Score::Score m_InitScore;
        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> m_CurrentState;
        Score::Score m_CurrentScore;

        static constexpr size_t Lh = 100;
        std::array<Score::Score, Lh> m_History {};

        uint32_t m_Iterations = 0;
        uint32_t m_IdleIterations = 0;

        bool m_NewBestFound = false;

        // ReSharper disable CppRedundantQualifier
        void step(const ::Heuristics::HeuristicProvider<X, Y, Z, W> &heuristicProvider) {
            m_NewBestFound = false;

            // Generate new candidate solution
            ::State::State<X, Y, Z, W>& candidateState = m_CurrentState;
            auto perturbator = heuristicProvider[0];
            perturbator->modify(candidateState);
            // `m_CurrentState` is now the new candidate state.

            // Evaluate candidate solution
            const Score::Score candidateScore = Evaluation::evaluateState(candidateState, m_Constraints);
            // std::cout << "Init score: " << initScore << ", candidate score: " << candidateScore << ", diff: " << (m_OutputState.getBitArray().getDifferenceBounds(state.getBitArray())) << std::endl;

            // Track idle iterations (termination criteria)
            if (candidateScore <= m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            // Compute virtual history index
            const uint32_t v = m_Iterations % Lh;
            const Score::Score& fv = m_History[v];

            // Selector (Acceptance criteria)
            if (candidateScore > fv || candidateScore >= m_CurrentScore) {
                // `m_CurrentState = candidateState` assignment is not needed, because `candidateState` is a reference
                // to "working memory" `m_CurrentState`. But we need to update score.
                m_CurrentScore = candidateScore;

                if (m_CurrentScore > m_OutputScore) {
                    // New best state is found. It is already update as the current "working memory" `m_CurrentState`.
                    m_OutputScore = m_CurrentScore;
                    m_OutputState = m_CurrentState;
                    m_NewBestFound = true;
                    #ifdef LOCALSEARCH_DEBUG
                    std::cout << "New best score: " << m_OutputScore << "; iterations=" << (m_Iterations + 1) << "; delta=" << (m_OutputScore - m_InitScore) << std::endl;
                    #endif
                }
            } else {
                // Revert the new candidate state to previous candidate state,
                // because the new candidate state references the "working memory" `m_CurrentState`.
                perturbator->revert(candidateState);
            }

            // Update history
            if (m_CurrentScore > fv) { m_History[v] = m_CurrentScore; }

            // Increment iterations
            ++m_Iterations;
        }
        // ReSharper restore CppRedundantQualifier

        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> getOutputState() const { return m_OutputState; }
        [[nodiscard]] Score::Score getOutputScore() const { return m_OutputScore; }

    private:
        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> m_OutputState;
        Score::Score m_OutputScore;
        // ReSharper disable once CppRedundantQualifier
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& m_Constraints;
    };
}

#endif //LOCALSEARCHTASK_H
