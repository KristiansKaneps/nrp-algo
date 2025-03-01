#ifndef PERTURBATOR_H
#define PERTURBATOR_H

#include "State/State.h"

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class HeuristicProvider;

    /**
     * A perturbator is a low level heuristic (LLH) that modifies a candidate solution resulting in another candidate
     * solution.
     */
    template<typename X, typename Y, typename Z, typename W>
    class Perturbator {
    public:
        explicit Perturbator() = default;
        virtual ~Perturbator() = default;

        virtual void modify(State::State<X, Y, Z, W>& state) = 0;
        virtual void revert(State::State<X, Y, Z, W>& state) const = 0;
    protected:
        virtual void reset() { }
    private:
        friend class HeuristicProvider<X, Y, Z, W>;
    };

    template<typename X, typename Y, typename Z, typename W>
    class IdentityPerturbator final : public Perturbator<X, Y, Z, W> {
    public:
        explicit IdentityPerturbator() = default;

        void modify(State::State<X, Y, Z, W>& state) override {}
        void revert(State::State<X, Y, Z, W>& state) const override {}
    };
}

#endif //PERTURBATOR_H
