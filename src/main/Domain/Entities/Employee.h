#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <string>
#include <unordered_map>

#include "State/Axes.h"

namespace Domain {
    using axis_size_t = Axes::axis_size_t;

    class Employee : public Axes::AxisEntity {
    public:
        explicit Employee(const axis_size_t index) : m_Index(index), m_Name(std::to_string(index + 1)) { }
        Employee(const axis_size_t index, const std::string& name) : m_Index(index), m_Name(name) { }

        ~Employee() override = default;

        [[nodiscard]] axis_size_t index() const { return m_Index; }
        [[nodiscard]] std::string name() const { return m_Name; }
        [[nodiscard]] const std::unordered_map<axis_size_t, float>& skills() const { return m_Skills; }

        [[nodiscard]] bool hasSkill(const axis_size_t skillIndex) const { return m_Skills.contains(skillIndex); }

        [[nodiscard]] float getSkillWeight(const axis_size_t skillIndex) const {
            const auto weight = m_Skills.find(skillIndex);
            if (weight == m_Skills.end()) return 0.0f;
            return weight->second;
        }

        void removeSkill(const axis_size_t skillIndex) { m_Skills.erase(skillIndex); }

        void setSkillWeight(const axis_size_t skillIndex, const float weight) { m_Skills[skillIndex] = weight; }

        void addSkill(const axis_size_t skillIndex, const float weight) { m_Skills[skillIndex] = weight; }

    private:
        const axis_size_t m_Index;
        const std::string m_Name;
        std::unordered_map<axis_size_t, float> m_Skills {};
    };
}

#endif //EMPLOYEE_H
