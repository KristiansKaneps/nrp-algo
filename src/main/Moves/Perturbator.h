#ifndef PERTURBATOR_H
#define PERTURBATOR_H

#include "State/State.h"

namespace Moves {
    using axis_size_t = ::State::axis_size_t;

    template<typename X, typename Y, typename Z, typename W>
    class HeuristicProvider;

    template<typename X, typename Y, typename Z, typename W>
    class PerturbatorChain;

    /**
     * A perturbator is a low-level heuristic (LLH) that modifies a candidate solution resulting in another candidate
     * solution.
     */
    template<typename X, typename Y, typename Z, typename W>
    class Perturbator {
    public:
        explicit Perturbator() noexcept = default;
        virtual ~Perturbator() noexcept = default;
        Perturbator(const Perturbator&) noexcept = default;
        Perturbator(Perturbator&&) noexcept = default;

        /**
         * Prepares this perturbator for modifying a given state.
         * @param state State to modify afterward.
         */
        virtual void configure(const ::State::State<X, Y, Z, W>& state) noexcept = 0;

        /**
         * Checks whether this perturbator (with current configuration) is effectively an identity perturbator.
         * @return `true` if this perturbator won't make any changes (with current configuration), `false` otherwise.
         */
        [[nodiscard]] virtual bool isIdentity() const noexcept { return false; }

        virtual void modify(::State::State<X, Y, Z, W>& state) noexcept = 0;
        virtual void revert(::State::State<X, Y, Z, W>& state) const noexcept = 0;

        constexpr void operator()(::State::State<X, Y, Z, W>& state) noexcept { modify(state); }

    protected:
        friend class PerturbatorChain<X, Y, Z, W>;
        friend class HeuristicProvider<X, Y, Z, W>;
    };
}

#endif //PERTURBATOR_H
