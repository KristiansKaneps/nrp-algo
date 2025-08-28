#ifndef LAHCLOCALSEARCHTASK_H
#define LAHCLOCALSEARCHTASK_H

#include "Search/LocalSearchTask.h"

#include <array>

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class LahcLocalSearchTask : public LocalSearchTask<X, Y, Z, W> {
        using Base = LocalSearchTask<X, Y, Z, W>;
    public:
        struct Params {
            size_t historyLength = 25; // active length <= LhMax
            int maxIdleIterationCount = 5000000;
            int iterAtZeroThreshold = 2000;
            int iterAtFeasibleThreshold = 4000;
            int maxFeasibleIdleIterationCount = 3000;
        };
        LahcLocalSearchTask(const LahcLocalSearchTask&) = delete;

        // ReSharper disable CppRedundantQualifier
        explicit LahcLocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                 const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints,
                                 Statistics::ScoreStatistics &scoreStatistics,
                                 const Params& params = Params{}) noexcept
            : Base(inputState, constraints, scoreStatistics), m_Params(params) {
            if (m_Params.historyLength == 0 || m_Params.historyLength > LhMax) m_Params.historyLength = 25;
            m_History.fill(Base::m_InitScore);
        }
        // ReSharper restore CppRedundantQualifier

        ~LahcLocalSearchTask() noexcept override = default;

        // ReSharper disable once CppRedundantQualifier
        void reset(const ::State::State<X, Y, Z, W> inputState) noexcept override {
            Base::reset(inputState);
            m_History.fill(Base::m_InitScore);
            m_Iterations = 0;
            m_IdleIterations = 0;
            m_IterationCountAtZeroScore = 0;
            m_IterationCountAtFeasibleScore = 0;
            m_RepairPerturbatorsApplied = false;
            m_BestScoreAchievedBeforePerturbationCount = 0;
            // m_AppliedPerturbators.clear();
        }

        void setParams(const Params& params) noexcept {
            m_Params = params;
            if (m_Params.historyLength == 0 || m_Params.historyLength > LhMax) m_Params.historyLength = 25;
            m_History.fill(Base::m_InitScore);
        }

        // ReSharper disable CppRedundantQualifier
        void step(::Heuristics::HeuristicProvider<X, Y, Z, W> &heuristicProvider) noexcept override {
            Base::m_NewBestFound = false;

            // Generate new candidate solution
            ::State::State<X, Y, Z, W>& candidateState = Base::m_CurrentState;
            auto perturbators = heuristicProvider.generateSearchPerturbators(Base::m_Evaluator, candidateState);
            // if (!m_RepairPerturbatorsApplied && m_BestScoreAchievedBeforePerturbationCount > 100000) {
            //     m_RepairPerturbatorsApplied = true;
            //     auto repairPerturbators = heuristicProvider.generateRepairPerturbators(Base::m_Evaluator, candidateState);
            //     repairPerturbators.modify(candidateState);
            //     m_AppliedPerturbators.append(repairPerturbators);
            //     std::cout << "Applying repair perturbators" << std::endl;
            // }
            perturbators.modify(candidateState);
            const Score::Score candidateScore = Base::m_Evaluator.evaluateState(candidateState);

            // Track idle iterations (termination criteria)
            if (candidateScore <= Base::m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            // Compute virtual history index
            const uint32_t v = m_Iterations % static_cast<uint32_t>(m_Params.historyLength);
            const Score::Score& fv = m_History[v];

            // Selector (Acceptance criteria)
            if (candidateScore > fv || candidateScore >= Base::m_CurrentScore) {
                // `m_CurrentState = candidateState` assignment is not needed, because `candidateState` is a reference
                // to "working memory" `m_CurrentState`. But we need to update the score.
                Base::m_CurrentScore = candidateScore;

                if (Base::m_CurrentScore > Base::m_OutputScore) {
                    // The new best state is found.
                    // It is already updated as the current "working memory" `m_CurrentState`.
                    Base::m_OutputScore = Base::m_CurrentScore;
                    Base::m_OutputState = Base::m_CurrentState;
                    Base::m_NewBestFound = true;

                    // Record score statistics
                    Base::m_ScoreStatistics.record(Base::m_CurrentScore);

                    #ifdef LOCALSEARCH_DEBUG
                    std::cout << "New best score: " << Base::m_OutputScore << "; iterations=" << (m_Iterations + 1) << "; delta=" << (Base::m_OutputScore - Base::m_InitScore) << std::endl;
                    // Base::m_Evaluator.printConstraintInfo();
                    #endif

                    m_RepairPerturbatorsApplied = false;
                    m_BestScoreAchievedBeforePerturbationCount = 0;
                } else {
                    m_BestScoreAchievedBeforePerturbationCount += perturbators.size();
                }

                // m_AppliedPerturbators.append(perturbators);
                // std::cout << "Applied perturbators size: " << m_AppliedPerturbators.size() << ", best score achieved before " << m_BestScoreAchievedBeforePerturbationCount << " perturbations; idle iteration count: " << m_IdleIterations << std::endl;
            } else {
                // Revert the new candidate state to the previous candidate state,
                // because the new candidate state references the "working memory" `m_CurrentState`.
                perturbators.revert(candidateState);
            }

            // Update history
            // This should maybe be put into top else branch where state is reverted.
            if (Base::m_CurrentScore > fv) { m_History[v] = Base::m_CurrentScore; }

            // Increment iterations
            ++m_Iterations;
        }
        // ReSharper restore CppRedundantQualifier

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
        std::array<Score::Score, LhMax> m_History {};
        Params m_Params{};

        uint64_t m_Iterations = 0;
        uint64_t m_IdleIterations = 0;

        bool m_RepairPerturbatorsApplied = false;
        size_t m_BestScoreAchievedBeforePerturbationCount {};
        // ::Heuristics::PerturbatorChain<X, Y, Z, W> m_AppliedPerturbators {};
    };
}

#endif //LAHCLOCALSEARCHTASK_H
