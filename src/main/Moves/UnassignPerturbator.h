#ifndef UNASSIGNPERTURBATOR_H
#define UNASSIGNPERTURBATOR_H

#include "Perturbator.h"

#include "State/Location.h"

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class UnassignPerturbator : public Perturbator<X, Y, Z, W> {
    public:
        explicit UnassignPerturbator(const ::State::Location& location) : m_Location(location) { }
        ~UnassignPerturbator() override = default;

        void configure(const ::State::State<X, Y, Z, W>& state) {
            m_PrevValue = state.get(m_Location);
        }

        bool isIdentity() const override {
            return m_PrevValue == 0;
        }

        void modify(::State::State<X, Y, Z, W>& state) override {
            state.clear(m_Location);
        }

        void revert(::State::State<X, Y, Z, W>& state) const override {
            state.assign(m_Location, m_PrevValue);
        }

    protected:
        ::State::Location m_Location;
        uint8_t m_PrevValue{};
    };
}

#endif //UNASSIGNPERTURBATOR_H
