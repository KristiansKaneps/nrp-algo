#ifndef UNASSIGNREPAIRPERTURBATOR_H
#define UNASSIGNREPAIRPERTURBATOR_H

#include "Heuristics/Perturbator.h"

#include <vector>

namespace Heuristics {
    template<typename X, typename Y, typename Z, typename W>
    class UnassignRepairPerturbator : public Perturbator<X, Y, Z, W> {
    public:
        explicit UnassignRepairPerturbator() = default;
        ~UnassignRepairPerturbator() override = default;

        [[nodiscard]] UnassignRepairPerturbator *clone() const override {
            return new UnassignRepairPerturbator(*this);
        }

        void configure(const Constraints::Violation *violation, const ::State::State<X, Y, Z, W>& state) override {
            if (violation == nullptr || violation->info != 0) return;
            const axis_size_t maxXi = violation->hasX() ? 1 : state.sizeX();
            const axis_size_t maxYi = violation->hasY() ? 1 : state.sizeY();
            const axis_size_t maxZi = violation->hasZ() ? 1 : state.sizeZ();
            const axis_size_t maxWi = violation->hasW() ? 1 : state.sizeW();
            m_Locations.reserve(maxXi * maxYi * maxZi * maxWi);
            for (axis_size_t xi = 0; xi < maxXi; ++xi) {
                const axis_size_t x = xi + violation->getX();
                for (axis_size_t yi = 0; yi < maxYi; ++yi) {
                    const axis_size_t y = yi + violation->getY();
                    for (axis_size_t zi = 0; zi < maxZi; ++zi) {
                        const axis_size_t z = zi + violation->getZ();
                        for (axis_size_t wi = 0; wi < maxWi; ++wi) {
                            const axis_size_t w = wi + violation->getW();
                            if (state.get(x, y, z, w)) m_Locations.emplace_back(::State::Location{x, y, z, w});
                        }
                    }
                }
            }
        }

        bool isIdentity() const override {
            return m_Locations.size() == 0;
        }

        void modify(::State::State<X, Y, Z, W>& state) override {
            for (const auto& [x, y, z, w] : m_Locations) {
                state.clear(x, y, z, w);
            }
        }

        void revert(::State::State<X, Y, Z, W>& state) const override {
            for (const auto& [x, y, z, w] : m_Locations) {
                state.set(x, y, z, w);
            }
        }

    protected:
        std::vector<::State::Location> m_Locations {};
    };
}

#endif //UNASSIGNREPAIRPERTURBATOR_H
