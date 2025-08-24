#ifndef PERTURBATORCHAIN_H
#define PERTURBATORCHAIN_H

#include <vector>

#include "Perturbator.h"

#define MAX_PERTURBATOR_HISTORY_SIZE 500

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class PerturbatorChain {
    public:
        explicit PerturbatorChain() noexcept : m_Perturbators{} { }
        explicit PerturbatorChain(const std::vector<Perturbator<X, Y, Z, W> *>& perturbators) noexcept : m_Perturbators(perturbators) { }
        PerturbatorChain(const PerturbatorChain&) = delete;
        PerturbatorChain(PerturbatorChain&& other) noexcept {
            m_Perturbators = std::move(other.m_Perturbators);
            m_DeletePerturbatorsOnDestroy = other.m_DeletePerturbatorsOnDestroy;
            other.m_DeletePerturbatorsOnDestroy = false;
        }
        ~PerturbatorChain() noexcept {
            if (m_DeletePerturbatorsOnDestroy)
                for (auto *perturbator : m_Perturbators) { delete perturbator; }
        }

        [[nodiscard]] size_t size() const noexcept { return m_Perturbators.size(); }
        [[nodiscard]] bool empty() const noexcept { return m_Perturbators.empty(); }

        void modify(::State::State<X, Y, Z, W>& state) noexcept {
            for (auto *perturbator : m_Perturbators) perturbator->modify(state);
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept {
            for (auto it = m_Perturbators.rbegin(); it != m_Perturbators.rend(); ++it) (*it)->revert(state);
        }

        constexpr void operator()(::State::State<X, Y, Z, W>& state) noexcept { modify(state); }

        PerturbatorChain& operator=(const PerturbatorChain&) = delete;
        [[nodiscard]] PerturbatorChain& operator=(PerturbatorChain&& other) noexcept {
            if (this != &other) [[likely]] {
                if (m_DeletePerturbatorsOnDestroy)
                    for (auto *perturbator : m_Perturbators) { delete perturbator; }
                m_Perturbators = std::move(other.m_Perturbators);
                m_DeletePerturbatorsOnDestroy = other.m_DeletePerturbatorsOnDestroy;
                other.m_DeletePerturbatorsOnDestroy = false;
            }
            return *this;
        }

        void append(PerturbatorChain& other) noexcept {
            other.m_DeletePerturbatorsOnDestroy = false;
            for (auto *perturbator : other.m_Perturbators) push(perturbator);
        }

        void push(Perturbator<X, Y, Z, W> *p) noexcept {
            if (m_Perturbators.size() >= MAX_PERTURBATOR_HISTORY_SIZE) {
                delete *m_Perturbators.begin();
                m_Perturbators.erase(m_Perturbators.begin());
            }
            m_Perturbators.push_back(p);
        }

        std::vector<Perturbator<X, Y, Z, W> *> perturbators() const {
            return m_Perturbators;
        }

    protected:
        bool m_DeletePerturbatorsOnDestroy = true;
        std::vector<Perturbator<X, Y, Z, W> *> m_Perturbators;

        friend class HeuristicProvider<X, Y, Z, W>;
    };
}

#endif //PERTURBATORCHAIN_H
