#ifndef LOCALSEARCHTASK_H
#define LOCALSEARCHTASK_H

#include "Evaluation.h"
#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Score/Score.h"
#include "Utils/Random.h"

#include <array>

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearchTask {
    public:
        LocalSearchTask(const LocalSearchTask&) = delete;

        // ReSharper disable once CppRedundantQualifier
        explicit LocalSearchTask(const State::State<X, Y, Z, W> inputState,
                                 const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints) :
            m_InitScore(Evaluation::evaluateState(inputState, constraints)),
            m_CurrentState(inputState),
            m_CurrentScore(m_InitScore),
            m_OutputState(inputState),
            m_OutputScore(m_InitScore),
            m_Constraints(constraints) {
            m_History.fill(m_InitScore);
        }

        virtual ~LocalSearchTask() = default;

        void reset(const State::State<X, Y, Z, W> inputState) { m_OutputState = inputState; }

        const Score::Score m_InitScore;
        State::State<X, Y, Z, W> m_CurrentState;
        Score::Score m_CurrentScore;

        static constexpr size_t Lh = 100;
        std::array<Score::Score, Lh> m_History {};

        uint32_t m_Iterations = 0;
        uint32_t m_IdleIterations = 0;

        bool m_NewBestFound = false;

        void step() {
            m_NewBestFound = false;

            // Generate new candidate solution
            State::State<X, Y, Z, W> candidateState = m_CurrentState;
            const uint32_t moveCount = m_Random.randomInt(1, 2);
            for (uint32_t i = 0; i < moveCount; ++i) {
                const auto x = m_Random.randomInt(0, m_OutputState.sizeX() - 1);
                const auto y = m_Random.randomInt(0, m_OutputState.sizeY() - 1);
                const auto z = m_Random.randomInt(0, m_OutputState.sizeZ() - 1);
                const auto w = m_Random.randomInt(0, m_OutputState.sizeW() - 1);
                candidateState.assign(x, y, z, w, m_Random.randomInt(0, 1));
            }

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
                m_CurrentState = candidateState;
                m_CurrentScore = candidateScore;

                if (m_CurrentScore > m_OutputScore) {
                    m_OutputScore = m_CurrentScore;
                    m_OutputState = m_CurrentState;
                    m_NewBestFound = true;
                    #ifdef LOCALSEARCH_DEBUG
                    std::cout << "New best score: " << m_OutputScore << "; iterations=" << (m_Iterations + 1) << "; delta=" << (m_OutputScore - m_InitScore) << std::endl;
                    #endif
                }
            }

            // Update history
            if (m_CurrentScore > fv) { m_History[v] = m_CurrentScore; }

            // Increment iterations
            ++m_Iterations;
        }

        State::State<X, Y, Z, W> getOutputState() const { return m_OutputState; }
        [[nodiscard]] Score::Score getOutputScore() const { return m_OutputScore; }

    private:
        inline static Random::RandomGenerator m_Random {};
        State::State<X, Y, Z, W> m_OutputState;
        Score::Score m_OutputScore;
        // ReSharper disable once CppRedundantQualifier
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& m_Constraints;
    };
}

#endif //LOCALSEARCHTASK_H
