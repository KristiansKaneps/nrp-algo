#ifndef RANDOMASSIGNMENTTOGGLEPERTURBATOR_H
#define RANDOMASSIGNMENTTOGGLEPERTURBATOR_H

#include "AutonomousPerturbator.h"

#include "Utils/Random.h"

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class RandomAssignmentTogglePerturbator final : public AutonomousPerturbator<X, Y, Z, W> {
    public:
        explicit RandomAssignmentTogglePerturbator() = default;

        [[nodiscard]] RandomAssignmentTogglePerturbator *clone() const override {
            return new RandomAssignmentTogglePerturbator(*this);
        }

        void configure(const ::State::State<X, Y, Z, W>& state) override {
            m_Location = ::State::Location {
                m_Random.randomInt(0, state.sizeX() - 1),
                m_Random.randomInt(0, state.sizeY() - 1),
                m_Random.randomInt(0, state.sizeZ() - 1),
                m_Random.randomInt(0, state.sizeW() - 1)
            };
            m_PrevValue = state.get(m_Location);
        }

        [[nodiscard]] bool isIdentity() const override { return false; }

        void modify(::State::State<X, Y, Z, W>& state) override {
            state.assign(m_Location, static_cast<uint8_t>(m_PrevValue ^ 1));
        }

        void revert(::State::State<X, Y, Z, W>& state) const override {
            state.assign(m_Location, m_PrevValue);
        }
    private:
        inline static Random::RandomGenerator& m_Random = Random::generator();
        ::State::Location m_Location{};
        uint8_t m_PrevValue{};
    };
}

#endif //RANDOMASSIGNMENTTOGGLEPERTURBATOR_H
