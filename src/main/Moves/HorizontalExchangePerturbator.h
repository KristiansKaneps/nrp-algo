#ifndef HORIZONTALEXCHANGEPERTURBATOR_H
#define HORIZONTALEXCHANGEPERTURBATOR_H

#include "AutonomousPerturbator.h"
#include "Structs/HorizontalExchangeAssignLocation.h"

#include "Utils/Random.h"

#include <unordered_map>

namespace Moves {
    using namespace Structs;

    template<typename X, typename Y, typename Z, typename W>
    class HorizontalExchangePerturbator : public AutonomousPerturbator<X, Y, Z, W> {
    public:
        explicit HorizontalExchangePerturbator() noexcept = default;
        ~HorizontalExchangePerturbator() noexcept override = default;

        [[nodiscard]] HorizontalExchangePerturbator *clone() const noexcept override {
            return new HorizontalExchangePerturbator(*this);
        }

        void configure(const ::State::State<X, Y, Z, W>& state) noexcept override {
            if (state.sizeY() < 2 || state.sizeZ() < 1) return;

            const axis_size_t y1 = m_Random.randomInt(state.sizeY() - 1);
            axis_size_t y2 = m_Random.randomInt(state.sizeY() - 2);
            if (y2 >= y1) y2 += 1;

            const axis_size_t randomZ = m_Random.randomInt(state.sizeZ() - 1);

            bool startZInitialized = false;
            axis_size_t startZ = 0, endZ = 0;
            std::unordered_map<HorizontalExchangeAssignLocation, uint8_t> locations1 {};
            locations1.reserve(5);
            for (axis_size_t zi = randomZ; zi < state.sizeZ() + randomZ; ++zi) {
                const axis_size_t z = zi % state.sizeZ();
                bool assignedAtZ = false;
                for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                    for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                        if (state.get(x, y1, z, w)) {
                            assignedAtZ = true;
                            if (!startZInitialized) {
                                startZInitialized = true;
                                startZ = z;
                            }
                            locations1.emplace(HorizontalExchangeAssignLocation{x, z, w}, 1);
                            break;
                        }
                    }
                }
                if (startZInitialized && !assignedAtZ) {
                    endZ = z;
                    break;
                }
            }
            std::unordered_map<HorizontalExchangeAssignLocation, uint8_t> locations2 {};
            locations2.reserve(endZ >= startZ ? endZ - startZ : state.sizeZ() - startZ + endZ);
            for (axis_size_t z = startZ; z < endZ; ++z) {
                for (axis_size_t x = 0; x < state.sizeX(); ++x) {
                    for (axis_size_t w = 0; w < state.sizeW(); ++w) {
                        if (state.get(x, y2, z, w)) {
                            locations2.emplace(HorizontalExchangeAssignLocation{x, z, w}, 1);
                            break;
                        }
                    }
                }
            }

            m_Y1 = y1;
            m_Y2 = y2;
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

        [[nodiscard]] bool isIdentity() const noexcept override {
            return m_Y1 == m_Y2 || m_LocationXors.empty();
        }

        void modify(::State::State<X, Y, Z, W>& state) noexcept override {
            apply(state);
        }

        void revert(::State::State<X, Y, Z, W>& state) const noexcept override {
            apply(state);
        }

    protected:
        inline static Random::RandomGenerator& m_Random = Random::generator();

        axis_size_t m_Y1 {}, m_Y2 {};
        std::unordered_map<HorizontalExchangeAssignLocation, uint8_t> m_LocationXors {};

        void apply(::State::State<X, Y, Z, W>& state) const noexcept {
            for (const auto& entry : m_LocationXors) {
                const auto& [x, z, w] = entry.first;
                const uint8_t v1 = state.get(x, m_Y1, z, w) ^ entry.second;
                const uint8_t v2 = state.get(x, m_Y2, z, w) ^ entry.second;
                state.assign(x, m_Y1, z, w, v1);
                state.assign(x, m_Y2, z, w, v2);
            }
        }
    };
}

#endif //HORIZONTALEXCHANGEPERTURBATOR_H
