#ifndef SHIFTCOVERAGECONSTRAINT_H
#define SHIFTCOVERAGECONSTRAINT_H

#include "DomainConstraint.h"

#include <chrono>

#include "Time/Range.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Domain::Constraints {
    class ShiftCoverageConstraint final : public DomainConstraint {
    public:
        explicit ShiftCoverageConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                         const Axes::Axis<Domain::Shift>& xAxis,
                                         const Axes::Axis<Domain::Day>& zAxis) : Constraint("SHIFT_COVERAGE") {
            mp_ShiftDurationInMinutes = new int32_t[xAxis.size() * zAxis.size()];

            for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                const auto& s = xAxis[x];

                for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                    const auto day = range.getDayAt(z, timeZone);
                    const auto shiftRange = s.interval().toRange(day);
                    const auto shiftDuration = shiftRange.duration<std::chrono::minutes>(timeZone);
                    mp_ShiftDurationInMinutes[x * zAxis.size() + z] = shiftDuration.count();
                }
            }
        }

        ~ShiftCoverageConstraint() override {
            delete[] mp_ShiftDurationInMinutes;
            mp_ShiftDurationInMinutes = nullptr;
        };

        [[nodiscard]] ConstraintScore evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            ConstraintScore totalScore;
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                const auto& s = state.x()[x];
                const uint8_t slotCount = s.slotCount();
                const uint8_t reqSlotCount = s.slotCount();
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    score_t dayScore = 0;

                    axis_size_t assignedEmployeeCount = 0;
                    for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                        assignedEmployeeCount += static_cast<axis_size_t>(state.get(x, y, z));
                    }

                    const auto shiftDurationInMinutes = mp_ShiftDurationInMinutes[x * state.sizeZ() + z];

                    if (slotCount != 0 && assignedEmployeeCount > slotCount) {
                        dayScore -= static_cast<score_t>(assignedEmployeeCount - static_cast<axis_size_t>(slotCount)) *
                            shiftDurationInMinutes;
                    }

                    if (assignedEmployeeCount < reqSlotCount) {
                        dayScore -= static_cast<score_t>(static_cast<axis_size_t>(reqSlotCount) - assignedEmployeeCount)
                            * shiftDurationInMinutes;
                    }

                    totalScore.violate(Violation::xz(x, z, {0, dayScore, 0}));
                }
            }

            return totalScore;
        }

    private:
        int32_t *mp_ShiftDurationInMinutes {};
    };
}

#endif //SHIFTCOVERAGECONSTRAINT_H
