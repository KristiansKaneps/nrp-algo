#ifndef EVALUATION_H
#define EVALUATION_H

#include "Constraints/Constraint.h"
#include "Score/Score.h"
#include "State/State.h"

namespace Evaluation {
    template<typename X, typename Y, typename Z, typename W>
    // ReSharper disable once CppRedundantQualifier
    Score::Score evaluateState(const ::State::State<X, Y, Z, W>& state, const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints) {
        Score::Score score {};
        // ReSharper disable once CppRedundantQualifier
        for (::Constraints::Constraint<X, Y, Z, W> *constraint : constraints)
            score += constraint->evaluate(state);
        return score;
    }
}

#endif //EVALUATION_H
