#ifndef SHIFTCOVERAGECONSTRAINT_H
#define SHIFTCOVERAGECONSTRAINT_H

#include "DomainConstraint.h"

#include <chrono>

#include "Time/Range.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Day.h"

namespace Domain::Constraints {
    class ShiftCoverageConstraint final : public DomainConstraint {
    public:
        explicit ShiftCoverageConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                         const Axes::Axis<Domain::Shift>& xAxis,
                                         const Axes::Axis<Domain::Day>& zAxis) : Constraint("SHIFT_COVERAGE", {}),
            m_CoverageData(xAxis.size() * zAxis.size(), CoverageData {}) {
            using std::chrono_literals::operator ""min;
            for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                const auto& s = xAxis[x];

                for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                    const auto day = range.getDayAt(z, timeZone);
                    const auto shiftRange = s.interval().toRange(day);
                    const auto shiftDuration = shiftRange.duration<std::chrono::minutes>(timeZone);
                    m_CoverageData[x * zAxis.size() + z] = {
                        s.slotCount(z),
                        s.requiredSlotCount(z),
                        static_cast<int32_t>(shiftDuration / 1min),
                    };
                }
            }
        }

        ~ShiftCoverageConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(const State::DomainState& state) override {
            ConstraintScore totalScore;
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    const auto& [slotCount, requiredSlotCount, durationInMinutes] = m_CoverageData[x * state.sizeZ() +
                        z];

                    score_t absDayScore = 0;

                    axis_size_t assignedEmployeeCount = 0;
                    for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                        assignedEmployeeCount += static_cast<axis_size_t>(state.get(x, y, z));
                    }

                    if (slotCount != 0 && assignedEmployeeCount > slotCount) {
                        absDayScore += static_cast<score_t>(assignedEmployeeCount - static_cast<axis_size_t>(slotCount))
                            *
                            durationInMinutes;
                    }

                    if (assignedEmployeeCount < requiredSlotCount) {
                        absDayScore += static_cast<score_t>(static_cast<axis_size_t>(requiredSlotCount) -
                                assignedEmployeeCount)
                            * durationInMinutes;
                    }

                    const score_t dayScore = -(absDayScore * absDayScore);

                    totalScore.violate(Violation::xz(x, z, {0, dayScore, 0}));
                }
            }

            return totalScore;
        }

    private:
        struct CoverageData {
            uint8_t slotCount;
            uint8_t requiredSlotCount;
            int32_t durationInMinutes;
        };

        std::vector<CoverageData> m_CoverageData;
    };
}

#endif //SHIFTCOVERAGECONSTRAINT_H
