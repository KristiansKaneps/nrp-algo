#ifndef REQUIREDSKILLCONSTRAINT_H
#define REQUIREDSKILLCONSTRAINT_H

#include "DomainConstraint.h"

#include "Array/BitMatrix.h"

namespace Domain::Constraints {
    class RequiredSkillConstraint final : public DomainConstraint {
    public:
        explicit RequiredSkillConstraint(const Axes::Axis<Domain::Shift>& xAxis,
                                         const Axes::Axis<Domain::Employee>& yAxis,
                                         const Axes::Axis<Domain::Skill>& wAxis) : Constraint("REQUIRED_SKILL", {
                new Moves::DomainUnassignRepairPerturbator(),
            }),
            m_AssignableShiftEmployeeSkillMatrix(xAxis.size(), yAxis.size(), wAxis.size()) {
            for (axis_size_t x = 0; x < xAxis.size(); ++x) {
                const auto& shift = xAxis[x];
                for (axis_size_t y = 0; y < yAxis.size(); ++y) {
                    const auto& employee = yAxis[y];
                    for (axis_size_t w = 0; w < wAxis.size(); ++w) {
                        if (!isAssignable(shift, employee, wAxis[w])) continue;
                        m_AssignableShiftEmployeeSkillMatrix.set(x, y, w);
                    }
                }
            }
        }

        ~RequiredSkillConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(
            const State::DomainState& state) override {
            ConstraintScore totalScore;

            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                    for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                        for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                            if (static_cast<int8_t>(m_AssignableShiftEmployeeSkillMatrix.get(x, y, w)) - static_cast<
                                int8_t>(state.get(x, y, z, w)) >= 0)
                                continue;
                            totalScore.violate(Violation::xyzw(x, y, z, w, {-static_cast<score_t>(1)}));
                        }
                    }
                }
            }

            return totalScore;
        }

    protected:
        static bool isAssignable(const Domain::Shift& shift, const Domain::Employee& employee,
                                 const Domain::Skill& skill) {
            if (!shift.requiresSkill()) [[unlikely]] return true;
            if (!employee.hasSkill(skill.index())) return false;

            const auto& shiftAllSkills = shift.requiredAllSkills();
            const auto& shiftOneSkills = shift.requiredOneSkills();

            const auto& employeeSkills = employee.skills();

            for (const std::pair<axis_size_t, float> shiftAllSkill : shiftAllSkills) {
                const auto skillIndex = shiftAllSkill.first;
                const auto shiftSkillWeight = shiftAllSkill.second;

                const auto employeeSkill = employeeSkills.find(skillIndex);
                if (employeeSkill == employeeSkills.cend()) return false;

                if (const auto employeeSkillWeight = employeeSkill->second.weight; shiftSkillWeight >
                    employeeSkillWeight)
                    return false;
            }

            if (shiftOneSkills.empty()) return true;

            for (const std::pair<axis_size_t, float> shiftOneSkill : shiftOneSkills) { // NOLINT(*-use-anyofallof)
                const auto skillIndex = shiftOneSkill.first;
                if (skillIndex != skill.index()) {
                    // Employee has a required 'one' skill, but we are checking for parameter `skill`.
                    continue;
                }

                const auto employeeSkill = employeeSkills.find(skillIndex);
                if (employeeSkill == employeeSkills.cend()) continue;

                const auto shiftSkillWeight = shiftOneSkill.second;

                // #if EXAMPLE != 2
                // if (shiftSkillWeight <= employeeSkill->second.weight)
                //     return true;
                // #else
                return true;
                // #endif
            }

            return false;
        }

    private:
        BitMatrix::BitMatrix3D m_AssignableShiftEmployeeSkillMatrix;
    };
}

#endif //REQUIREDSKILLCONSTRAINT_H
