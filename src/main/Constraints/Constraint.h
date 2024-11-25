#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <string>
#include <utility>

#include "State/State.h"
#include "Score/Score.h"

namespace Constraints {
    using axis_size_t = State::axis_size_t;
    using state_size_t = State::state_size_t;
    using score_t = Score::score_t;

    template<typename X, typename Y, typename Z, typename W>
    class Constraint {
    public:
        explicit Constraint(std::string name) : m_Name(std::move(name)) { }

        virtual ~Constraint() = default;

        virtual Score::Score evaluate(const State::State<X, Y, Z, W>& state) = 0;

        [[nodiscard]] const std::string& name() const { return m_Name; }

    private:
        std::string m_Name;
    };
}

#endif //CONSTRAINT_H
