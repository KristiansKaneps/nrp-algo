#ifndef HEURISTICPROVIDER_H
#define HEURISTICPROVIDER_H

#include "Perturbator.h"

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class HeuristicProvider {
    public:
        explicit HeuristicProvider(const std::vector<Perturbator<X, Y, Z, W> *>&& perturbators) : m_Perturbators(
            std::move(perturbators)) { }

        ~HeuristicProvider() = default;

        Perturbator<X, Y, Z, W> * operator[](const size_t index) const {
            auto *perturbator = m_Perturbators[index];
            perturbator->reset();
            return perturbator;
        }

    private:
        const std::vector<Perturbator<X, Y, Z, W> *> m_Perturbators;
    };
}

#endif //HEURISTICPROVIDER_H
