#ifndef PERTURBATORCHAIN_H
#define PERTURBATORCHAIN_H

#include <vector>

#include "Perturbator.h"

#define MAX_PERTURBATOR_HISTORY_SIZE 500

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class PerturbatorChain {
    public:
        explicit PerturbatorChain() noexcept : m_Perturbators{}, m_MaxHistory(MAX_PERTURBATOR_HISTORY_SIZE), m_Start(0), m_Size(0) {
            m_Perturbators.resize(MAX_PERTURBATOR_HISTORY_SIZE);
        }
        explicit PerturbatorChain(const std::vector<Perturbator<X, Y, Z, W> *>& perturbators) noexcept : m_Perturbators(perturbators), m_MaxHistory(perturbators.size()), m_Start(0), m_Size(0) { }
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
            for (auto *perturbator : other.m_Perturbators) push(perturbator);
        }

        void push(Perturbator<X, Y, Z, W> *p) noexcept {
            std::size_t pos = (m_Start + m_Size) % m_MaxHistory;
            m_Perturbators[pos] = p;
            if (m_Size < m_MaxHistory) {
                m_Size++;
            } else {
                m_Start = (m_Start + 1) % m_MaxHistory;
            }
        }

        std::vector<Perturbator<X, Y, Z, W> *> perturbators() const {
            std::vector<Perturbator<X, Y, Z, W> *> history;
            history.reserve(m_Size);
            for (std::size_t i = 0; i < m_Size; ++i) {
                history.push_back(m_Perturbators[(m_Start + i) % m_MaxHistory]);
            }
            return history;
        }

    protected:
        bool m_DeletePerturbatorsOnDestroy = true;
        std::vector<Perturbator<X, Y, Z, W> *> m_Perturbators;
        size_t m_MaxHistory;
        size_t m_Start;
        size_t m_Size;

        friend class HeuristicProvider<X, Y, Z, W>;
    };
}

#endif //PERTURBATORCHAIN_H
