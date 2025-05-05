#ifndef EMPLOYMENTMAXDURATIONCONSTRAINT_H
#define EMPLOYMENTMAXDURATIONCONSTRAINT_H

#include <chrono>

#include "DomainConstraint.h"

namespace Domain::Constraints {
    class EmploymentMaxDurationConstraint final : public DomainConstraint {
    public:
        explicit EmploymentMaxDurationConstraint(const Time::Range& range,
                                                 const axis_size_t partitionSize,
                                                 const std::chrono::time_zone *timeZone,
                                                 const Axes::Axis<Domain::Shift>& xAxis,
                                                 const Axes::Axis<Domain::Employee>& yAxis,
                                                 const Axes::Axis<Domain::Day>& zAxis) : Constraint(
                "EMPLOYMENT_MAX_DURATION", {
                }),
            m_WorkdayCount(range.getWorkdayCount(timeZone)),
            m_PartialWorkdayCount(range.getPartialWorkdayCount(timeZone)),
            m_PartitionSize(zAxis.size() < partitionSize ? zAxis.size() : partitionSize),
            m_PartitionCount((zAxis.size() - 1 + m_PartitionSize) / m_PartitionSize),
            m_WorkloadDurationInRange(m_WorkdayCount * 8 * 60),
            m_ShiftDurationInMinutes(xAxis.size() * zAxis.size(), 0) {

            for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                const auto& s = xAxis[x];

                for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                    const auto day = range.getDayAt(z, timeZone);
                    const auto shiftRange = s.interval().toRange(day);
                    const auto shiftDuration = shiftRange.duration<std::chrono::minutes>(timeZone);
                    m_ShiftDurationInMinutes[x * zAxis.size() + z] = shiftDuration.count();
                }
            }

            m_PartitionRanges.reserve(m_PartitionCount);
            for (axis_size_t p = 0; p < m_PartitionCount; ++p) {
                const axis_size_t start = p * m_PartitionSize;
                axis_size_t end = start + m_PartitionSize;
                if (end > zAxis.size()) [[unlikely]] end = zAxis.size();
                const auto rangePartition = range.getRangePartitionByDays(static_cast<size_t>(start), static_cast<size_t>(partitionSize), timeZone);
                const auto workdayCount = range.getWorkdayCount(timeZone);
                const auto partialWorkdayCount = range.getPartialWorkdayCount(timeZone);
                const auto factor = partialWorkdayCount / m_PartialWorkdayCount;
                m_PartitionRanges.emplace_back(PartitionRange{start, end, workdayCount, partialWorkdayCount, factor});
            }
        }

        ~EmploymentMaxDurationConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::DomainState& state) override {
            ConstraintScore totalScore;

            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
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
                            totalDurationInMinutes += static_cast<int64_t>(m_ShiftDurationInMinutes[x * state.sizeZ() +
                                z]);
                        }
                    }

                    const int64_t diff = maxWorkloadDurationInMinutes - totalDurationInMinutes;
                    const int64_t diffScale = diff > 0 ? 2 : 1; // seems to balance workload between employees
                    const int64_t overtimeDiff = diff + maxWorkloadOvertimeDurationInMinutes;

                    constexpr int64_t ABS_DIFF_ALLOWANCE = 3 * 60;
                    const int64_t absDiff = abs(diff);

                    const score_t absHard = (absDiff - 1) * diffScale / ABS_DIFF_ALLOWANCE; // scale with larger differences

                    const score_t strict = -(overtimeDiff < 0);
                    const score_t hard = -(absHard * absHard);

                    if (strict != 0 || hard != 0) { totalScore.violate(Violation::yw(y, w, {strict, hard})); }
                }
            }

            return totalScore;
        }

    private:
        struct PartitionRange;

        const int32_t m_WorkdayCount;
        const float m_PartialWorkdayCount;
        const axis_size_t m_PartitionSize;
        const axis_size_t m_PartitionCount;
        const int64_t m_WorkloadDurationInRange;
        std::vector<int32_t> m_ShiftDurationInMinutes;
        std::vector<PartitionRange> m_PartitionRanges;

        struct PartitionRange {
            axis_size_t start, end;
            int32_t workdayCount;
            float partialWorkdayCount;
            float factor;
        };
    };
}

#endif //EMPLOYMENTMAXDURATIONCONSTRAINT_H
