#ifndef RESTBETWEENSHIFTSCONSTRAINT_H
#define RESTBETWEENSHIFTSCONSTRAINT_H

#include "DomainConstraint.h"

#include "Array/BitSquareMatrix.h"
#include "Array/BitSymmetricalMatrix.h"

namespace Domain::Constraints {
    class RestBetweenShiftsConstraint final : public DomainConstraint {
    public:
        explicit RestBetweenShiftsConstraint(const Axes::Axis<Domain::Shift>& xAxis) :
            Constraint("REST_BETWEEN_SHIFTS"),
            m_IntersectingShiftsInSameDayMatrix(BitMatrix::createIdentitySymmetricalMatrix(xAxis.size())),
            m_IntersectingShiftsInAdjacentDaysMatrix(BitMatrix::createSquareMatrix(xAxis.size())) {
            for (axis_size_t x1 = 0; x1 < xAxis.size(); ++x1) {
                const auto interval1 = xAxis[x1].interval();
                const auto paddedInterval1 = xAxis[x1].interval().withPadding(12 * 60);

                if (
                    paddedInterval1.intersectsOtherFromNextDay(interval1)
                    || paddedInterval1.intersectsOtherFromPrevDay(interval1)
                )
                    m_IntersectingShiftsInSameDayMatrix.set(x1, x1);

                for (axis_size_t x2 = x1 + 1; x2 < xAxis.size(); ++x2) {
                    const auto interval2 = xAxis[x2].interval();
                    const auto paddedInterval2 = xAxis[x2].interval().withPadding(12 * 60);

                    if (interval1.intersectsInSameDay(paddedInterval2) || paddedInterval1.intersectsInSameDay(interval2))
                        m_IntersectingShiftsInSameDayMatrix.set(x1, x2);
                    if (interval1.intersectsOtherFromNextDay(paddedInterval2) || paddedInterval1.intersectsOtherFromNextDay(interval2))
                        m_IntersectingShiftsInAdjacentDaysMatrix.set(x1, x2);
                    if (interval1.intersectsOtherFromPrevDay(paddedInterval2) || paddedInterval1.intersectsOtherFromPrevDay(interval2))
                        m_IntersectingShiftsInAdjacentDaysMatrix.set(x2, x1);
                }
            }
        }

        ~RestBetweenShiftsConstraint() override = default;

        [[nodiscard]] Score::Score evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            score_t totalScore = 0;
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                score_t employeeScore = 0;
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    score_t dayScore = 0;

                    // Check same day intersections
                    for (axis_size_t x1 = 0; x1 < state.sizeX() - 1; ++x1) {
                        if (!state.get(x1, y, z)) continue; // not assigned
                        for (axis_size_t x2 = x1 + 1; x2 < state.sizeX(); ++x2) {
                            if (!state.get(x2, y, z)) continue; // not assigned
                            dayScore -= m_IntersectingShiftsInSameDayMatrix.get(x1, x2);
                        }
                    }

                    // Check previous and next day intersections
                    if (z > 0) [[likely]] {
                        for (axis_size_t x1 = 0; x1 < state.sizeX(); ++x1) {
                            if (!state.get(x1, y, z - 1)) continue; // not assigned
                            for (axis_size_t x2 = 0; x2 < state.sizeX(); ++x2) {
                                if (!state.get(x2, y, z)) continue; // not assigned
                                dayScore -= m_IntersectingShiftsInAdjacentDaysMatrix.get(x1, x2) +
                                    m_IntersectingShiftsInAdjacentDaysMatrix.get(x2, x1);
                            }
                        }
                    }

                    employeeScore += dayScore;
                }

                totalScore += employeeScore;
            }

            return {.strict = totalScore, .hard = 0, .soft = 0};
        }

    private:
        BitMatrix::BitSymmetricalMatrix m_IntersectingShiftsInSameDayMatrix;
        /**
         * If x > y, then the bit is set if x (in the previous day) intersects y (in the next day).<br>
         * If y > x, then the bit is set if y (in the previous day) intersects x (in the next day).<br>
         * If x == y, then the bit is set if x (in the previous day) intersects y (in the next day) or the other way
         * around.
         */
        BitMatrix::BitSquareMatrix m_IntersectingShiftsInAdjacentDaysMatrix;
    };
}

#endif //RESTBETWEENSHIFTSCONSTRAINT_H
