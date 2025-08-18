#ifndef SALOCALSEARCHTASK_H
#define SALOCALSEARCHTASK_H

#include "Search/LocalSearchTask.h"
#include "Utils/Random.h"
#include <algorithm>
#include <cmath>

namespace Search::Task {
    template<typename X, typename Y, typename Z, typename W>
    class SaLocalSearchTask : public LocalSearchTask<X, Y, Z, W> {
        using Base = LocalSearchTask<X, Y, Z, W>;
    public:
        SaLocalSearchTask(const SaLocalSearchTask&) = delete;

        explicit SaLocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                   const std::vector<::Constraints::Constraint<X, Y, Z, W>*>& constraints,
                                   Statistics::ScoreStatistics& scoreStatistics) noexcept
                : Base(inputState, constraints, scoreStatistics) {
            resetTemperature();
        }

        ~SaLocalSearchTask() noexcept override = default;

        void reset(const ::State::State<X, Y, Z, W> inputState) noexcept override {
            Base::reset(inputState);
            m_Iterations = 0;
            m_IdleIterations = 0;
            m_IterationCountAtZeroScore = 0;
            m_IterationCountAtFeasibleScore = 0;
            resetTemperature();
        }

        void step(::Heuristics::HeuristicProvider<X, Y, Z, W>& heuristicProvider) noexcept override {
            Base::m_NewBestFound = false;

            ::State::State<X, Y, Z, W>& candidateState = Base::m_CurrentState;
            auto perturbators = heuristicProvider.generateSearchPerturbators(Base::m_Evaluator, candidateState);
            perturbators.modify(candidateState);
            const Score::Score candidateScore = Base::m_Evaluator.evaluateState(candidateState);

            if (candidateScore <= Base::m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            // Lexicographic delta across components
            const Score::score_t delta = computeLexicographicDelta(candidateScore, Base::m_CurrentScore);

            bool accept = delta >= 0;
            if (!accept) {
                const double prob = std::exp(static_cast<double>(delta) / m_Temperature);
                const double u = static_cast<double>(Random::generator().randomFloat(0.0f, 1.0f));
                accept = u < prob;
            }

            if (accept) {
                Base::m_CurrentScore = candidateScore;

                if (Base::m_CurrentScore > Base::m_OutputScore) {
                    Base::m_OutputScore = Base::m_CurrentScore;
                    Base::m_OutputState = Base::m_CurrentState;
                    Base::m_NewBestFound = true;
                    Base::m_ScoreStatistics.record(Base::m_CurrentScore);
                }

                m_AppliedPerturbators.append(perturbators);
            } else {
                perturbators.revert(candidateState);
            }

            coolDown();
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

        // SA parameters (geometric cooling)
        static constexpr double INITIAL_TEMPERATURE = 1000.0;
        static constexpr double MIN_TEMPERATURE = 1e-6;
        static constexpr double COOLING_RATE = 0.995; // T <- alpha * T
        static constexpr uint32_t STEPS_PER_TEMPERATURE = 100;
        static constexpr uint64_t REHEAT_IDLE_THRESHOLD = 20000;
        static constexpr double REHEAT_FACTOR = 2.0; // T <- min(T * factor, INITIAL_TEMPERATURE)

        uint64_t m_Iterations = 0;
        uint64_t m_IdleIterations = 0;

        double m_Temperature = INITIAL_TEMPERATURE;
        uint32_t m_StepsSinceTempUpdate = 0;

        ::Heuristics::PerturbatorChain<X, Y, Z, W> m_AppliedPerturbators{};

        static Score::score_t computeLexicographicDelta(const Score::Score& a, const Score::Score& b) noexcept {
            if (a.strict != b.strict) return a.strict - b.strict;
            if (a.hard != b.hard) return a.hard - b.hard;
            return a.soft - b.soft;
        }

        void resetTemperature() noexcept {
            m_Temperature = INITIAL_TEMPERATURE;
            m_StepsSinceTempUpdate = 0;
        }

        void coolDown() noexcept {
            m_StepsSinceTempUpdate += 1;
            if (m_StepsSinceTempUpdate >= STEPS_PER_TEMPERATURE) {
                m_Temperature = std::max(m_Temperature * COOLING_RATE, MIN_TEMPERATURE);
                m_StepsSinceTempUpdate = 0;
            }

            if (m_IdleIterations > REHEAT_IDLE_THRESHOLD && m_Temperature < INITIAL_TEMPERATURE * 0.5) {
                m_Temperature = std::min(m_Temperature * REHEAT_FACTOR, INITIAL_TEMPERATURE);
                m_IdleIterations = 0;
            }
        }
    };
}

#endif //SALOCALSEARCHTASK_H


