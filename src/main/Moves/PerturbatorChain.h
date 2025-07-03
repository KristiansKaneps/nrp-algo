#ifndef PERTURBATORCHAIN_H
#define PERTURBATORCHAIN_H

#include <vector>

#include "Perturbator.h"

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class PerturbatorChain {
    public:
        explicit PerturbatorChain() noexcept : m_Perturbators{} {}
        explicit PerturbatorChain(const std::vector<Perturbator<X, Y, Z, W> *>& perturbators) noexcept : m_Perturbators(perturbators) { }
        PerturbatorChain(const PerturbatorChain&) noexcept = default;
        PerturbatorChain(PerturbatorChain&&) noexcept = default;
        ~PerturbatorChain() noexcept {
            if (m_DeletePerturbatorsOnDestroy)
                for (auto *perturbator : m_Perturbators) { delete perturbator; }
        }

        [[nodiscard]] size_t size() const noexcept { return m_Perturbators.size(); }
        [[nodiscard]] bool empty() const noexcept { return m_Perturbators.empty(); }

        void modify(::State::State<X, Y, Z, W>& state) noexcept {
            for (auto it = m_Perturbators.begin(); it != m_Perturbators.end(); ++it) {
                auto *perturbator = *it;
                perturbator->modify(state);
            }
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept {
            for (auto it = m_Perturbators.rbegin(); it != m_Perturbators.rend(); ++it) {
                auto *perturbator = *it;
                perturbator->revert(state);
            }
        }

        constexpr void operator()(::State::State<X, Y, Z, W>& state) noexcept { modify(state); }

        [[nodiscard]] PerturbatorChain& operator=(PerturbatorChain&& other) noexcept {
            if (this != &other) [[likely]] {
                for (auto *perturbator : m_Perturbators) { delete perturbator; }
                m_Perturbators = std::move(other.m_Perturbators);
            }
            return *this;
        }

        void append(PerturbatorChain& other) noexcept {
            other.m_DeletePerturbatorsOnDestroy = false;
            m_Perturbators.reserve(m_Perturbators.size() + other.m_Perturbators.size());
            for (auto *perturbator : other.m_Perturbators) m_Perturbators.emplace_back(perturbator);
        }

    protected:
        bool m_DeletePerturbatorsOnDestroy = true;
        std::vector<Perturbator<X, Y, Z, W> *> m_Perturbators;

        friend class HeuristicProvider<X, Y, Z, W>;
    };
}

#endif //PERTURBATORCHAIN_H
