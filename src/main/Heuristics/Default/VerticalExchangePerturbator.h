#ifndef VERTICALEXCHANGEPERTURBATOR_H
#define VERTICALEXCHANGEPERTURBATOR_H

#include "Heuristics/AutonomousPerturbator.h"
#include "Structs/VerticalExchangeAssignLocation.h"

#include "Utils/Random.h"

#include <unordered_map>

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class VerticalExchangePerturbator : public AutonomousPerturbator<X, Y, Z, W> {
    public:
        explicit VerticalExchangePerturbator() = default;
        ~VerticalExchangePerturbator() override = default;

        [[nodiscard]] VerticalExchangePerturbator *clone() const override {
            return new VerticalExchangePerturbator(*this);
        }

        void configure(const ::State::State<X, Y, Z, W>& state) override {
            if (state.sizeZ() < 2 || state.sizeY() < 1) return;

            const axis_size_t z1 = m_Random.randomInt(state.sizeZ() - 1);
            axis_size_t z2 = m_Random.randomInt(state.sizeZ() - 2);
            if (z2 >= z1) z2 += 1;

            const axis_size_t randomY = m_Random.randomInt(state.sizeY() - 1);

            bool startYInitialized = false;
            axis_size_t startY = 0, endY = 0;
            std::unordered_map<VerticalExchangeAssignLocation, uint8_t> locations1 {};
            locations1.reserve(5);
            for (axis_size_t yi = randomY; yi < state.sizeY() + randomY; ++yi) {
                const axis_size_t y = yi % state.sizeY();
                bool assignedAtY = false;
                for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                    for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                        if (state.get(x, y, z1, w)) {
                            assignedAtY = true;
                            if (!startYInitialized) {
                                startYInitialized = true;
                                startY = y;
                            }
                            locations1.emplace(VerticalExchangeAssignLocation{x, y, w}, 1);
                            break;
                        }
                    }
                }
                if (startYInitialized && !assignedAtY) {
                    endY = y;
                    break;
                }
            }
            std::unordered_map<VerticalExchangeAssignLocation, uint8_t> locations2 {};
            locations2.reserve(endY >= startY ? endY - startY : state.sizeZ() - startY + endY);
            for (axis_size_t y = startY; y < endY; ++y) {
                for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                    for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                        if (state.get(x, y, z2, w)) {
                            locations2.emplace(VerticalExchangeAssignLocation{x, y, w}, 1);
                            break;
                        }
                    }
                }
            }

            m_Z1 = z1;
            m_Z2 = z2;
            m_LocationXors = std::move(locations1);
            for (const auto& entry : locations2) {
                auto loc1 = m_LocationXors.find(entry.first);
                if (loc1 != m_LocationXors.end()) {
                    loc1->second ^= entry.second;
                } else {
                    m_LocationXors.emplace(entry.first, entry.second);
                }
            }
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
        inline static Random::RandomGenerator& m_Random = Random::generator();

        axis_size_t m_Z1 {}, m_Z2 {};
        std::unordered_map<VerticalExchangeAssignLocation, uint8_t> m_LocationXors {};

        void apply(::State::State<X, Y, Z, W>& state) const {
            for (const auto& entry : m_LocationXors) {
                const auto& [x, y, w] = entry.first;
                const uint8_t v1 = state.get(x, y, m_Z1, w) ^ entry.second;
                const uint8_t v2 = state.get(x, y, m_Z2, w) ^ entry.second;
                state.assign(x, y, m_Z1, w, v1);
                state.assign(x, y, m_Z2, w, v2);
            }
        }
    };
}

#endif //VERTICALEXCHANGEPERTURBATOR_H
