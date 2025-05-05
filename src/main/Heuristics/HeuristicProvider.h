#ifndef HEURISTICPROVIDER_H
#define HEURISTICPROVIDER_H

#include "Perturbator.h"
#include "PerturbatorChain.h"
#include "Search/Evaluation.h"
#include "State/State.h"

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class HeuristicProvider {
    public:
        explicit HeuristicProvider(const std::vector<Perturbator<X, Y, Z, W> *>&& perturbators) : m_Perturbators(
            std::move(perturbators)) { m_GeneratedPerturbators.shrink_to_fit(); }

        ~HeuristicProvider() = default;

        Perturbator<X, Y, Z, W> *operator[](const size_t index) const { return m_Perturbators[index]; }

        PerturbatorChain<X, Y, Z, W> generatePerturbators(const Evaluation::Evaluator<X, Y, Z, W>& evaluator,
                                                          const ::State::State<X, Y, Z, W>& state) {
            m_GeneratedPerturbators.clear();
            m_GeneratedPerturbators.reserve(evaluator.m_ViolatedConstraintCount);
            for (size_t i = 0; i < evaluator.m_ConstraintScores.size(); ++i) {
                const auto& constraint = evaluator.m_Constraints[i];
                const auto& constraintScore = evaluator.m_ConstraintScores[i];
                if (constraint->getRepairPerturbators().size() > 0) [[likely]] {
                    for (const auto& repairPerturbator : constraint->getRepairPerturbators()) {
                        for (const auto& violation : constraintScore.violations()) {
                            Perturbator<X, Y, Z, W> *perturb = repairPerturbator->clone();
                            perturb->configure(&violation, state);
                            m_GeneratedPerturbators.emplace_back(perturb);
                        }
                    }
                } else {
                    Perturbator<X, Y, Z, W> *perturb = m_Perturbators[i % m_Perturbators.size()]->clone();
                    perturb->configure(nullptr, state);
                    m_GeneratedPerturbators.emplace_back(perturb);
                }
            }
            // Perturbator<X, Y, Z, W> *perturb = m_Perturbators[0]->clone();
            // perturb->configure(nullptr, state);
            // m_GeneratedPerturbators.emplace_back(perturb);
            // std::cout << "Generated perturbator count: " << m_GeneratedPerturbators.size() << std::endl;
            return PerturbatorChain(m_GeneratedPerturbators);
        }

    private:
        const std::vector<Perturbator<X, Y, Z, W> *> m_Perturbators;
        std::vector<Perturbator<X, Y, Z, W> *> m_GeneratedPerturbators {};
    };
}

#endif //HEURISTICPROVIDER_H
