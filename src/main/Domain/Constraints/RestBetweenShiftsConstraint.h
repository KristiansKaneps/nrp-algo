#ifndef RESTBETWEENSHIFTSCONSTRAINT_H
#define RESTBETWEENSHIFTSCONSTRAINT_H

#include "DomainConstraint.h"

#include "Array/BitSquareMatrix.h"
#include "Array/BitSymmetricalMatrix.h"

namespace Domain::Constraints {
    class RestBetweenShiftsConstraint final : public DomainConstraint {
    public:
        explicit RestBetweenShiftsConstraint(const Axes::Axis<Domain::Shift>& xAxis) :
            Constraint("REST_BETWEEN_SHIFTS", {}),
            m_IntersectingShiftsInSameDayMatrix(BitMatrix::createIdentitySymmetricalMatrix(xAxis.size())) {
            int32_t maxDuration = 0;
            for (axis_size_t i = 0; i < xAxis.size(); ++i) {
                const auto& shift = xAxis[i];
                const auto& duration = shift.interval().durationInMinutes();
                if (duration + shift.restMinutesBefore() > maxDuration)
                    maxDuration = duration + shift.restMinutesBefore();
                if (duration + shift.restMinutesAfter() > maxDuration)
                    maxDuration = duration + shift.restMinutesAfter();
            }
            m_MaxOffsetDays = (maxDuration + Time::DailyInterval::MINUTES_IN_A_DAY - 1) /
                Time::DailyInterval::MINUTES_IN_A_DAY;
            m_IntersectingShiftsInAdjacentDaysMatrices.reserve(m_MaxOffsetDays);

            for (axis_size_t x1 = 0; x1 < xAxis.size(); ++x1) {
                const auto& shift1 = xAxis[x1];
                const auto& interval1 = shift1.interval();
                const auto paddedInterval1 = interval1.withPadding(shift1.restMinutesBefore(),
                                                                   shift1.restMinutesAfter());

                for (axis_size_t x2 = x1; x2 < xAxis.size(); ++x2) {
                    const auto& shift2 = xAxis[x2];
                    const auto& interval2 = shift2.interval();

                    if (const auto paddedInterval2 = interval2.
                            withPadding(shift2.restMinutesBefore(), shift2.restMinutesAfter()); interval1.
                        intersectsInSameDay(paddedInterval2) || paddedInterval1.intersectsInSameDay(interval2))
                        m_IntersectingShiftsInSameDayMatrix.set(x1, x2);
                }
            }

            for (int32_t i = 0; i < m_MaxOffsetDays; ++i) {
                const int32_t offsetDay = i + 1;
                auto matrix = BitMatrix::createSquareMatrix(xAxis.size());

                for (axis_size_t x1 = 0; x1 < xAxis.size(); ++x1) {
                    const auto& shift1 = xAxis[x1];
                    const auto& interval1 = shift1.interval();
                    const auto paddedInterval1 = interval1.withPadding(shift1.restMinutesBefore(),
                                                                       shift1.restMinutesAfter());

                    for (axis_size_t x2 = x1; x2 < xAxis.size(); ++x2) {
                        const auto& shift2 = xAxis[x2];
                        const auto& interval2 = shift2.interval();
                        const auto paddedInterval2 = interval2.withPadding(
                            shift2.restMinutesBefore(), shift2.restMinutesAfter());

                        if (interval1.intersectsOtherInOffsetDay(paddedInterval2, offsetDay) || paddedInterval1.
                            intersectsOtherInOffsetDay(interval2, offsetDay))
                            matrix.set(x1, x2);
                        if (interval1.intersectsOtherInOffsetDay(paddedInterval2, -offsetDay) || paddedInterval1.
                            intersectsOtherInOffsetDay(interval2, -offsetDay))
                            matrix.set(x2, x1);
                    }
                }

                m_IntersectingShiftsInAdjacentDaysMatrices.emplace_back(matrix);
            }
        }

        ~RestBetweenShiftsConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(const State::DomainState& state) override {
            ConstraintScore totalScore;
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                axis_size_t z = 0;

                // Check same-day intersections for z = 0
                for (axis_size_t x1 = 0; x1 < state.sizeX() - 1; ++x1) {
                    if (!state.get(x1, y, z)) continue; // not assigned
                    for (axis_size_t x2 = x1 + 1; x2 < state.sizeX(); ++x2) {
                        if (!state.get(x2, y, z) || !m_IntersectingShiftsInSameDayMatrix.get(x1, x2))
                            continue; // not assigned or not intersecting
                        totalScore.violate(Violation::xyz(x1, y, z, {-1}));
                        totalScore.violate(Violation::xyz(x2, y, z, {-1}));
                    }
                }

                for (z = 1; z < state.sizeZ(); ++z) {
                    // Check same-day intersections for z > 0
                    for (axis_size_t x1 = 0; x1 < state.sizeX() - 1; ++x1) {
                        if (!state.get(x1, y, z)) continue; // not assigned
                        for (axis_size_t x2 = x1 + 1; x2 < state.sizeX(); ++x2) {
                            if (!state.get(x2, y, z) || !m_IntersectingShiftsInSameDayMatrix.get(x1, x2))
                                continue; // not assigned or not intersecting
                            totalScore.violate(Violation::xyz(x1, y, z, {-1}));
                            totalScore.violate(Violation::xyz(x2, y, z, {-1}));
                        }
                    }

                    // Check previous and next day intersections
                    for (axis_size_t x1 = 0; x1 < state.sizeX(); ++x1) {
                        for (int32_t i = 0; i < m_MaxOffsetDays; ++i) {
                            const int32_t offsetDay = i + 1;
                            if (offsetDay > z) continue;
                            if (state.get(x1, y, z - offsetDay)) {
                                for (axis_size_t x2 = 0; x2 < state.sizeX(); ++x2) {
                                    if (!state.get(x2, y, z) || (!m_IntersectingShiftsInAdjacentDaysMatrices[i].
                                        get(x1, x2) && !m_IntersectingShiftsInAdjacentDaysMatrices[i].get(x2, x1)))
                                        continue; // not assigned or not intersecting
                                    totalScore.violate(Violation::xyz(x1, y, z, {-1}));
                                    totalScore.violate(Violation::xyz(x2, y, z, {-1}));
                                }
                            }
                        }
                    }
                }
            }

            return totalScore;
        }

    private:
        int32_t m_MaxOffsetDays;
        BitMatrix::BitSymmetricalMatrix m_IntersectingShiftsInSameDayMatrix;
        /**
         * Indexed by offset day (index 0 = offset 1 and -1 day).
         * The bit is set if x (in the previous day) intersects y (in the next day).<br>
         * In other words, x corresponds to the previous day and y corresponds to the next day.
         */
        std::vector<BitMatrix::BitSquareMatrix> m_IntersectingShiftsInAdjacentDaysMatrices {};
    };
}

#endif //RESTBETWEENSHIFTSCONSTRAINT_H
