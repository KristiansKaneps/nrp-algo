#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <string>
#include <utility>

#include "ConstraintScore.h"
#include "State/State.h"

namespace Constraints {
    template<typename X, typename Y, typename Z, typename W>
    class Constraint {
    public:
        explicit Constraint(std::string name) : m_Name(std::move(name)) { }

        virtual ~Constraint() = default;

        virtual ConstraintScore evaluate(const ::State::State<X, Y, Z, W>& state) = 0;

        [[nodiscard]] const std::string& name() const { return m_Name; }

    private:
        std::string m_Name;
    };
}

#endif //CONSTRAINT_H
