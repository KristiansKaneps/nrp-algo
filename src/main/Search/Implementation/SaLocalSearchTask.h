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
        struct Params {
            double initialTemperature = 20000.0;
            double minTemperature = 1e-8;
            double coolingRate = 0.999; // T <- alpha * T (slower cooling, higher acceptance)
            uint32_t stepsPerTemperature = 600;
            uint64_t reheatIdleThreshold = 8000;
            double reheatFactor = 2.5; // T <- min(T * factor, INITIAL_TEMPERATURE)
            // Component-wise acceptance control (relative to base temperature)
            // Lower multipliers => rarer acceptance for worsening that component
            double hardTempMultiplier = 1.0;   // baseT * 1.0 for hard component
            double strictTempMultiplier = 0.5; // baseT * 0.5 for strict component
            // Compound move settings
            uint32_t minPerturbatorsPerStep = 3;  // at cold temperatures
            uint32_t maxPerturbatorsPerStep = 16; // at hot temperatures
            // Floor acceptance probabilities at max temperature for worsening moves
            double baseHardWorsenAcceptProb = 0.6;   // at full temperature
            double baseStrictWorsenAcceptProb = 0.3; // at full temperature
            // High-temperature energy-based acceptance (weighted sum of components)
            bool useEnergyAcceptanceHighTemp = false;
            double energyTempThreshold = 0.7; // switch to energy-based when T/initial >= threshold
            double energyWeightStrict = 1e6;
            double energyWeightHard = 1e3;
            double energyWeightSoft = 1.0;
            // Global acceptance floor at high temperature (applies after main rule)
            double globalAcceptFloor = 0.1;
        };

        SaLocalSearchTask(const SaLocalSearchTask&) = delete;

        explicit SaLocalSearchTask(const ::State::State<X, Y, Z, W> inputState,
                                   const std::vector<::Constraints::Constraint<X, Y, Z, W>*>& constraints,
                                   Statistics::ScoreStatistics& scoreStatistics,
                                   const Params& params = Params{}) noexcept
                : Base(inputState, constraints, scoreStatistics), m_Params(params) {
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
            // m_AppliedPerturbators.clear();
        }

        void setParams(const Params& params) noexcept {
            m_Params = params;
            resetTemperature();
        }

        void step(::Heuristics::HeuristicProvider<X, Y, Z, W>& heuristicProvider) noexcept override {
            Base::m_NewBestFound = false;

            ::State::State<X, Y, Z, W>& candidateState = Base::m_CurrentState;

            // Temperature-scaled compound moves: apply more perturbators when hot, fewer when cold
            const double tempRatio = std::clamp(m_Temperature / m_Params.initialTemperature, 0.0, 1.0);
            const uint32_t movesThisStep = m_Params.minPerturbatorsPerStep + static_cast<uint32_t>(
                std::round(tempRatio * static_cast<double>(m_Params.maxPerturbatorsPerStep - m_Params.minPerturbatorsPerStep))
            );

            ::Heuristics::PerturbatorChain<X, Y, Z, W> compoundPerturbators{};
            for (uint32_t i = 0; i < movesThisStep; ++i) {
                auto chain = heuristicProvider.generateSearchPerturbators(Base::m_Evaluator, candidateState);
                if (!chain.empty()) {
                    compoundPerturbators.append(chain);
                    chain.modify(candidateState);
                }
            }
            const Score::Score candidateScore = Base::m_Evaluator.evaluateState(candidateState);

            if (candidateScore <= Base::m_CurrentScore) { m_IdleIterations++; } else { m_IdleIterations = 0; }

            // Acceptance policy (annealed lexicographic):
            // - If strict improves, accept.
            // - Else if strict worsens, accept with prob exp(dStrict / (T * strictMult)).
            // - If strict equal and hard improves, accept.
            // - Else if hard worsens, accept with prob exp(dHard / (T * hardMult)).
            // - If strict and hard equal, apply SA on soft with base T.
            const Score::score_t dStrict = candidateScore.strict - Base::m_CurrentScore.strict;
            const Score::score_t dHard = candidateScore.hard - Base::m_CurrentScore.hard;
            const Score::score_t dSoft = candidateScore.soft - Base::m_CurrentScore.soft;

            bool accept = false;
            const double tStrict = std::max(m_Temperature * m_Params.strictTempMultiplier, 1e-12);
            const double tHard = std::max(m_Temperature * m_Params.hardTempMultiplier, 1e-12);

            if (m_Params.useEnergyAcceptanceHighTemp && tempRatio >= m_Params.energyTempThreshold) {
                // Energy-based acceptance at high temperature (softens lexicographic dominance)
                const double dE = static_cast<double>(dStrict) * m_Params.energyWeightStrict
                                  + static_cast<double>(dHard) * m_Params.energyWeightHard
                                  + static_cast<double>(dSoft) * m_Params.energyWeightSoft;
                if (dE >= 0.0) {
                    accept = true;
                } else {
                    const double prob = std::exp(dE / std::max(m_Temperature, 1e-12));
                    const double u = static_cast<double>(Random::generator().randomFloat(0.0f, 1.0f));
                    accept = u < prob;
                }
            } else {
                // Annealed lexicographic acceptance with floors
                if (dStrict > 0) {
                    accept = true;
                } else if (dStrict < 0) {
                    const double probBoltz = std::exp(static_cast<double>(dStrict) / tStrict);
                    const double probFloor = m_Params.baseStrictWorsenAcceptProb * tempRatio;
                    const double prob = std::max(probBoltz, probFloor);
                    const double u = static_cast<double>(Random::generator().randomFloat(0.0f, 1.0f));
                    accept = u < prob;
                } else { // dStrict == 0
                    if (dHard > 0) {
                        accept = true;
                    } else if (dHard < 0) {
                        const double probBoltz = std::exp(static_cast<double>(dHard) / tHard);
                        const double probFloor = m_Params.baseHardWorsenAcceptProb * tempRatio;
                        const double prob = std::max(probBoltz, probFloor);
                        const double u = static_cast<double>(Random::generator().randomFloat(0.0f, 1.0f));
                        accept = u < prob;
                    } else { // dHard == 0
                        if (dSoft >= 0) {
                            accept = true;
                        } else {
                            const double prob = std::exp(static_cast<double>(dSoft) / m_Temperature);
                            const double u = static_cast<double>(Random::generator().randomFloat(0.0f, 1.0f));
                            accept = u < prob;
                        }
                    }
                }
            }

            // Global acceptance floor when very hot
            if (!accept) {
                const double floorProb = m_Params.globalAcceptFloor * tempRatio;
                const double u2 = static_cast<double>(Random::generator().randomFloat(0.0f, 1.0f));
                if (u2 < floorProb) accept = true;
            }

            if (accept) {
                Base::m_CurrentScore = candidateScore;

                if (Base::m_CurrentScore > Base::m_OutputScore) {
                    Base::m_OutputScore = Base::m_CurrentScore;
                    Base::m_OutputState = Base::m_CurrentState;
                    Base::m_NewBestFound = true;
                    Base::m_ScoreStatistics.record(Base::m_CurrentScore);
                }

                // m_AppliedPerturbators.append(compoundPerturbators);
            } else {
                compoundPerturbators.revert(candidateState);
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
        Params m_Params{};

        uint64_t m_Iterations = 0;
        uint64_t m_IdleIterations = 0;

        double m_Temperature = 0.0;
        uint32_t m_StepsSinceTempUpdate = 0;

        // ::Heuristics::PerturbatorChain<X, Y, Z, W> m_AppliedPerturbators{};

        static Score::score_t computeLexicographicDelta(const Score::Score& a, const Score::Score& b) noexcept {
            if (a.strict != b.strict) return a.strict - b.strict;
            if (a.hard != b.hard) return a.hard - b.hard;
            return a.soft - b.soft;
        }

        void resetTemperature() noexcept {
            m_Temperature = m_Params.initialTemperature;
            m_StepsSinceTempUpdate = 0;
        }

        void coolDown() noexcept {
            m_StepsSinceTempUpdate += 1;
            if (m_StepsSinceTempUpdate >= m_Params.stepsPerTemperature) {
                m_Temperature = std::max(m_Temperature * m_Params.coolingRate, m_Params.minTemperature);
                m_StepsSinceTempUpdate = 0;
            }

            if (m_IdleIterations > m_Params.reheatIdleThreshold && m_Temperature < m_Params.initialTemperature * 0.5) {
                m_Temperature = std::min(m_Temperature * m_Params.reheatFactor, m_Params.initialTemperature);
                m_IdleIterations = 0;
            }
        }
    };
}

#endif //SALOCALSEARCHTASK_H


