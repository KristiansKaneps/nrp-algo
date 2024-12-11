#ifndef LOCALSEARCHSTEP_H
#define LOCALSEARCHSTEP_H

#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Score/Score.h"
#include "Moves/Move.h"

namespace Search::Step {
    enum Type {
        DEFAULT,
    };

    template<typename X, typename Y, typename Z, typename W>
    class LocalSearchStep {
    public:
        LocalSearchStep(const LocalSearchStep& other) = delete;

        virtual ~LocalSearchStep() = default;

        [[nodiscard]] Type type() const { return m_Type; }

    protected:
        explicit LocalSearchStep(const Type type, State::State<X, Y, Z, W> *state) : m_Type(type),
                                                                    m_State(state) { }

    private:
        const Type m_Type;
        State::State<X, Y, Z, W>& m_State;
    };
}

#endif //LOCALSEARCHSTEP_H
