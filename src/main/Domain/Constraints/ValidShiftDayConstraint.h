#ifndef VALIDSHIFTDAYCONSTRAINT_H
#define VALIDSHIFTDAYCONSTRAINT_H

#include "DomainConstraint.h"

#include "Array/BitMatrix.h"

namespace Domain::Constraints {
    class ValidShiftDayConstraint final : public DomainConstraint {
    public:
        explicit ValidShiftDayConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                         const Axes::Axis<Domain::Shift>& xAxis,
                                         const Axes::Axis<Domain::Day>& zAxis) : Constraint("VALID_SHIFT_DAY"),
            m_AssignableShiftAndDayPairMatrix(BitMatrix::createMatrix(xAxis.size(), zAxis.size())) {
            for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                const auto& day = zAxis[z];

                const auto weekday = Time::InstantToWeekday(day.range().start(), timeZone);

                for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                    const auto& shift = xAxis[x];
                    // TODO: Implement holidays as 8th bit
                    if ((shift.weekdayBitMask() >> weekday & 0b1) == 0) { m_AssignableShiftAndDayPairMatrix.set(x, z); }
                }
            }
        }

        ~ValidShiftDayConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            ConstraintScore totalScore;
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    totalScore.violate(Violation::xz(x, z, {
                                                              -static_cast<score_t>(m_AssignableShiftAndDayPairMatrix.
                                                                  get(x, z))
                                                          }));
                }
            }

            return totalScore;
        }

    private:
        BitMatrix::BitMatrix m_AssignableShiftAndDayPairMatrix;
    };
}

#endif //VALIDSHIFTDAYCONSTRAINT_H
