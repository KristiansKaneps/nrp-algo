#ifndef UNASSIGNPERTURBATOR_H
#define UNASSIGNPERTURBATOR_H

#include "Perturbator.h"

#include "State/Location.h"

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class UnassignPerturbator : public Perturbator<X, Y, Z, W> {
    public:
        explicit UnassignPerturbator(const ::State::Location& location) noexcept : m_Location(location) { }
        ~UnassignPerturbator() noexcept override = default;

        void configure(const ::State::State<X, Y, Z, W>& state) noexcept {
            m_PrevValue = state.get(m_Location);
        }

        [[nodiscard]] bool isIdentity() const noexcept override {
            return m_PrevValue == 0;
        }

        void modify(::State::State<X, Y, Z, W>& state) noexcept override {
            state.clear(m_Location);
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept override {
            state.assign(m_Location, m_PrevValue);
        }

    protected:
        ::State::Location m_Location;
        uint8_t m_PrevValue{};
    };
}

#endif //UNASSIGNPERTURBATOR_H
