#ifndef SHIFT_H
#define SHIFT_H

#include <string>
#include <unordered_map>
#include <utility>

#include "Time/DailyInterval.h"

#include "State/Axes.h"

namespace Domain {
    using axis_size_t = Axes::axis_size_t;

    class Shift : public Axes::AxisEntity {
    public:
        Shift(const axis_size_t index, const Time::DailyInterval& interval, std::string name) : m_Index(index),
            m_Interval(interval),
            m_Name(std::move(name)) { }

        [[nodiscard]] axis_size_t index() const { return m_Index; }
        [[nodiscard]] const Time::DailyInterval& interval() const { return m_Interval; }
        [[nodiscard]] const std::string& name() const { return m_Name; }

        const std::unordered_map<axis_size_t, float>& requiredAllSkills() const { return m_RequiredAllSkills; }
        const std::unordered_map<axis_size_t, float>& requiredOneSkills() const { return m_RequiredOneSkills; }

        [[nodiscard]] bool requiresSkill() const {
            return !m_RequiredAllSkills.empty() || !m_RequiredOneSkills.empty();
        }

        [[nodiscard]] bool requiresSkill(const axis_size_t skillIndex) const {
            return m_RequiredAllSkills.contains(skillIndex) || (m_RequiredOneSkills.size() == 1 && m_RequiredOneSkills.
                contains(skillIndex));
        }

        void removeRequiredAllSkill(const axis_size_t skillIndex) { m_RequiredAllSkills.erase(skillIndex); }
        void removeRequiredOneSkill(const axis_size_t skillIndex) { m_RequiredOneSkills.erase(skillIndex); }

        void setRequiredAllSkillMinWeight(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredAllSkills[skillIndex] = minWeight;
        }

        void setRequiredOneSkillMinWeight(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredOneSkills[skillIndex] = minWeight;
        }

        void addRequiredAllSkill(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredAllSkills[skillIndex] = minWeight;
        }

        void addRequiredOneSkill(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredOneSkills[skillIndex] = minWeight;
        }

    private:
        const axis_size_t m_Index;
        const Time::DailyInterval m_Interval;
        const std::string m_Name;

        std::unordered_map<axis_size_t, float> m_RequiredAllSkills {};
        std::unordered_map<axis_size_t, float> m_RequiredOneSkills {};
    };
}

#endif //SHIFT_H
