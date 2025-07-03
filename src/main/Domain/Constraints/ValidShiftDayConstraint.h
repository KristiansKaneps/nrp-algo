#ifndef VALIDSHIFTDAYCONSTRAINT_H
#define VALIDSHIFTDAYCONSTRAINT_H

#include "DomainConstraint.h"
#include "Domain/Moves/ValidShiftDayRepairPerturbator.h"

#include "Array/BitMatrix.h"

namespace Domain::Constraints {
    class ValidShiftDayConstraint final : public DomainConstraint {
    public:
        explicit ValidShiftDayConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                         const Axes::Axis<Domain::Shift>& xAxis,
                                         const axis_size_t yAxisSize,
                                         const Axes::Axis<Domain::Day>& zAxis,
                                         const axis_size_t wAxisSize) noexcept : Constraint("VALID_SHIFT_DAY", {
                new Moves::ValidShiftDayRepairPerturbator(yAxisSize, wAxisSize),
            }),
            m_ShiftAndDayConflictMatrix(BitMatrix::createMatrix(xAxis.size(), zAxis.size())) {
            for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                const auto& day = zAxis[z];

                const auto weekday = Time::InstantToWeekday(day.range().start(), timeZone);

                for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                    const auto& shift = xAxis[x];
                    // TODO: Implement holidays as 8th bit
                    if ((shift.weekdayBitMask() >> weekday & 0b1) == 0) {
                        m_ShiftAndDayConflictMatrix.set(x, z);
                    }
                }
            }
        }

        ~ValidShiftDayConstraint() noexcept override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::DomainState& state) noexcept override {
            ConstraintScore totalScore;
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    if (!state.getXZ(x, z) || !m_ShiftAndDayConflictMatrix.get(x, z)) continue;
                    totalScore.violate(Violation::xz(x, z, {-1}));
                }
            }

            return totalScore;
        }

    private:
        BitMatrix::BitMatrix m_ShiftAndDayConflictMatrix;
    };
}

#endif //VALIDSHIFTDAYCONSTRAINT_H
