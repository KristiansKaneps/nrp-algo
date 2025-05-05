#ifndef PERTURBATORCHAIN_H
#define PERTURBATORCHAIN_H

#include <vector>

#include "Perturbator.h"

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class PerturbatorChain {
    public:
        explicit PerturbatorChain(const std::vector<Perturbator<X, Y, Z, W> *>& perturbators) : m_Perturbators(perturbators) { }
        PerturbatorChain(const PerturbatorChain&) = default;
        PerturbatorChain(PerturbatorChain&&) = default;
        ~PerturbatorChain() { for (auto *perturbator : m_Perturbators) { delete perturbator; } }

        void modify(::State::State<X, Y, Z, W>& state) {
            for (auto it = m_Perturbators.begin(); it != m_Perturbators.end(); ++it) {
                auto *perturbator = *it;
                perturbator->modify(state);
            }
        }

        void revert(::State::State<X, Y, Z, W>& state) const {
            for (auto it = m_Perturbators.rbegin(); it != m_Perturbators.rend(); ++it) {
                auto *perturbator = *it;
                perturbator->revert(state);
            }
        }

        constexpr void operator()(::State::State<X, Y, Z, W>& state) { modify(state); }

    protected:
        PerturbatorChain() = default;

        std::vector<Perturbator<X, Y, Z, W> *> m_Perturbators;

        friend class HeuristicProvider<X, Y, Z, W>;
    };
}

#endif //PERTURBATORCHAIN_H
