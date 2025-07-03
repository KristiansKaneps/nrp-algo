#ifndef RANKEDINTERSECTIONTOGGLEPERTURBATOR_H
#define RANKEDINTERSECTIONTOGGLEPERTURBATOR_H

#include "AutonomousPerturbator.h"

#include "Utils/Random.h"

#include <unordered_map>

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class RankedIntersectionTogglePerturbator : public AutonomousPerturbator<X, Y, Z, W> {
    public:
        explicit RankedIntersectionTogglePerturbator(const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints) noexcept {
            for (size_t i = 0; i < constraints.size(); ++i) {
                const auto* constraint = constraints[i];
                if (constraint->name() == "EMPLOYMENT_MAX_DURATION")
                    m_EmployeeMaxDurationConstraintIndex = i;
                if (constraint->name() == "SHIFT_COVERAGE")
                    m_CoverageConstraintIndex = i;
            }

            assert(m_EmployeeMaxDurationConstraintIndex != m_CoverageConstraintIndex && "Could not find necessary constraints");

            // TODO: determine min Z width to assign at once (for now it is 2).
            //   There must always be a probability that only 1 z will be assigned (for later on in the search process).
        }
        ~RankedIntersectionTogglePerturbator() noexcept override = default;

        [[nodiscard]] RankedIntersectionTogglePerturbator *clone() const noexcept override {
            return new RankedIntersectionTogglePerturbator(*this);
        }

        [[nodiscard]] bool configureIfApplicable(const Evaluation::Evaluator<X, Y, Z, W>& evaluator,
                                                 const ::State::State<X, Y, Z, W>& state) noexcept override {
            const auto& coverageConstraintScore = evaluator.constraintScores()[m_CoverageConstraintIndex];
            const auto& maxDurationConstraintScore = evaluator.constraintScores()[m_EmployeeMaxDurationConstraintIndex];

            if ((coverageConstraintScore.score().isFeasible() && maxDurationConstraintScore.score().isFeasible()) || (coverageConstraintScore.violations().empty() || maxDurationConstraintScore.violations().empty()))
                return false;

            // max duration constraint: info = 2 if you can assign more shifts, 1 if max workload is already reached
            // y!, w?
            const size_t maxDurationConstraintViolationIndex = m_Random.randomInt(0, maxDurationConstraintScore.violations().size() - 1);
            // coverage constraint: info = 2 if you can assign more employees to shift, 1 if max employees per this shift is already reached
            // x!, z!
            const size_t coverageConstraintViolationIndex = m_Random.randomInt(0, coverageConstraintScore.violations().size() - 1);

            // for (size_t i = 0; i < maxDurationConstraintScore.violations().size(); ++i) {
            //     for (; coverageConstraintViolationIndex < coverageConstraintScore.violations().size(); ++coverageConstraintViolationIndex) {
            //     }
            // }
            const auto& maxDurationViolation = maxDurationConstraintScore.violations()[maxDurationConstraintViolationIndex];
            const auto& coverageViolation = coverageConstraintScore.violations()[coverageConstraintViolationIndex];

            if (maxDurationViolation.hasW()) {
                const auto location = ::State::Location {
                    coverageViolation.getX(),
                    maxDurationViolation.getY(),
                    coverageViolation.getZ(),
                    maxDurationViolation.getW()
                };

                if (maxDurationViolation.info == 2 && coverageViolation.info == 2) {
                    if (!state.get(location)) {
                        m_LocationXors.emplace(location, 1);
                        if (state.sizeZ() > 1 && m_Random.randomInt(0, 10) < 8) {
                            if (location.z + 1 == state.sizeZ()) {
                                m_LocationXors.emplace(location.withZ(location.z - 1), 1);
                            } else {
                                m_LocationXors.emplace(location.withZ(location.z + 1), 1);
                            }
                        }
                    }
                } else if (state.get(location)) {
                    m_LocationXors.emplace(location, 1);
                    if (state.sizeZ() > 1 && m_Random.randomInt(0, 10) < 8) {
                        if (location.z + 1 == state.sizeZ()) {
                            m_LocationXors.emplace(location.withZ(location.z - 1), 1);
                        } else {
                            m_LocationXors.emplace(location.withZ(location.z + 1), 1);
                        }
                    }
                }
            } else {
                const axis_size_t x = coverageViolation.getX();
                const axis_size_t y = maxDurationViolation.getY();
                const axis_size_t z = coverageViolation.getZ();

                if (maxDurationViolation.info == 2 && coverageViolation.info == 2) {
                    if (!state.get(x, y, z)) {
                        const auto location = ::State::Location{x, y, z, m_Random.randomInt(0, state.sizeW() - 1)};
                        m_LocationXors.emplace(location, 1);
                        if (state.sizeZ() > 1 && m_Random.randomInt(0, 10) < 8) {
                            if (location.z + 1 == state.sizeZ()) {
                                m_LocationXors.emplace(location.withZ(location.z - 1), 1);
                            } else {
                                m_LocationXors.emplace(location.withZ(location.z + 1), 1);
                            }
                        }
                    }
                } else {
                    for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                        if (state.get(x, y, z, w)) {
                            m_LocationXors.emplace(::State::Location{x, y, z, w}, 1);
                            if (state.sizeZ() > 1 && m_Random.randomInt(0, 10) < 8) {
                                if (z + 1 == state.sizeZ()) {
                                    m_LocationXors.emplace(::State::Location {x, y, z - 1, w}, 1);
                                } else {
                                    m_LocationXors.emplace(::State::Location {x, y, z + 1, w}, 1);
                                }
                            }
                            break;
                        }
                    }
                }
            }

            return !m_LocationXors.empty();
        }

        void configure(const ::State::State<X, Y, Z, W>& state) noexcept override {

        }

        [[nodiscard]] bool isIdentity() const noexcept override { return m_LocationXors.empty(); }

        void modify(::State::State<X, Y, Z, W>& state) noexcept override { apply(state); }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept override { apply(state); }

    protected:
        inline static Random::RandomGenerator& m_Random = Random::generator();

        size_t m_CoverageConstraintIndex, m_EmployeeMaxDurationConstraintIndex;

        std::unordered_map<::State::Location, uint8_t> m_LocationXors {};

        void apply(::State::State<X, Y, Z, W>& state) const noexcept {
            for (const auto& [loc, val] : m_LocationXors) {
                state.assign(loc, state.get(loc) ^ val);
            }
        }
    };
}

#endif //RANKEDINTERSECTIONTOGGLEPERTURBATOR_H
