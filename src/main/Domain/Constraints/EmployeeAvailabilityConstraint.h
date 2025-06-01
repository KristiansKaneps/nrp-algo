#ifndef EMPLOYEEAVAILABILITYCONSTRAINT_H
#define EMPLOYEEAVAILABILITYCONSTRAINT_H

#include "DomainConstraint.h"

#include "Array/BitMatrix.h"

namespace Domain::Constraints {
    class EmployeeAvailabilityConstraint final : public DomainConstraint {
    public:
        explicit EmployeeAvailabilityConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                                const Axes::Axis<Domain::Shift>& xAxis,
                                                const Axes::Axis<Domain::Employee>& yAxis,
                                                const Axes::Axis<Domain::Day>& zAxis) :
            Constraint("EMPLOYEE_AVAILABILITY", {
            }),
            m_IntersectingEmployeeUnavailabilitiesAndShifts(
                BitMatrix::BitMatrix3D(xAxis.size(), yAxis.size(), zAxis.size())),
            m_IntersectingEmployeeDesiredAvailabilitiesAndShifts(
                BitMatrix::BitMatrix3D(xAxis.size(), yAxis.size(), zAxis.size())),
            m_SpecificRequests(std::vector<std::vector<std::vector<int8_t>>>(xAxis.size(), std::vector<std::vector<int8_t>>(yAxis.size(), std::vector<int8_t>(zAxis.size())))) {
            for (axis_size_t y = 0; y < yAxis.size(); ++y) {
                const auto& e = yAxis[y];

                for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                    const auto& day = range.getDayAt(z, timeZone);

                    for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                        const auto& s = xAxis[x];
                        const auto& interval = s.interval();
                        const auto& shiftRange = interval.toRange(day);

                        if (e.paidUnavailableAvailability().m_RangeCollection.intersects(shiftRange)
                            || e.unpaidUnavailableAvailability().m_RangeCollection.intersects(shiftRange)) {
                            m_IntersectingEmployeeUnavailabilitiesAndShifts.set(x, y, z);
                        } else if (e.desiredAvailability().m_RangeCollection.intersects(shiftRange)) {
                            m_IntersectingEmployeeDesiredAvailabilitiesAndShifts.set(x, y, z);
                        }
                    }
                }

                for (const auto& specificRequest : e.desiredAvailability().m_SpecificRequests) {
                    m_SpecificRequests[specificRequest.shiftIndex][y][specificRequest.dayIndex] = specificRequest.weight;
                }
                for (const auto& specificRequest : e.paidUnavailableAvailability().m_SpecificRequests) {
                    m_SpecificRequests[specificRequest.shiftIndex][y][specificRequest.dayIndex] = -specificRequest.weight; // NOLINT(*-narrowing-conversions)
                }
                for (const auto& specificRequest : e.unpaidUnavailableAvailability().m_SpecificRequests) {
                    m_SpecificRequests[specificRequest.shiftIndex][y][specificRequest.dayIndex] = -specificRequest.weight; // NOLINT(*-narrowing-conversions)
                }
            }
        }

        ~EmployeeAvailabilityConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::DomainState& state) override {
            ConstraintScore totalScore;
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                        if (!state.get(x, y, z)) continue;
                        if (m_IntersectingEmployeeUnavailabilitiesAndShifts.get(x, y, z)) {
                            totalScore.violate(Violation::xyz(x, y, z, {-1}));
                        }
                        if (m_IntersectingEmployeeDesiredAvailabilitiesAndShifts.get(x, y, z)) {
                            totalScore.addSoftScore(1);
                        }
                        if (const int8_t specificRequestWeight = m_SpecificRequests[x][y][z]; specificRequestWeight < 0) {
                            totalScore.violate(Violation::xyz(x, y, z, {0, 0, static_cast<score_t>(specificRequestWeight)}));
                        } else if (specificRequestWeight > 0) {
                            totalScore.addSoftScore(static_cast<score_t>(specificRequestWeight));
                        }
                    }
                }
            }

            return totalScore;
        }

    private:
        BitMatrix::BitMatrix3D m_IntersectingEmployeeUnavailabilitiesAndShifts;
        BitMatrix::BitMatrix3D m_IntersectingEmployeeDesiredAvailabilitiesAndShifts;
        std::vector<std::vector<std::vector<int8_t>>> m_SpecificRequests;
    };
}

#endif //EMPLOYEEAVAILABILITYCONSTRAINT_H
