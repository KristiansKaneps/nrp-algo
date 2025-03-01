#ifndef RANDOMASSIGNMENTTOGGLEPERTURBATOR_H
#define RANDOMASSIGNMENTTOGGLEPERTURBATOR_H

#include "DomainPerturbator.h"
#include "Domain/State/DomainState.h"

#include "Utils/Random.h"

namespace Domain::Heuristics {
    class RandomAssignmentTogglePerturbator final : public DomainPerturbator {
    public:
        explicit RandomAssignmentTogglePerturbator() = default;

        void modify(State::DomainState& state) override {
            m_X = m_Random.randomInt(0, state.sizeX() - 1);
            m_Y = m_Random.randomInt(0, state.sizeY() - 1);
            m_Z = m_Random.randomInt(0, state.sizeZ() - 1);
            m_W = m_Random.randomInt(0, state.sizeW() - 1);
            m_PrevValue = state.get(m_X, m_Y, m_Z, m_W);
            state.assign(m_X, m_Y, m_Z, m_W, static_cast<uint8_t>(static_cast<uint8_t>(m_PrevValue ^ 1) & 1));
        }

        void revert(State::DomainState& state) const override {
            state.assign(m_X, m_Y, m_Z, m_W, m_PrevValue);
        }
    private:
        inline static Random::RandomGenerator m_Random {};
        uint8_t m_PrevValue{};
        axis_size_t m_X{}, m_Y{}, m_Z{}, m_W{};
    };
}

#endif //RANDOMASSIGNMENTTOGGLEPERTURBATOR_H
