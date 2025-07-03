#ifndef ADDCOVERSHIFTSPERTURBATOR_H
#define ADDCOVERSHIFTSPERTURBATOR_H

#include "DomainPerturbator.h"
#include "Domain/State/DomainState.h"

#include "Utils/Random.h"

namespace Domain::Moves {
    class AddCoverShiftsPerturbator final : public DomainAutonomousPerturbator {
    public:
        explicit AddCoverShiftsPerturbator() noexcept = default;

        [[nodiscard]] AddCoverShiftsPerturbator *clone() const noexcept override {
            return new AddCoverShiftsPerturbator(*this);
        }

        void configure(const Constraints::Violation *violation, const State::DomainState& state) noexcept override {
            m_X = m_Random.randomInt(0, state.sizeX() - 1);
            m_Y = m_Random.randomInt(0, state.sizeY() - 1);
            m_Z = m_Random.randomInt(0, state.sizeZ() - 1);
            m_W = m_Random.randomInt(0, state.sizeW() - 1);
            m_PrevValue = state.get(m_X, m_Y, m_Z, m_W);
        }

        bool isIdentity() const noexcept override { return false; }

        void modify(State::DomainState& state) noexcept override {
            state.assign(m_X, m_Y, m_Z, m_W, static_cast<uint8_t>(static_cast<uint8_t>(m_PrevValue ^ 1) & 1));
        }

        void revert(State::DomainState& state) const noexcept override {
            state.assign(m_X, m_Y, m_Z, m_W, m_PrevValue);
        }
    private:
        inline static Random::RandomGenerator& m_Random = Random::generator();
        uint8_t m_PrevValue{};
        axis_size_t m_X{}, m_Y{}, m_Z{}, m_W{};
    };
}

#endif //ADDCOVERSHIFTSPERTURBATOR_H
