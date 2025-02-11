#ifndef EMPLOYMENTMAXDURATIONCONSTRAINT_H
#define EMPLOYMENTMAXDURATIONCONSTRAINT_H

#include <chrono>

#include "Constraints/Constraint.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Constraints {
    class EmploymentMaxDurationConstraint final : public Constraint<
            Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill> {
    public:
        explicit EmploymentMaxDurationConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                                 const Axes::Axis<Domain::Shift>& xAxis,
                                                 const Axes::Axis<Domain::Employee>& yAxis,
                                                 const Axes::Axis<Domain::Day>& zAxis) : Constraint(
            "EMPLOYMENT_MAX_DURATION") {
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

        ~EmploymentMaxDurationConstraint() override {
            delete[] mp_ShiftDurationInMinutes;
            mp_ShiftDurationInMinutes = nullptr;
        };

        [[nodiscard]] Score::Score evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            score_t totalScore = 0;

            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                score_t employeeScore = 0;

                const auto& e = state.y()[y];

                int64_t totalDurationInMinutes {};

                for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                    for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                        if (!state.get(x, y, z)) continue;
                        // ReSharper disable once CppDFANullDereference
                        totalDurationInMinutes += static_cast<int64_t>(mp_ShiftDurationInMinutes[x * state.sizeZ() +
                            z]);
                    }
                }

                const int64_t diff = 168 * 60L - totalDurationInMinutes;

                if (diff < 0) { employeeScore += diff; }

                totalScore += employeeScore;
            }

            return {.strict = totalScore, .hard = 0, .soft = 0};
        }

    private:
        int32_t *mp_ShiftDurationInMinutes {};
    };
}

#endif //EMPLOYMENTMAXDURATIONCONSTRAINT_H
