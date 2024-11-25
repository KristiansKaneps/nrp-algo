#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <unordered_map>

#include "State/Axes.h"

namespace Domain {
    using axis_size_t = Axes::axis_size_t;

    class Employee : public Axes::AxisEntity {
    public:
        explicit Employee(const axis_size_t index) : m_Index(index) { }

        ~Employee() override = default;

        [[nodiscard]] axis_size_t index() const { return m_Index; }
        const std::unordered_map<axis_size_t, float>& skills() const { return m_Skills; }

        bool hasSkill(const axis_size_t skillIndex) const { return m_Skills.contains(skillIndex); }

        float getSkillWeight(const axis_size_t skillIndex) const {
            const auto weight = m_Skills.find(skillIndex);
            if (weight == m_Skills.end()) return 0.0f;
            return weight->second;
        }

        void removeSkill(const axis_size_t skillIndex) { m_Skills.erase(skillIndex); }

        void setSkillWeight(const axis_size_t skillIndex, const float weight) { m_Skills[skillIndex] = weight; }

        void addSkill(const axis_size_t skillIndex, const float weight) { m_Skills[skillIndex] = weight; }

    private:
        const axis_size_t m_Index;
        std::unordered_map<axis_size_t, float> m_Skills {};
    };
}

#endif //EMPLOYEE_H
