#ifndef DLASLOCALSEARCHTASK_H
#define DLASLOCALSEARCHTASK_H

#include "Search/LocalSearchTask.h"
#include <array>
#include <algorithm>

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class DlasLocalSearchTask : public LocalSearchTask<X, Y, Z, W> {
        using Base = LocalSearchTask<X, Y, Z, W>;
    public:
        struct Params {
            size_t historyLength = 25; // active length <= LhMax
            int maxIdleIterationCount = 5000000;
            int iterAtZeroThreshold = 2000;
            int iterAtFeasibleThreshold = 4000;
            int maxFeasibleIdleIterationCount = 3000;
        };
        DlasLocalSearchTask(const DlasLocalSearchTask&) = delete;

        explicit DlasLocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                     const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints,
                                     Statistics::ScoreStatistics &scoreStatistics,
                                     const Params& params = Params{}) noexcept
            : Base(inputState, constraints, scoreStatistics), m_Params(params) {
            if (m_Params.historyLength == 0 || m_Params.historyLength > LhMax) m_Params.historyLength = 25;
            m_History.fill(Base::m_InitScore);
            m_PhiMin = Base::m_InitScore;
            m_N = m_Params.historyLength;
        }

        ~DlasLocalSearchTask() noexcept override = default;

        void reset(const ::State::State<X, Y, Z, W> inputState) noexcept override {
            Base::reset(inputState);
            m_History.fill(Base::m_InitScore);
            m_PhiMin = Base::m_InitScore;
            m_N = m_Params.historyLength;
            m_Iterations = 0;
            m_IdleIterations = 0;
            m_IterationCountAtZeroScore = 0;
            m_IterationCountAtFeasibleScore = 0;
            // m_AppliedPerturbators.clear();
        }

        void setParams(const Params& params) noexcept {
            m_Params = params;
            if (m_Params.historyLength == 0 || m_Params.historyLength > LhMax) m_Params.historyLength = 25;
            m_History.fill(Base::m_InitScore);
            m_PhiMin = Base::m_InitScore;
            m_N = m_Params.historyLength;
        }

        void step(::Heuristics::HeuristicProvider<X, Y, Z, W> &heuristicProvider) noexcept override {
            Base::m_NewBestFound = false;

            // Save previous fitness value
            const Score::Score prevScore = Base::m_CurrentScore;

            // Generate new candidate solution
            ::State::State<X, Y, Z, W>& candidateState = Base::m_CurrentState;
            auto perturbators = heuristicProvider.generateSearchPerturbators(Base::m_Evaluator, candidateState);
            perturbators.modify(candidateState);
            const Score::Score candidateScore = Base::m_Evaluator.evaluateState(candidateState);

            // Track idle iterations (termination criteria)
            if (candidateScore <= Base::m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            // Compute history index
            const uint32_t l = m_Iterations % static_cast<uint32_t>(m_Params.historyLength);
            Score::Score& phi_l = m_History[l];

            // DLAS acceptance criteria
            if (candidateScore == Base::m_CurrentScore || candidateScore > m_PhiMin) {
                // Accept candidate
                // `m_CurrentState = candidateState` assignment is not needed, because `candidateState` is a reference
                // to "working memory" `m_CurrentState`. But we need to update the score.
                Base::m_CurrentScore = candidateScore;

                if (Base::m_CurrentScore > Base::m_OutputScore) {
                    // New best state found
                    Base::m_OutputScore = Base::m_CurrentScore;
                    Base::m_OutputState = Base::m_CurrentState;
                    Base::m_NewBestFound = true;
                    Base::m_ScoreStatistics.record(Base::m_CurrentScore);

                    #ifdef LOCALSEARCH_DEBUG
                    std::cout << "New best score: " << Base::m_OutputScore << "; iterations=" << (m_Iterations + 1) << "; delta=" << (Base::m_OutputScore - Base::m_InitScore) << std::endl;
                    // Base::m_Evaluator.printConstraintInfo();
                    #endif
                }

                // Update history
                if (Base::m_CurrentScore < phi_l) {
                    phi_l = Base::m_CurrentScore;
                } else if (Base::m_CurrentScore > phi_l && Base::m_CurrentScore > prevScore) {
                    if (phi_l == m_PhiMin) {
                        m_N--;
                    }
                    phi_l = Base::m_CurrentScore;
                    if (m_N == 0) {
                        // Recompute PhiMin and N
                        m_PhiMin = *std::ranges::min_element(m_History);
                        m_N = std::ranges::count(m_History, m_PhiMin);
                    }
                }

                // m_AppliedPerturbators.append(perturbators);
                // std::cout << "Applied perturbators size: " << m_AppliedPerturbators.size() << ", best score achieved before " << m_BestScoreAchievedBeforePerturbationCount << " perturbations; idle iteration count: " << m_IdleIterations << std::endl;
            } else {
                // Revert candidate
                perturbators.revert(candidateState);
            }

            ++m_Iterations;
        }

        [[nodiscard]] bool shouldStep() noexcept override {
            if (Base::m_OutputScore.isZero()) [[unlikely]] {
                if (m_IterationCountAtZeroScore >= static_cast<uint64_t>(m_Params.iterAtZeroThreshold)) [[unlikely]] return m_IdleIterations < static_cast<uint64_t>(m_Params.maxFeasibleIdleIterationCount) >> 1;
                m_IterationCountAtZeroScore += 1;
                return true;
            }
            if (Base::m_OutputScore.isFeasible()) [[unlikely]] {
                if (m_IterationCountAtFeasibleScore >= static_cast<uint64_t>(m_Params.iterAtFeasibleThreshold)) [[unlikely]] return m_IdleIterations < static_cast<uint64_t>(m_Params.maxFeasibleIdleIterationCount);
                m_IterationCountAtFeasibleScore += 1;
                return true;
            }
            return m_IdleIterations < static_cast<uint64_t>(m_Params.maxIdleIterationCount);
        }

    private:
        uint64_t m_IterationCountAtZeroScore = 0, m_IterationCountAtFeasibleScore = 0;

        static constexpr size_t LhMax = 256;
        std::array<Score::Score, LhMax> m_History{};
        Score::Score m_PhiMin;
        size_t m_N = 25;

        Params m_Params{};

        uint64_t m_Iterations = 0;
        uint64_t m_IdleIterations = 0;

        size_t m_BestScoreAchievedBeforePerturbationCount {};
        // ::Heuristics::PerturbatorChain<X, Y, Z, W> m_AppliedPerturbators {};
    };
}

#endif //DLASLOCALSEARCHTASK_H 