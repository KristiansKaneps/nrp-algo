#ifndef AUTONOMOUSPERTURBATOR_H
#define AUTONOMOUSPERTURBATOR_H

#include "Perturbator.h"

#include "State/State.h"
#include "Constraints/Violation.h"

namespace Evaluation {
    template<typename X, typename Y, typename Z, typename W>
    class Evaluator;
}

namespace Moves {
    /**
     * A perturbator is a low-level heuristic (LLH) that modifies a candidate solution resulting in another candidate
     * solution.
     */
    template<typename X, typename Y, typename Z, typename W>
    class AutonomousPerturbator : public Perturbator<X, Y, Z, W> {
    public:
        explicit AutonomousPerturbator() = default;
        virtual ~AutonomousPerturbator() = default;
        AutonomousPerturbator(const AutonomousPerturbator&) = default;
        AutonomousPerturbator(AutonomousPerturbator&&) = default;

        /**
         * Clone should be deleted afterward.
         * @return Cloned perturbator.
         */
        [[nodiscard]] virtual AutonomousPerturbator *clone() const = 0;

        /**
         * Check if this perturbator is applicable for current evaluated candidate state and configure it.
         * @param evaluator Evaluator
         * @param state Current candidate state
         * @return `true` if configuration is done; `false` otherwise
         */
        [[nodiscard]] virtual bool configureIfApplicable(const ::Evaluation::Evaluator<X, Y, Z, W>& evaluator, const ::State::State<X, Y, Z, W>& state) {
            return false;
        }

        /**
         * Prepares this perturbator for modifying a given state.
         * @param violation Violation to repair.
         * @param state State to modify afterward.
         * @see configureIfApplicable
         */
        virtual void configure(const Constraints::Violation *violation, const ::State::State<X, Y, Z, W>& state) {
            configure(state);
        }

        /**
         * Prepares this perturbator for modifying a given state.
         * @param state State to modify afterward.
         */
        void configure(const ::State::State<X, Y, Z, W>& state) override { }

    protected:
        friend class PerturbatorChain<X, Y, Z, W>;
        friend class HeuristicProvider<X, Y, Z, W>;
    };

    template<typename X, typename Y, typename Z, typename W>
    class IdentityPerturbator final : public AutonomousPerturbator<X, Y, Z, W> {
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

        bool isIdentity() const override { return true; }

        void modify(::State::State<X, Y, Z, W>&) noexcept override { }
        void revert(::State::State<X, Y, Z, W>&) const noexcept override { }

    private:
        IdentityPerturbator() = default;
        ~IdentityPerturbator() override = default;
    };
}

#endif //AUTONOMOUSPERTURBATOR_H
