#ifndef VERTICALEXCHANGEPERTURBATOR_H
#define VERTICALEXCHANGEPERTURBATOR_H

#include "Heuristics/Perturbator.h"
#include "Structs/HorizontalExchangeAssignLocation.h"

#include "Utils/Random.h"

#include <unordered_map>

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class VerticalExchangePerturbator : public Perturbator<X, Y, Z, W> {
    public:
        explicit VerticalExchangePerturbator() = default;
        ~VerticalExchangePerturbator() override = default;

        [[nodiscard]] VerticalExchangePerturbator *clone() const override {
            return new VerticalExchangePerturbator(*this);
        }

        void configure(const Constraints::Violation *violation, const ::State::State<X, Y, Z, W>& state) override {
            if (state.sizeZ() < 2 || state.sizeY() < 1) return;

            const axis_size_t z1 = m_Random.randomInt(state.sizeY() - 1);
            axis_size_t z2 = m_Random.randomInt(state.sizeY() - 2);
            if (z2 >= z1) z2 += 1;

            const axis_size_t randomY = m_Random.randomInt(state.sizeY() - 1);

            // TODO
        }

        [[nodiscard]] bool isIdentity() const override {
            return m_Z1 == m_Z2 || m_LocationXors.empty();
        }

        void modify(::State::State<X, Y, Z, W>& state) override {
            apply(state);
        }

        void revert(::State::State<X, Y, Z, W>& state) const override {
            apply(state);
        }

    protected:
        inline static thread_local Random::RandomGenerator m_Random {};

        axis_size_t m_Z1 {}, m_Z2 {};
        std::unordered_map<HorizontalExchangeAssignLocation, uint8_t> m_LocationXors {};

        void apply(::State::State<X, Y, Z, W>& state) const {
            for (const auto& entry : m_LocationXors) {
                const auto& [x, z, w] = entry.first;
                const uint8_t v1 = state.get(x, m_Z1, z, w) ^ entry.second;
                const uint8_t v2 = state.get(x, m_Z2, z, w) ^ entry.second;
                state.assign(x, m_Z1, z, w, v1);
                state.assign(x, m_Z2, z, w, v2);
            }
        }
    };
}

#endif //VERTICALEXCHANGEPERTURBATOR_H
