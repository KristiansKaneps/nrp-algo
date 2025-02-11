#ifndef REQUIREDSKILLCONSTRAINT_H
#define REQUIREDSKILLCONSTRAINT_H

#include "Constraints/Constraint.h"

#include "Array/BitMatrix.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Constraints {
    class RequiredSkillConstraint final : public Constraint<
            Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill> {
    public:
        explicit RequiredSkillConstraint(const Axes::Axis<Domain::Shift>& xAxis,
                                         const Axes::Axis<Domain::Employee>& yAxis,
                                         const Axes::Axis<Domain::Skill>& wAxis) : Constraint("REQUIRED_SKILL"),
            m_AssignableShiftEmployeeSkillMatrix(xAxis.size(), yAxis.size(), wAxis.size()),
            m_EvaluateCache(wAxis.size()) {
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

        [[nodiscard]] Score::Score evaluate(
            const State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>& state) override {
            score_t totalScore = 0;

            for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                    for (axis_size_t z = 0; z < state.sizeZ(); ++z) {
                        state.getLineXYZ(m_EvaluateCache, x, y, z);
                        const auto offsetZ = m_AssignableShiftEmployeeSkillMatrix.offsetZ(x, y);
                        m_AssignableShiftEmployeeSkillMatrix.validateZ(m_EvaluateCache, offsetZ);
                        totalScore -= static_cast<score_t>(m_EvaluateCache.count());
                    }
                }
            }

            return {.strict = totalScore, .hard = 0, .soft = 0};
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

                if (const auto employeeSkillWeight = employeeSkill->second; shiftSkillWeight > employeeSkillWeight)
                    return false;
            }

            if (shiftOneSkills.empty()) return true;

            for (const std::pair<axis_size_t, float> shiftOneSkill : shiftOneSkills) { // NOLINT(*-use-anyofallof)
                const auto skillIndex = shiftOneSkill.first;
                const auto shiftSkillWeight = shiftOneSkill.second;

                const auto employeeSkill = employeeSkills.find(skillIndex);
                if (employeeSkill == employeeSkills.cend()) continue;

                if (shiftSkillWeight <= employeeSkill->second)
                    return true;
            }

            return false;
        }

    private:
        BitMatrix::BitMatrix3D m_AssignableShiftEmployeeSkillMatrix;

        BitArray::BitArray m_EvaluateCache;
    };
}

#endif //REQUIREDSKILLCONSTRAINT_H
