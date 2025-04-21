#ifndef EMPLOYMENTMAXDURATIONCONSTRAINT_H
#define EMPLOYMENTMAXDURATIONCONSTRAINT_H

#include <chrono>

#include "DomainConstraint.h"

namespace Domain::Constraints {
    class EmploymentMaxDurationConstraint final : public DomainConstraint {
    public:
        explicit EmploymentMaxDurationConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                                 const Axes::Axis<Domain::Shift>& xAxis,
                                                 const Axes::Axis<Domain::Employee>& yAxis,
                                                 const Axes::Axis<Domain::Day>& zAxis) : Constraint(
                "EMPLOYMENT_MAX_DURATION"),
            m_WorkloadDurationInRange(range.getWorkdayCount(timeZone) * 8 * 60) {
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
        }

        [[nodiscard]] ConstraintScore evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            ConstraintScore totalScore;

            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                score_t employeeStrictScore = 0;
                score_t employeeHardScore = 0;

                const auto& e = state.y()[y];

                for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                    const auto& skillIndex = state.w()[w].index();
                    const auto *s = e.skill(skillIndex);

                    int64_t maxWorkloadDurationInMinutes = 0;
                    int64_t maxWorkloadOvertimeDurationInMinutes = 0;

                    if (s != nullptr && s->strategy != Workload::Strategy::NONE) {
                        maxWorkloadOvertimeDurationInMinutes = static_cast<int64_t>(s->event.maxOvertimeHours * 60L);
                        if (s->strategy == Workload::Strategy::STATIC) {
                            maxWorkloadDurationInMinutes = static_cast<int64_t>(static_cast<double>(
                                m_WorkloadDurationInRange) * s->event.staticLoad);
                        } else if (s->strategy == Workload::Strategy::DYNAMIC) {
                            maxWorkloadDurationInMinutes = static_cast<int64_t>(s->event.dynamicLoadHours * 60L);
                        }
                    }

                    int64_t totalDurationInMinutes {};

                    for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                        for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                            if (!state.get(x, y, z, w)) continue;
                            // ReSharper disable once CppDFANullDereference
                            totalDurationInMinutes += static_cast<int64_t>(mp_ShiftDurationInMinutes[x * state.sizeZ() +
                                z]);
                        }
                    }

                    const int64_t diff = maxWorkloadDurationInMinutes - totalDurationInMinutes;
                    const int64_t overtimeDiff = diff + maxWorkloadOvertimeDurationInMinutes;

                    constexpr int64_t ABS_DIFF_ALLOWANCE = 3 * 60;
                    const int64_t absDiff = abs(diff);

                    const score_t strict = -(overtimeDiff < 0);
                    const score_t hard = -((absDiff - 1) / ABS_DIFF_ALLOWANCE); // scale with larger differences

                    if (strict != 0 || hard != 0) { totalScore.violate(Violation::yw(y, w, {strict, hard})); }
                }
            }

            return totalScore;
        }

    private:
        const int64_t m_WorkloadDurationInRange;
        int32_t *mp_ShiftDurationInMinutes {};
    };
}

#endif //EMPLOYMENTMAXDURATIONCONSTRAINT_H
