#ifndef ASSIGNPERTURBATOR_H
#define ASSIGNPERTURBATOR_H

#include "Moves/Perturbator.h"

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class AssignPerturbator : public Perturbator<X, Y, Z, W> {
    public:
        explicit AssignPerturbator(::State::Location location) noexcept : m_Location(location) { }
        ~AssignPerturbator() override = default;

        void configure(const ::State::State<X, Y, Z, W>& state) noexcept {
            m_PrevValue = state.get(m_Location);
        }

        [[nodiscard]] bool isIdentity() const noexcept override {
            return m_PrevValue == 1;
        }

        void modify(::State::State<X, Y, Z, W>& state) noexcept override {
            state.set(m_Location);
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept override {
            state.assign(m_Location, m_PrevValue);
        }

    protected:
        ::State::Location m_Location;
        uint8_t m_PrevValue{};
    };
}

#endif //ASSIGNPERTURBATOR_H
