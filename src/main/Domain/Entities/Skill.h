#ifndef SKILL_H
#define SKILL_H

#include <string>

#include "State/Axes.h"

namespace Domain {
    using axis_size_t = Axes::axis_size_t;

    class Skill : public Axes::AxisEntity {
    public:
        explicit Skill(const axis_size_t index, const std::string& name) : m_Index(index),
                                                                    m_Name(name) { }

        [[nodiscard]] axis_size_t index() const { return m_Index; }
        [[nodiscard]] const std::string& name() const { return m_Name; }

    private:
        const axis_size_t m_Index;
        const std::string m_Name;
    };
}

#endif //SKILL_H
