#ifndef PERTURBATOR_H
#define PERTURBATOR_H

#include "State/State.h"
#include "Constraints/Violation.h"

namespace Heuristics {
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
        explicit Perturbator() = default;
        virtual ~Perturbator() = default;
        Perturbator(const Perturbator&) = default;
        Perturbator(Perturbator&&) = default;

        /**
         * Clone should be deleted afterward.
         * @return Cloned perturbator.
         */
        [[nodiscard]] virtual Perturbator *clone() const = 0;

        /**
         * Prepares this perturbator for modifying a given state.
         * @param violation Violation to repair.
         * @param state State to modify afterward.
         */
        virtual void configure(const Constraints::Violation *violation, const ::State::State<X, Y, Z, W>& state) = 0;

        virtual void modify(::State::State<X, Y, Z, W>& state) = 0;
        virtual void revert(::State::State<X, Y, Z, W>& state) const = 0;

        constexpr void operator()(::State::State<X, Y, Z, W>& state) { modify(state); }

    protected:
        friend class PerturbatorChain<X, Y, Z, W>;
        friend class HeuristicProvider<X, Y, Z, W>;
    };

    template<typename X, typename Y, typename Z, typename W>
    class IdentityPerturbator final : public Perturbator<X, Y, Z, W> {
    public:
        static IdentityPerturbator& instance() noexcept {
            static IdentityPerturbator instance;
            return instance;
        }

        IdentityPerturbator(const IdentityPerturbator&) = delete;
        IdentityPerturbator& operator=(const IdentityPerturbator&) = delete;

        [[nodiscard]] IdentityPerturbator *clone() const noexcept override {
            return const_cast<IdentityPerturbator *>(this);
        }

        void *operator new(const std::size_t) noexcept { return &instance(); }

        void *operator new[](const std::size_t) noexcept { return &instance(); }

        void operator delete(void *) noexcept { /* no-op */
        }

        void operator delete[](void *) noexcept { /* no-op */
        }

        void configure(const Constraints::Violation *, const ::State::State<X, Y, Z, W>&) noexcept override { }

        void modify(::State::State<X, Y, Z, W>&) noexcept override { }
        void revert(::State::State<X, Y, Z, W>&) const noexcept override { }

    private:
        IdentityPerturbator() = default;
        ~IdentityPerturbator() override = default;
    };
}

#endif //PERTURBATOR_H
