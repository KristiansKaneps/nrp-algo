#ifndef CUMULATIVEFATIGUECONSTRAINT_H
#define CUMULATIVEFATIGUECONSTRAINT_H

#ifndef NDEBUG
// #define CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
#endif

#include "DomainConstraint.h"

namespace Domain::Constraints {
    class CumulativeFatigueConstraint final : public DomainConstraint {
    public:
        explicit CumulativeFatigueConstraint(const int32_t maxCumulativeMinutes,
                                             const Axes::Axis<Domain::Shift>& xAxis) :
            Constraint("CUMULATIVE_FATIGUE_CONSTRAINT", {
                new Heuristics::DomainUnassignRepairPerturbator(),
            }),
            m_MaxCumulativeMinutes(maxCumulativeMinutes) {
            m_SortedShiftIndices.reserve(xAxis.size());
            for (axis_size_t i = 0; i < xAxis.size(); ++i)
                m_SortedShiftIndices.push_back(i);
            std::ranges::sort(m_SortedShiftIndices,
                              [xAxis](const axis_size_t& a, const axis_size_t& b) {
                                  const auto& aInterval = xAxis[a].interval();
                                  const auto& bInterval = xAxis[b].interval();
                                  return aInterval.endInMinutes() > bInterval.endInMinutes() || (aInterval.
                                      endInMinutes() == bInterval.endInMinutes() && aInterval.startInMinutes() <
                                      bInterval.startInMinutes());
                              });
        }

        explicit CumulativeFatigueConstraint(const Axes::Axis<Domain::Shift>& xAxis) : CumulativeFatigueConstraint(
            5 * 8 * 60, xAxis) { }

        ~CumulativeFatigueConstraint() override = default;

        [[nodiscard]] ConstraintScore evaluate(const State::DomainState& state) override {
            #ifdef CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
            debug_MaxConsecutiveShiftsPerEmployee.clear();
            debug_MaxConsecutiveShiftsPerEmployee.reserve(state.sizeY());
            #endif

            ConstraintScore totalScore;

            LastConsecutiveShift lastConsecutiveShift;

            for (axis_size_t y = 0; y < state.sizeY(); ++y) {
                #ifdef CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
                DebugMaxConsecutiveShifts debug_MaxConsecutiveShifts {};
                #endif

                for (axis_size_t z = 0, nextXi = 0; z < state.sizeZ(); ++z) {
                    // Find a starting point.
                    lastConsecutiveShift.reset(z);
                    for (axis_size_t xi = nextXi; xi < state.sizeX(); ++xi) {
                        const auto x = m_SortedShiftIndices[xi];
                        if (!state.get(x, y, z)) continue;
                        const auto& shift = state.x()[x];
                        const auto& interval = shift.interval();
                        lastConsecutiveShift.updateCumulativeEnd(0, shift.consecutiveRestMinutes(), interval);
                        lastConsecutiveShift.count += 1;
                        lastConsecutiveShift.totalDuration += static_cast<int64_t>(interval.durationInMinutes());
                        lastConsecutiveShift.x = x;
                        lastConsecutiveShift.xi = xi;
                    }
                    nextXi = 0;

                    // Check shifts on consecutive days.
                    evaluateConsecutiveShifts(state, lastConsecutiveShift, y, z, nextXi, totalScore);

                    #ifdef CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
                    if (debug_MaxConsecutiveShifts.count < lastConsecutiveShift.count) {
                        debug_MaxConsecutiveShifts.count = lastConsecutiveShift.count;
                        debug_MaxConsecutiveShifts.z0 = lastConsecutiveShift.z0;
                        debug_MaxConsecutiveShifts.z = lastConsecutiveShift.z;
                        debug_MaxConsecutiveShifts.totalDuration = lastConsecutiveShift.totalDuration;
                    }
                    #endif
                }

                #ifdef CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
                debug_MaxConsecutiveShiftsPerEmployee.push_back(debug_MaxConsecutiveShifts);
                #endif
            }

            return totalScore;
        }

        #ifdef CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
        [[nodiscard]] bool printsInfo() const override { return true; }

