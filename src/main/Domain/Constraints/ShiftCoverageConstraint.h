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
                                         const Axes::Axis<Domain::Day>& zAxis) noexcept : Constraint("SHIFT_COVERAGE", {}),
            m_CoverageData(xAxis.size() * zAxis.size(), CoverageData {}),
            m_WorkloadDurationInRange(range.getWorkdayCount(timeZone) * 8 * 60) {
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

        ~ShiftCoverageConstraint() noexcept override = default;

        [[nodiscard]] ConstraintScore evaluate(const State::DomainState& state) noexcept override {
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

            // for (axis_size_t x = 0; x < state.sizeX(); ++x) {
            //     for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
            //         const auto& cd = m_CoverageData[x * state.sizeZ() + z];
            //         const int req  = cd.requiredSlotCount;
            //         const int max  = cd.slotCount == 0 ? req : cd.slotCount; // treat 0 = unlimited as req
            //         const int durM = cd.durationInMinutes;
            //
            //         /* -----------------------------------------------------------
            //          * Gather coverage + workload data for this shift
            //          * ----------------------------------------------------------- */
            //         int assigned        = 0;
            //         int64_t freeMinutes = 0; // minutes still free before hitting max
            //         int64_t needMinutes = 0; // minutes still missing to reach min
            //
            //         for (axis_size_t y = 0; y < state.sizeY(); ++y) {
            //             const auto [remainingCapacity, missingToMin] = employeeAssignmentDuration(state, y);
            //             needMinutes += missingToMin;
            //             if (!state.get(x, y, z)) continue;
            //             ++assigned;
            //             freeMinutes += remainingCapacity;
            //         }
            //
            //         const int diff = assigned - req; // (+) over, (‑) under
            //         const bool under = diff < 0;
            //         const bool over  = diff > 0 && assigned > max;
            //
            //         /* -----------------------------------------------------------
            //          * Strict violation
            //          * ----------------------------------------------------------- */
            //         // const score_t strict = -(under || over);
            //         const score_t strict = -(under || over);
            //
            //         /* -----------------------------------------------------------
            //          * Tandem scaling factor
            //          *   • plentyFree  = fraction of minutes still free (<1)
            //          *   • needFactor  = fraction still below min   (≥1)
            //          * ----------------------------------------------------------- */
            //         const double rosterFree =
            //             std::clamp<double>(static_cast<double>(freeMinutes) /
            //                                std::max<int64_t>(1, state.sizeY() * 60 * 2), 0.0, 1.0);
            //         const double rosterNeed =
            //             std::clamp<double>(static_cast<double>(needMinutes) /
            //                                std::max<int64_t>(1, state.sizeY() * 60 * 2), 0.0, 2.0);
            //
            //         // If many minutes are free, we scale UP the cost of understaffing
            //         const double scaleUnder = 1.0 + rosterFree;   // 1..2
            //         // If people still need minutes to reach their minimum, having too FEW
            //         // assignments also hurts more; having too MANY helps them, so down‑scale
            //         const double scaleOver  = 1.0 / (1.0 + rosterNeed); // <=1
            //
            //         /* -----------------------------------------------------------
            //          * Progressive hard cost
            //          * ----------------------------------------------------------- */
            //         score_t hard = 0;
            //         if (under) {
            //             const int missing = -diff; // positive
            //             hard = static_cast<score_t>(
            //                      -std::min<int>(missing * durM / 15 * scaleUnder, 1000));
            //         } else if (over) {
            //             const int excess = assigned - max;
            //             hard = static_cast<score_t>(
            //                      -std::min<int>(excess * durM / 15 * scaleOver, 1000));
            //         }
            //
            //         /* -----------------------------------------------------------
            //          * Info code
            //          * ----------------------------------------------------------- */
            //         Violation::info_t info = under ? 2 : (over ? 1 : 0);
            //
            //         if (strict != 0 || hard != 0)
            //             totalScore.violate(Violation::xz(x, z, {strict, hard, 0}, info));
            //     }
            // }

            return totalScore;
        }

    private:
        struct CoverageData {
            uint8_t slotCount;
            uint8_t requiredSlotCount;
            int32_t durationInMinutes;
        };

        struct EmployeeAssignmentDuration {
            int64_t remainingCapacity;
            int64_t missingToMin;
        };

        std::vector<CoverageData> m_CoverageData;
        const int64_t m_WorkloadDurationInRange;

        EmployeeAssignmentDuration employeeAssignmentDuration(const State::DomainState& st, const axis_size_t y) const noexcept {
            const auto& employee = st.y()[y];
            const auto& totalChangeEvent = employee.totalChangeEvent();

            // TODO: Total change event should be calculated in Employee class if input data does not provide it.

            const auto maxTotalWorkloadDurationInMinutes = static_cast<int64_t>(totalChangeEvent.maxLoadHours * 60L);
            const auto maxTotalOvertimeDurationInMinutes = static_cast<int64_t>(totalChangeEvent.maxOvertimeHours * 60L);

            int64_t totalDurationInMinutes {};

            for (axis_size_t w = 0; w < st.sizeW(); ++w) {
                for (axis_size_t x = 0; x < st.sizeX(); ++x) {
                    for (axis_size_t z = 0; z < st.sizeZ(); ++z) {
                        if (!st.get(x, y, z, w)) continue;
                        // ReSharper disable once CppDFANullDereference
                        totalDurationInMinutes += static_cast<int64_t>(m_CoverageData[x * st.sizeZ() + z].durationInMinutes);
                    }
                }
            }

            return EmployeeAssignmentDuration {
                std::max<int64_t>(0, maxTotalWorkloadDurationInMinutes + maxTotalOvertimeDurationInMinutes - totalDurationInMinutes),
                std::max<int64_t>(0, maxTotalWorkloadDurationInMinutes - totalDurationInMinutes),
            };
        }
    };
}

#endif //SHIFTCOVERAGECONSTRAINT_H
