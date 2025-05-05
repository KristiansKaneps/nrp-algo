#ifndef VALIDSHIFTDAYPERTURBATOR_H
#define VALIDSHIFTDAYPERTURBATOR_H

#include "DomainPerturbator.h"
#include "Domain/State/DomainState.h"

namespace Domain::Heuristics {
    class ValidShiftDayPerturbator final : public DomainPerturbator {
    public:
        explicit ValidShiftDayPerturbator(const axis_size_t yAxisSize, const axis_size_t wAxisSize) : m_PrevValue(BitArray::BitArray(yAxisSize * wAxisSize)) {}

        [[nodiscard]] ValidShiftDayPerturbator *clone() const override {
            return new ValidShiftDayPerturbator(*this);
        }

        void configure(const Constraints::Violation *violation, const State::DomainState& state) override {
            m_X = violation->getX();
            m_Z = violation->getZ();
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                    m_PrevValue.assign(y * state.sizeW() + w, state.get(m_X, y, m_Z, w));
                }
            }
        }

        void modify(State::DomainState& state) override {
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                    state.clear(m_X, y, m_Z, w);
                }
            }
        }

        void revert(State::DomainState& state) const override {
            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                    state.assign(m_X, y, m_Z, w, m_PrevValue.get(y * state.sizeW() + w));
                }
            }
        }
    private:
        BitArray::BitArray m_PrevValue;
        axis_size_t m_X{}, m_Z{};
    };
}

#endif //VALIDSHIFTDAYPERTURBATOR_H