        void printInfo() const override {
            axis_size_t i = 0;
            std::cout << "Max consecutive shifts: " << std::endl;
            for (const auto& maxConsecutiveShifts : debug_MaxConsecutiveShiftsPerEmployee) {
                std::cout << "  Employee #" << (i + 1) << ":" << std::endl;
                std::cout << "    Max consecutive shift count: " << maxConsecutiveShifts.count << std::endl;
                std::cout << "    Day span: " << (maxConsecutiveShifts.z0 + 1) << '-' << (maxConsecutiveShifts.z + 1) <<
                    std::endl;
                const auto totalHours = maxConsecutiveShifts.totalDuration / 60;
                const auto totalMinutes = maxConsecutiveShifts.totalDuration % 60;
                std::cout << "    Total duration: " << totalHours << 'h';
                if (totalMinutes > 0) std::cout << ' ' << totalMinutes << 'm' << std::endl;
                else std::cout << std::endl;
                i += 1;
            }
        }
        #endif

    private:
        std::vector<axis_size_t> m_SortedShiftIndices;
        int32_t m_MaxCumulativeMinutes;

        struct LastConsecutiveShift {
            axis_size_t count {};
            int64_t totalDuration {};
            Time::day_minutes_t end {};
            Time::day_minutes_t consecutiveRestAfter {};
            axis_size_t x {};
            axis_size_t xi {};
            axis_size_t z0 {}, z {};

            void reset(const axis_size_t z) {
                count = 0;
                totalDuration = 0;
                end = 0;
                consecutiveRestAfter = 0;
                x = 0;
                xi = 0;
                z0 = z;
                this->z = z;
            }

            void updateCumulativeEnd(const axis_size_t deltaZ, const Time::day_minutes_t consecutiveRestAfter,
                                     const Time::DailyInterval& interval) {
                if (static_cast<int32_t>(end) + static_cast<int32_t>(this->consecutiveRestAfter) < static_cast<int32_t>(
                    deltaZ) * 24 * 60 + static_cast<int32_t>(interval.endInMinutes()) + static_cast<int32_t>(
                    consecutiveRestAfter)) { this->consecutiveRestAfter = consecutiveRestAfter; }
                end = interval.endInMinutes();
            }
        };

        #ifdef CUMULATIVEFATIGUECONSTRAINT_CONSTRAINT_DEBUG_INFO
        struct DebugMaxConsecutiveShifts {
            axis_size_t count {};
            axis_size_t z0 {}, z {};
            int64_t totalDuration {};
        };

        std::vector<DebugMaxConsecutiveShifts> debug_MaxConsecutiveShiftsPerEmployee;
        #endif

        void evaluateConsecutiveShifts(const State::DomainState& state, LastConsecutiveShift& lastConsecutiveShift,
                                       const axis_size_t y, axis_size_t& z, axis_size_t& nextXi,
                                       ConstraintScore& totalScore) const {
            for (axis_size_t i = 1; z + i < state.sizeZ(); ++i) {
                for (axis_size_t xi = 0; xi < state.sizeX(); ++xi) {
                    const auto x = m_SortedShiftIndices[xi];
                    if (!state.get(x, y, z + i))
                        continue;

                    const auto& shift = state.x()[x];
                    const auto& interval = shift.interval();

                    const auto deltaZ = z + i - lastConsecutiveShift.z;
                    const auto breakStart = static_cast<int64_t>(lastConsecutiveShift.end);
                    const auto breakEnd = static_cast<int64_t>(deltaZ * 24 * 60) + static_cast<int64_t>(interval
                        .startInMinutes());
                    if (breakEnd - breakStart < lastConsecutiveShift.consecutiveRestAfter) {
                        lastConsecutiveShift.updateCumulativeEnd(
                            deltaZ, shift.consecutiveRestMinutes(), interval);
                        lastConsecutiveShift.count += 1;
                        lastConsecutiveShift.totalDuration += static_cast<int64_t>(interval.
                            durationInMinutes());
                        lastConsecutiveShift.x = x;
                        lastConsecutiveShift.xi = xi;
                        lastConsecutiveShift.z = z + i;

                        if (lastConsecutiveShift.totalDuration > m_MaxCumulativeMinutes) {
                            totalScore.violate(Violation::xyz(x, y, z + i, {-1}));
                            if (x + 1 >= state.sizeX()) {
                                z = lastConsecutiveShift.z + 1;
                                nextXi = 0;
                            } else {
                                z = lastConsecutiveShift.z;
                                nextXi = x + 1;
                            }
                            return;
                        }
                    }
                }
            }
        }
    };
}

#endif //CUMULATIVEFATIGUECONSTRAINT_H
