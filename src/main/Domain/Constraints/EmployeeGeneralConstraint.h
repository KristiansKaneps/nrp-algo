#ifndef EMPLOYEEGENERALCONSTRAINT_H
#define EMPLOYEEGENERALCONSTRAINT_H

#include "DomainConstraint.h"
#include "Domain/State/DomainState.h"

namespace Domain::Constraints {
    class EmployeeGeneralConstraint final : public DomainConstraint {
    public:
        explicit EmployeeGeneralConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                           const Axes::Axis<Domain::Shift>& xAxis,
                                           const Axes::Axis<Domain::Day>& zAxis) noexcept : Constraint(
                "EMPLOYEE_GENERAL_CONSTRAINT", {}),
            m_Weekends(zAxis.size()),
            m_ShiftDurationInMinutes(xAxis.size() * zAxis.size(), 0) {
            for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                const auto& d = zAxis[z];
                const auto weekday = Time::InstantToWeekday(range.getDayAt(z, timeZone));
                m_Weekends.assign(z, weekday == 5 || weekday == 6);

                const auto day = range.getDayAt(z, timeZone);

                for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                    const auto& s = xAxis[x];
                    const auto shiftRange = s.interval().toRange(day);
                    const auto shiftDuration = shiftRange.duration<std::chrono::minutes>(timeZone);
                    m_ShiftDurationInMinutes[x * zAxis.size() + z] = shiftDuration.count();
                }
            }
        }

        ~EmployeeGeneralConstraint() noexcept override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::DomainState& state) noexcept override {
            ConstraintScore totalScore;
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                const auto& g = state.y()[y].generalConstraints();
                uint8_t consecutiveShiftCount = 0;
                uint8_t consecutiveDaysOffCount = 0;
                int16_t workingWeekendCount = 0;

                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    bool wasWorking = false;
                    int32_t minutes = 0;
                    for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                        if (state.get(x, y, z)) {
                            wasWorking = true;
                            minutes = m_ShiftDurationInMinutes[x];
                            break;
                        }
                    }

                    if (wasWorking) {
                        consecutiveDaysOffCount = 0;
                        consecutiveShiftCount = 1;
                        // Comment out the next first line and uncomment the next second line
                        // if working weekends corresponds to planning horizon:
                        // workingWeekendCount = static_cast<int16_t>(m_Weekends[z]);
                        workingWeekendCount += m_Weekends[z];
                    } else {
                        consecutiveDaysOffCount = 1;
                        consecutiveShiftCount = 0;
                        // Comment out the next line
                        // if working weekends corresponds to planning horizon:
                        // workingWeekendCount = 0;
                    }

                    if (z + 1 >= state.sizeZ()) {
                        if (g.maxWorkingWeekendCount >= 0 && workingWeekendCount > g.maxWorkingWeekendCount) {
                            totalScore.violate(Violation::yz(y, z, {0, -minutes / 2}));
                        }
                    }

                    for (axis_size_t z1 = z + 1; z1 < state.sizeZ(); ++z1) {
                        bool isWorkingConsecutively = false;
                        for (axis_size_t x1 = 0; x1 < state.sizeX(); ++x1) {
                            if (state.get(x1, y, z1)) {
                                isWorkingConsecutively = true;
                                break;
                            }
                        }

                        if (wasWorking && isWorkingConsecutively) {
                            consecutiveShiftCount += 1;
                            workingWeekendCount += m_Weekends[z1];
                            // Check max working days
                            if (g.maxConsecutiveShiftCount > 0 && consecutiveShiftCount > g.maxConsecutiveShiftCount) {
                                totalScore.violate(Violation::yz(y, z1, {-1}));
                            }
                            if (g.maxWorkingWeekendCount >= 0 && workingWeekendCount > g.maxWorkingWeekendCount) {
                                totalScore.violate(Violation::yz(y, z1, {0, -minutes / 2}));
                            }
                        } else if (wasWorking) {
                            z = z1 - 1;
                            // Check min working days
                            if (consecutiveShiftCount < g.minConsecutiveShiftCount) {
                                totalScore.violate(Violation::yz(y, z, {-1}));
                            }
                            break;
                        } else if (!isWorkingConsecutively) {
                            consecutiveDaysOffCount += 1;
                            // Check max days off
                        } else {
                            z = z1 - 1;
                            // Check min days off
                            if (consecutiveDaysOffCount < g.minConsecutiveDaysOffCount) {
                                totalScore.violate(Violation::yz(y, z1, {-1}));
                            }
                            break;
                        }
                    }
                }
            }

            return totalScore;
        }

    private:
        BitArray::BitArray m_Weekends;

        std::vector<int32_t> m_ShiftDurationInMinutes;
    };
}

#endif //EMPLOYEEGENERALCONSTRAINT_H
