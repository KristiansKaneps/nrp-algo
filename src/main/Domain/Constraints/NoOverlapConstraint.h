#ifndef NOOVERLAPCONSTRAINT_H
#define NOOVERLAPCONSTRAINT_H

#include "DomainConstraint.h"

#include "Array/BitSquareMatrix.h"
#include "Array/BitSymmetricalMatrix.h"

namespace Domain::Constraints {
    class NoOverlapConstraint final : public DomainConstraint {
    public:
        explicit NoOverlapConstraint(const Axes::Axis<Domain::Shift>& xAxis) : Constraint("NO_OVERLAP", {
                                                                               }),
                                                                               m_IntersectingShiftsInSameDayMatrix(
                                                                                   BitMatrix::createIdentitySymmetricalMatrix(
                                                                                       xAxis.size())),
                                                                               m_IntersectingShiftsInAdjacentDaysMatrix(
                                                                                   BitMatrix::createSquareMatrix(
                                                                                       xAxis.size())) {
            if (xAxis.size() == 0) [[unlikely]] return;
            for (axis_size_t x1 = 0; x1 < xAxis.size() - 1; ++x1) {
                const auto& s1 = xAxis[x1];
                const auto& interval1 = s1.interval();
                for (axis_size_t x2 = x1 + 1; x2 < xAxis.size(); ++x2) {
                    const auto& s2 = xAxis[x2];
                    const auto& interval2 = s2.interval();

                    if (interval1.intersectsInSameDay(interval2))
                        m_IntersectingShiftsInSameDayMatrix.set(x1, x2);
                    if (interval1.intersectsOtherInNextDay(interval2) || s1.blocksShiftIndex(x2))
                        m_IntersectingShiftsInAdjacentDaysMatrix.set(x1, x2);
                    if (interval1.intersectsOtherInPrevDay(interval2) || s2.blocksShiftIndex(x1))
                        m_IntersectingShiftsInAdjacentDaysMatrix.set(x2, x1);
                }
            }
        }

        ~NoOverlapConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::DomainState& state) override {
            ConstraintScore totalScore;
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    // Check same-day intersections
                    for (axis_size_t x1 = 0; x1 < state.sizeX() - 1; ++x1) {
                        if (!state.get(x1, y, z)) continue; // not assigned
                        for (axis_size_t x2 = x1 + 1; x2 < state.sizeX(); ++x2) {
                            if (!state.get(x2, y, z) || !m_IntersectingShiftsInSameDayMatrix.get(x1, x2)) continue
                                ; // not assigned or not intersecting
                            totalScore.violate(Violation::xyz(x1, y, z, {-1}));
                            totalScore.violate(Violation::xyz(x2, y, z, {-1}));
                        }
                    }

                    // Check previous and next day intersections
                    if (z > 0) [[likely]] {
                        for (axis_size_t x1 = 0; x1 < state.sizeX(); ++x1) {
                            if (!state.get(x1, y, z - 1)) continue; // not assigned
                            for (axis_size_t x2 = 0; x2 < state.sizeX(); ++x2) {
                                if (!state.get(x2, y, z) || (!m_IntersectingShiftsInAdjacentDaysMatrix.get(x1, x2) && !
                                    m_IntersectingShiftsInAdjacentDaysMatrix.get(x2, x1))) continue
                                    ; // not assigned or not intersecting
                                totalScore.violate(Violation::xyz(x1, y, z, {-1}));
                                totalScore.violate(Violation::xyz(x2, y, z, {-1}));
                            }
                        }
                    }
                }
            }

            return totalScore;
        }

    private:
        BitMatrix::BitSymmetricalMatrix m_IntersectingShiftsInSameDayMatrix;
        /**
         * The bit is set if x (in the previous day) intersects y (in the next day).<br>
         * In other words, x corresponds to the previous day and y corresponds to the next day.<br>
         * Single shift's maximum duration limit is 24H.
         */
        BitMatrix::BitSquareMatrix m_IntersectingShiftsInAdjacentDaysMatrix;
    };
}

#endif //NOOVERLAPCONSTRAINT_H
