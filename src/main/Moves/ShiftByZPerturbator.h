#ifndef SHIFTBYZPERTURBATOR_H
#define SHIFTBYZPERTURBATOR_H

#include "AutonomousPerturbator.h"

#include "Utils/Random.h"

#include <unordered_map>

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class ShiftByZPerturbator : public AutonomousPerturbator<X, Y, Z, W> {
    public:
        explicit ShiftByZPerturbator() noexcept = default;
        ~ShiftByZPerturbator() noexcept override = default;

        [[nodiscard]] ShiftByZPerturbator *clone() const noexcept override {
            return new ShiftByZPerturbator(*this);
        }

        [[nodiscard]] bool configureIfApplicable(const Evaluation::Evaluator<X, Y, Z, W>& evaluator,
                                                 const ::State::State<X, Y, Z, W>& state) noexcept override {
            return false;
        }

        void configure(const ::State::State<X, Y, Z, W>& state) noexcept override {
            std::vector<axis_size_t> employeesWithWork;
            for (axis_size_t x = 0; x < state.sizeX(); x++) {
                for (axis_size_t z = 0; z < state.sizeZ(); z++) {
                    for (axis_size_t y = 0; y < state.sizeY(); y++) {
                        if (state.get(x, y, z))
                            employeesWithWork.push_back(y);
                    }
                }
            }
            if (employeesWithWork.empty()) return;

            const axis_size_t y = m_Random.choice(employeesWithWork);

            std::vector<::State::Location> assignments;
            for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                    for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                        if (!state.get(x, y, z, w)) continue;
                        assignments.push_back(::State::Location{x, y, z, w});
                    }
                }
            }

            if (assignments.empty()) return;

            std::vector<std::pair<axis_size_t, axis_size_t>> chains;
            axis_size_t start = 0;
            for (size_t i = 1; i < assignments.size(); ++i) {
                if (assignments[i].z != assignments[i - 1].z + 1) {
                    chains.emplace_back(start, i);
                    start = i;
                }
            }
            chains.emplace_back(start, assignments.size());

            const auto& [assignmentStart, assignmentEnd] = m_Random.choice(chains);
            const auto& zStart = assignments[assignmentStart].z;
            const auto& zEnd = assignments[assignmentEnd - 1].z;

            if (-zStart == state.sizeZ() - zEnd) return;

            const auto window = static_cast<int64_t>(state.sizeZ() / 2 - (zEnd - zStart));
            int64_t k = m_Random.randomInt(-window, window - 1);
            if (k >= 0) k += 1;

            for (axis_size_t i = assignmentStart; i < assignmentEnd; ++i) {
                const auto& prevLocation = assignments[i];
                m_UnassignLocations.push_back(prevLocation);
                const auto nextZ = static_cast<axis_size_t>(prevLocation.z + k);
                if (nextZ < 0 || nextZ >= state.sizeZ()) continue;
                const auto nextLocation = ::State::Location{prevLocation.x, prevLocation.y, nextZ, prevLocation.w};
                if (state.get(nextLocation.x, nextLocation.y, nextLocation.z)) continue; // Do not overwrite existing assignments
                m_AssignLocations.emplace_back(nextLocation);
            }
        }

        [[nodiscard]] bool isIdentity() const noexcept override { return m_UnassignLocations.empty() && m_AssignLocations.empty(); }

        void modify(::State::State<X, Y, Z, W>& state) noexcept override {
            for (const auto& unassignLocation : m_UnassignLocations)
                state.clear(unassignLocation);
            for (const auto& assignLocation : m_AssignLocations)
                state.set(assignLocation);
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept override {
            for (const auto& unassignLocation : m_AssignLocations)
                state.clear(unassignLocation);
            for (const auto& assignLocation : m_UnassignLocations)
                state.set(assignLocation);
        }

    protected:
        inline static Random::RandomGenerator& m_Random = Random::generator();

        std::vector<::State::Location> m_UnassignLocations {};
        std::vector<::State::Location> m_AssignLocations {};
    };
}

#endif //SHIFTBYZPERTURBATOR_H
