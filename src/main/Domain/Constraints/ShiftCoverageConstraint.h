#ifndef SHIFTCOVERAGECONSTRAINT_H
#define SHIFTCOVERAGECONSTRAINT_H

#include "Constraints/Constraint.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Constraints {
    class ShiftCoverageConstraint final : public Constraint<
            Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill> {
    public:
        explicit ShiftCoverageConstraint(const Axes::Axis<Domain::Shift>& xAxis) : Constraint(
            "SHIFT_COVERAGE") { }

        ~ShiftCoverageConstraint() override = default;

        [[nodiscard]] Score::Score evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            score_t totalScore = 0;
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                score_t shiftScore = 0;
                const auto& s = state.x()[x];
                const axis_size_t slotCount = s.slotCount();
                const axis_size_t reqSlotCount = s.slotCount();
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    score_t dayScore = 0;

                    axis_size_t assignedEmployeeCount = 0;
                    for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                        assignedEmployeeCount += static_cast<axis_size_t>(state.get(x, y, z));
                    }

                    if (slotCount != 0 && assignedEmployeeCount > slotCount) {
                        dayScore -= static_cast<score_t>(assignedEmployeeCount - slotCount);
                    }

                    if (assignedEmployeeCount < reqSlotCount) {
                        dayScore -= static_cast<score_t>(reqSlotCount - assignedEmployeeCount);
                    }

                    shiftScore += dayScore;
                }

                totalScore += shiftScore;
            }

            return {.strict = totalScore, .hard = 0, .soft = 0};
        }

    private:
    };
}

#endif //SHIFTCOVERAGECONSTRAINT_H
