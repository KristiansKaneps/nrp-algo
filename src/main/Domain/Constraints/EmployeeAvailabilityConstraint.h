#ifndef EMPLOYEEAVAILABILITYCONSTRAINT_H
#define EMPLOYEEAVAILABILITYCONSTRAINT_H

#include "Constraints/Constraint.h"

#include "Array/BitMatrix.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Constraints {
    class EmployeeAvailabilityConstraint final : public Constraint<
            Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill> {
    public:
        explicit EmployeeAvailabilityConstraint(const Time::Range& range, const std::chrono::time_zone *timeZone,
                                                const Axes::Axis<Domain::Shift>& xAxis,
                                                const Axes::Axis<Domain::Employee>& yAxis,
                                                const Axes::Axis<Domain::Day>& zAxis) :
            Constraint("EMPLOYEE_AVAILABILITY"),
            m_IntersectingEmployeeUnavailabilitiesAndShifts(
                BitMatrix::BitMatrix3D(xAxis.size(), yAxis.size(), zAxis.size())),
            m_IntersectingEmployeeDesiredAvailabilitiesAndShifts(
                BitMatrix::BitMatrix3D(xAxis.size(), yAxis.size(), zAxis.size())) {
            for (axis_size_t z = 0; z < zAxis.size(); ++z) {
                const auto& day = range.getDayAt(z, timeZone);

                for (axis_size_t y = 0; y < yAxis.size(); ++y) {
                    const auto& e = yAxis[y];

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
            }
        }

        ~EmployeeAvailabilityConstraint() override = default;

        [[nodiscard]] Score::Score evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            Score::Score totalScore {};
            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                score_t shiftUnavailabilityScore = 0;
                score_t shiftDesiredScore = 0;
                for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                    score_t dayUnavailabilityScore = 0;
                    score_t dayDesiredScore = 0;

                    for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                        dayUnavailabilityScore -= state.get(x, y, z) & m_IntersectingEmployeeUnavailabilitiesAndShifts.
                            get(x, y, z);
                        dayDesiredScore += state.get(x, y, z) & m_IntersectingEmployeeDesiredAvailabilitiesAndShifts.
                            get(x, y, z);
                    }

                    shiftUnavailabilityScore += dayUnavailabilityScore;
                    shiftDesiredScore += dayDesiredScore;
                }

                totalScore.strict += shiftUnavailabilityScore;
                totalScore.soft += shiftDesiredScore;
            }

            return totalScore;
        }

    private:
        BitMatrix::BitMatrix3D m_IntersectingEmployeeUnavailabilitiesAndShifts;
        BitMatrix::BitMatrix3D m_IntersectingEmployeeDesiredAvailabilitiesAndShifts;
    };
}

#endif //EMPLOYEEAVAILABILITYCONSTRAINT_H
