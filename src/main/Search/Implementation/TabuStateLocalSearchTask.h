#ifndef TABUSTATELOCALSEARCHTASK_H
#define TABUSTATELOCALSEARCHTASK_H

#include "Search/LocalSearchTask.h"
#include "Array/BitArray.h"

#include <unordered_set>
#include <deque>
#include <cstdint>

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class TabuStateLocalSearchTask : public LocalSearchTask<X, Y, Z, W> {
        using Base = LocalSearchTask<X, Y, Z, W>;
    public:
        struct Params {
            uint32_t tabuTenure = 1024;
            int maxIdleIterationCount = 500000;
            int iterAtZeroThreshold = 2000;
            int iterAtFeasibleThreshold = 4000;
            int maxFeasibleIdleIterationCount = 3000;
        };
        TabuStateLocalSearchTask(const TabuStateLocalSearchTask&) = delete;

        explicit TabuStateLocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                     const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints,
                                     Statistics::ScoreStatistics& scoreStatistics,
                                     const Params& params = Params{}) noexcept
                : Base(inputState, constraints, scoreStatistics), m_Params(params), m_TabuTenure(params.tabuTenure) { }

        ~TabuStateLocalSearchTask() noexcept override = default;

        void reset(const ::State::State<X, Y, Z, W> inputState) noexcept override {
            Base::reset(inputState);
            m_Iterations = 0;
            m_IdleIterations = 0;
            m_IterationCountAtZeroScore = 0;
            m_IterationCountAtFeasibleScore = 0;
            m_TabuQueue.clear();
            m_TabuSet.clear();
            // m_AppliedPerturbators.clear();
        }

        void setParams(const Params& params) noexcept {
            m_Params = params;
            m_TabuTenure = params.tabuTenure;
        }

        void step(::Heuristics::HeuristicProvider<X, Y, Z, W>& heuristicProvider) noexcept override {
            Base::m_NewBestFound = false;

            // Generate new candidate solution
            ::State::State<X, Y, Z, W>& candidateState = Base::m_CurrentState;
            auto perturbators = heuristicProvider.generateSearchPerturbators(Base::m_Evaluator, candidateState);
            perturbators.modify(candidateState);
            const Score::Score candidateScore = Base::m_Evaluator.evaluateState(candidateState);

            // Track idle iterations (termination criteria)
            if (candidateScore <= Base::m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            // Compute state hash for tabu check
            const uint64_t candidateHash = computeStateHash(candidateState);
            const bool isTabu = m_TabuSet.contains(candidateHash);
            const bool aspiration = candidateScore > Base::m_OutputScore;

            bool accept = false;
            if (!isTabu) {
                // Greedy acceptance (lexicographic non-worsening)
                accept = candidateScore >= Base::m_CurrentScore;
            } else {
                // Aspiration criterion: allow tabu if strictly improves global best
                accept = aspiration;
            }

            if (accept) {
                Base::m_CurrentScore = candidateScore;

                if (Base::m_CurrentScore > Base::m_OutputScore) {
                    Base::m_OutputScore = Base::m_CurrentScore;
                    Base::m_OutputState = Base::m_CurrentState;
                    Base::m_NewBestFound = true;
                    Base::m_ScoreStatistics.record(Base::m_CurrentScore);
                }

                // m_AppliedPerturbators.append(perturbators);
                pushTabu(candidateHash);
            } else {
                // Revert the candidate state
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

        uint64_t m_Iterations = 0;
        uint64_t m_IdleIterations = 0;

        Params m_Params{};
        uint32_t m_TabuTenure;
        std::deque<uint64_t> m_TabuQueue{};
        std::unordered_set<uint64_t> m_TabuSet{};

        // ::Heuristics::PerturbatorChain<X, Y, Z, W> m_AppliedPerturbators{};

        static uint64_t rotl(const uint64_t x, const uint8_t r) noexcept {
            return (x << r) | (x >> (64 - r));
        }

        [[nodiscard]] uint64_t computeStateHash(const ::State::State<X, Y, Z, W>& state) const noexcept {
            const auto& bits = state.getBitArray();
            const BitArray::array_size_t size = bits.size();
            uint64_t h = 1469598103934665603ULL; // FNV offset basis
            for (BitArray::array_size_t i = 0; i < size; i += 64) {
                const uint64_t w = bits.word(i);
                h ^= w;
                h *= 1099511628211ULL; // FNV prime
                h = rotl(h, 13);
            }
            // Mix in size to avoid collisions for different sizes
            h ^= size + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            return h;
        }

        void pushTabu(const uint64_t hash) noexcept {
            m_TabuQueue.push_back(hash);
            m_TabuSet.insert(hash);
            while (m_TabuQueue.size() > m_TabuTenure) {
                const uint64_t old = m_TabuQueue.front();
                m_TabuQueue.pop_front();
                if (auto it = m_TabuSet.find(old); it != m_TabuSet.end()) m_TabuSet.erase(it);
            }
        }
    };
}

#endif //TABUSTATELOCALSEARCHTASK_H


