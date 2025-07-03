#ifndef RANDOMASSIGNMENTTOGGLEPERTURBATOR_H
#define RANDOMASSIGNMENTTOGGLEPERTURBATOR_H

#include "AutonomousPerturbator.h"

#include "Utils/Random.h"

namespace Moves {
    template<typename X, typename Y, typename Z, typename W>
    class RandomAssignmentTogglePerturbator final : public AutonomousPerturbator<X, Y, Z, W> {
    public:
        explicit RandomAssignmentTogglePerturbator() noexcept {
            // TODO: determine min Z width to assign at once (for now it is 2).
            //   There must always be a probability that only 1 z will be assigned (for later on in the search process).
            m_MaxZWidth = 2;
        }

        [[nodiscard]] RandomAssignmentTogglePerturbator *clone() const noexcept override {
            return new RandomAssignmentTogglePerturbator(*this);
        }

        void configure(const ::State::State<X, Y, Z, W>& state) noexcept override {
            m_Location = ::State::Location {
                m_Random.randomInt(0, state.sizeX() - 1),
                m_Random.randomInt(0, state.sizeY() - 1),
                m_Random.randomInt(0, state.sizeZ() - 1),
                m_Random.randomInt(0, state.sizeW() - 1)
            };
            m_ZSideIncrement = m_Random.randomInt(0, state.sizeZ() >= m_MaxZWidth ? m_MaxZWidth - 1 : state.sizeZ());
            if (m_Location.z + m_ZSideIncrement >= state.sizeZ()) {
                m_ZSideIncrement = 0;
            }
        }

        [[nodiscard]] bool isIdentity() const noexcept override { return false; }

        void modify(::State::State<X, Y, Z, W>& state) noexcept override {
            apply(state);
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept override {
            apply(state);
        }
    private:
        inline static Random::RandomGenerator& m_Random = Random::generator();
        axis_size_t m_MaxZWidth = 1;
        int32_t m_ZSideIncrement = 1;
        ::State::Location m_Location{};

        void apply(::State::State<X, Y, Z, W>& state) const noexcept {
            state.assign(m_Location, state.get(m_Location) ^ 1);
            for (int32_t i = 0; i < m_ZSideIncrement; ++i) {
                state.assign(m_Location.x, m_Location.y, m_Location.z + i, m_Location.w, state.get(m_Location.x, m_Location.y, m_Location.z + i, m_Location.w) ^ 1);
            }
        }
    };
}

#endif //RANDOMASSIGNMENTTOGGLEPERTURBATOR_H
