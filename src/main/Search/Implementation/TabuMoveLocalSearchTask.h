#ifndef TABUMOVELOCALSEARCHTASK_H
#define TABUMOVELOCALSEARCHTASK_H

#include "Search/LocalSearchTask.h"
#include "Array/BitArray.h"

#include <unordered_map>
#include <deque>

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class TabuMoveLocalSearchTask : public LocalSearchTask<X, Y, Z, W> {
        using Base = LocalSearchTask<X, Y, Z, W>;
    public:
        TabuMoveLocalSearchTask(const TabuMoveLocalSearchTask&) = delete;

        explicit TabuMoveLocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                         const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints,
                                         Statistics::ScoreStatistics& scoreStatistics,
                                         const uint32_t tabuTenure = 256) noexcept
                : Base(inputState, constraints, scoreStatistics), m_TabuTenure(tabuTenure) { }

        ~TabuMoveLocalSearchTask() noexcept override = default;

        void reset(const ::State::State<X, Y, Z, W> inputState) noexcept override {
            Base::reset(inputState);
            m_Iterations = 0;
            m_IdleIterations = 0;
            m_IterationCountAtZeroScore = 0;
            m_IterationCountAtFeasibleScore = 0;
            m_TabuQueue.clear();
            m_TabuMap.clear();
        }

        void step(::Heuristics::HeuristicProvider<X, Y, Z, W>& heuristicProvider) noexcept override {
            Base::m_NewBestFound = false;

            ::State::State<X, Y, Z, W>& candidateState = Base::m_CurrentState;
            auto perturbators = heuristicProvider.generateSearchPerturbators(Base::m_Evaluator, candidateState);

            // Snapshot previous bit-array to compute changed positions
            const auto& prevBits = candidateState.getBitArray();
            BitArray::BitArray prevSnapshot(prevBits);

            // Apply move to get candidate
            perturbators.modify(candidateState);

            // Compute move signature from bit differences (attribute-level)
            const uint64_t moveSig = computeMoveSignature(prevSnapshot, candidateState.getBitArray());
            const Score::Score candidateScore = Base::m_Evaluator.evaluateState(candidateState);

            if (candidateScore <= Base::m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            const bool isTabu = m_TabuMap.find(moveSig) != m_TabuMap.end();
            const bool aspiration = candidateScore > Base::m_OutputScore;

            bool accept = false;
            if (!isTabu) {
                accept = candidateScore >= Base::m_CurrentScore;
            } else {
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
                pushTabu(moveSig);
            } else {
                perturbators.revert(candidateState);
            }

            ++m_Iterations;
        }

        [[nodiscard]] bool shouldStep() noexcept override {
            if (Base::m_OutputScore.isZero()) [[unlikely]] {
                if (m_IterationCountAtZeroScore >= ITERATION_COUNT_AT_ZERO_SCORE_THRESHOLD) [[unlikely]] return m_IdleIterations < MAX_FEASIBLE_IDLE_ITERATION_COUNT >> 1;
                m_IterationCountAtZeroScore += 1;
                return true;
            }
            if (Base::m_OutputScore.isFeasible()) [[unlikely]] {
                if (m_IterationCountAtFeasibleScore >= ITERATION_COUNT_AT_FEASIBLE_SCORE_THRESHOLD) [[unlikely]] return m_IdleIterations < MAX_FEASIBLE_IDLE_ITERATION_COUNT;
                m_IterationCountAtFeasibleScore += 1;
                return true;
            }
            return m_IdleIterations < MAX_IDLE_ITERATION_COUNT;
        }

    private:
        static constexpr int ITERATION_COUNT_AT_ZERO_SCORE_THRESHOLD = 2000;
        static constexpr int ITERATION_COUNT_AT_FEASIBLE_SCORE_THRESHOLD = 4000;
        static constexpr int MAX_FEASIBLE_IDLE_ITERATION_COUNT = 3000;
        static constexpr int MAX_IDLE_ITERATION_COUNT = 500000;
        uint64_t m_IterationCountAtZeroScore = 0, m_IterationCountAtFeasibleScore = 0;

        uint64_t m_Iterations = 0;
        uint64_t m_IdleIterations = 0;

        uint32_t m_TabuTenure;
        std::deque<uint64_t> m_TabuQueue{};
        std::unordered_map<uint64_t, uint32_t> m_TabuMap{}; // sig -> count in queue

        static uint64_t rotl(const uint64_t x, const uint8_t r) noexcept {
            return (x << r) | (x >> (64 - r));
        }

        [[nodiscard]] uint64_t computeMoveSignature(const BitArray::BitArray& before,
                                                    const BitArray::BitArray& after) const noexcept {
            const BitArray::array_size_t size = before.size();
            uint64_t h = 1469598103934665603ULL; // FNV offset basis
            for (BitArray::array_size_t i = 0; i < size; i += 64) {
                const uint64_t bw = before.word(i);
                const uint64_t aw = after.word(i);
                const uint64_t delta = bw ^ aw; // toggled bits
                if (delta == 0ULL) continue;
                h ^= delta; h *= 1099511628211ULL; h = rotl(h, 13);
            }
            // Also mix count of changed bits for extra discrimination
            h ^= after.count() - before.count();
            return h;
        }

        void pushTabu(const uint64_t sig) noexcept {
            m_TabuQueue.push_back(sig);
            m_TabuMap[sig] += 1;
            while (m_TabuQueue.size() > m_TabuTenure) {
                const uint64_t old = m_TabuQueue.front();
                m_TabuQueue.pop_front();
                if (auto it = m_TabuMap.find(old); it != m_TabuMap.end()) {
                    if (it->second <= 1) m_TabuMap.erase(it); else it->second -= 1;
                }
            }
        }
    };
}

#endif //TABUMOVELOCALSEARCHTASK_H


