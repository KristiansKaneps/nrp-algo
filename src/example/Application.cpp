#include "Application.h"

#include "GUI/Scenes/TimetableScene.h"
#include "GUI/Scenes/ScoreStatisticsScene.h"

using State::state_size_t;

void Application::onStart() {
    state().renderCache.employeeTotalWorkDuration = new uint64_t[state().state.sizeY()];
    state().renderCache.dayCoverageValid = new bool[state().state.sizeZ()];
    state().renderCache.xw = new BitArray::BitArray(state().state.sizeX() * state().state.sizeW());

    state().renderCache.weekends.reserve(state().state.sizeZ());
    state().renderCache.employeeAvailabilityPerDay = new std::vector<RenderCache::DayAvailability>[state().state.sizeY() * state().state.sizeZ()];

    for (axis_size_t z = 0; z < state().state.sizeZ(); ++z) {
        const uint8_t weekday = Time::InstantToWeekday(state().state.range().getDayAt(z, state().state.timeZone()));
        state().renderCache.weekends.emplace_back(weekday == 5 || weekday == 6);
        const auto &dayRange = state().state.range().getDayRangeAt(z, state().state.timeZone());
        for (axis_size_t y = 0; y < state().state.sizeY(); ++y) {
            const auto &e = state().state.y()[y];

            std::vector<RenderCache::DayAvailability> dayAvailabilities;

            auto unpaidUnavailabilityRangeCollection = e.unpaidUnavailableAvailability().m_RangeCollection;
            const auto &unpaidUnavailabilityRangeIntersection = unpaidUnavailabilityRangeCollection.getIntersection(dayRange);
            for (const auto &range : unpaidUnavailabilityRangeIntersection.ranges()) {
                if (range.intersects(dayRange)) {
                    const auto &start = range.start() > dayRange.start() ? range.start() : dayRange.start();
                    const auto &end = range.end() > dayRange.end() ? dayRange.end() : range.end();

                    using std::chrono_literals::operator ""min;

                    const auto dayDuration = static_cast<double>(dayRange.duration() / 1min);
                    const auto startOffset = static_cast<double>((start - dayRange.start()) / 1min);
                    const auto endOffset = static_cast<double>((dayRange.end() - end) / 1min);

                    const auto startPercentage = static_cast<float>(startOffset / dayDuration);
                    const auto widthPercentage = static_cast<float>((dayDuration - endOffset - startOffset) / dayDuration);

                    dayAvailabilities.push_back({Availability::Type::UNAVAILABLE, {startPercentage, widthPercentage}});
                }
            }

            auto paidUnavailabilityRangeCollection = e.paidUnavailableAvailability().m_RangeCollection;
            const auto &paidUnavailabilityRangeIntersection = paidUnavailabilityRangeCollection.getIntersection(dayRange);
            for (const auto &range : paidUnavailabilityRangeIntersection.ranges()) {
                if (range.intersects(dayRange)) {
                    const auto &start = range.start() > dayRange.start() ? range.start() : dayRange.start();
                    const auto &end = range.end() > dayRange.end() ? dayRange.end() : range.end();

                    using std::chrono_literals::operator ""min;

                    const auto dayDuration = static_cast<double>(dayRange.duration() / 1min);
                    const auto startOffset = static_cast<double>((start - dayRange.start()) / 1min);
                    const auto endOffset = static_cast<double>((dayRange.end() - end) / 1min);

                    const auto startPercentage = static_cast<float>(startOffset / dayDuration);
                    const auto widthPercentage = static_cast<float>((dayDuration - endOffset - startOffset) / dayDuration);

                    dayAvailabilities.push_back({Availability::Type::UNAVAILABLE, {startPercentage, widthPercentage}});
                }
            }

            auto desiredRangeCollection = e.desiredAvailability().m_RangeCollection;
            const auto &desiredRangeIntersection = desiredRangeCollection.getIntersection(dayRange);
            for (const auto &range : desiredRangeIntersection.ranges()) {
                if (range.intersects(dayRange)) {
                    const auto &start = range.start() > dayRange.start() ? range.start() : dayRange.start();
                    const auto &end = range.end() > dayRange.end() ? dayRange.end() : range.end();

                    using std::chrono_literals::operator ""min;

                    const auto dayDuration = static_cast<double>(dayRange.duration() / 1min);
                    const auto startOffset = static_cast<double>((start - dayRange.start()) / 1min);
                    const auto endOffset = static_cast<double>((dayRange.end() - end) / 1min);

                    const auto startPercentage = static_cast<float>(startOffset / dayDuration);
                    const auto widthPercentage = static_cast<float>((dayDuration - endOffset - startOffset) / dayDuration);

                    dayAvailabilities.push_back({Availability::Type::DESIRED, {startPercentage, widthPercentage}});
                }
            }

            *(state().renderCache.employeeAvailabilityPerDay + y * state().state.sizeZ() + z) = dayAvailabilities;

            // if (e.unpaidUnavailableAvailability().m_RangeCollection.intersects(dayRange) || e.paidUnavailableAvailability().m_RangeCollection.intersects(dayRange))
            //     employeeAvailabilityPerDay[y * appState().state.sizeZ() + z] = Availability::Type::UNAVAILABLE;
            // else if (e.desiredAvailability().m_RangeCollection.intersects(dayRange))
            //     employeeAvailabilityPerDay[y * appState().state.sizeZ() + z] = Availability::Type::DESIRED;
            // else
            //     employeeAvailabilityPerDay[y * appState().state.sizeZ() + z] = Availability::Type::AVAILABLE;
        }
    }
}

void Application::onClose() {
    g_LocalSearchShouldStop = true;
}

void Application::mainLoop(const double dt, const uint64_t elapsedTicks) {
    // if (dt > 0.02)
    //     std::cout << "Delta time: " << dt << std::endl;

    bool stateUpdated = false;

    if (elapsedTicks % 30 == 0) {
        if (g_UpdateFlag == LocalSearchUpdateFlag::PENDING) {
            if (g_ConcurrentDataMutex.try_lock()) {
                g_UpdateFlag = LocalSearchUpdateFlag::NONE;
                // ReSharper disable CppDFANullDereference
                gp_AppState->state = gp_Update->state;
                gp_AppState->score = gp_Update->score;
                gp_AppState->scoreStatistics = gp_Update->scoreStatistics;
                // ReSharper restore CppDFANullDereference
                g_ConcurrentDataMutex.unlock();

                stateUpdated = true;
            }
        }
    }

    AppState& appState = Application::state();

    const axis_size_t shiftCount = appState.state.sizeX();
    const axis_size_t employeeCount = appState.state.sizeY();
    const axis_size_t dayCount = appState.state.sizeZ();
    const axis_size_t skillCount = appState.state.sizeW();

    uint64_t &maxTotalWorkDuration = appState.renderCache.maxTotalWorkDuration;
    uint64_t &minTotalWorkDuration = appState.renderCache.minTotalWorkDuration;
    auto *employeeTotalWorkDuration = appState.renderCache.employeeTotalWorkDuration;

    auto *dayCoverageValid = appState.renderCache.dayCoverageValid;

    if (stateUpdated) {
        maxTotalWorkDuration = 0;
        minTotalWorkDuration = std::numeric_limits<uint64_t>::max();
        for (axis_size_t i = 0; i < employeeCount; ++i) {
            employeeTotalWorkDuration[i] = 0;
            for (axis_size_t j = 0; j < shiftCount; ++j) {
                for (axis_size_t k = 0; k < dayCount; ++k) {
                    if (!appState.state.get(j, i, k)) continue;
                    using std::chrono_literals::operator ""min;
                    const auto duration = static_cast<uint64_t>(appState.state.x()[j].interval().toRange(
                            appState.state.range().getDayAt(k, appState.state.timeZone())).duration<std::chrono::minutes>()
                        /
                        1min);
                    employeeTotalWorkDuration[i] += duration;
                }
            }
            if (employeeTotalWorkDuration[i] > maxTotalWorkDuration) maxTotalWorkDuration = employeeTotalWorkDuration[i];
            if (employeeTotalWorkDuration[i] < minTotalWorkDuration) minTotalWorkDuration = employeeTotalWorkDuration[i];
        }

        for (axis_size_t i = 0; i < dayCount; ++i) {
            dayCoverageValid[i] = true;
            for (axis_size_t j = 0; j < shiftCount; ++j) {
                const Shift& s = appState.state.x()[j];
                const auto reqSlots = s.requiredSlotCount(i);
                const auto maxSlots = s.slotCount(i);
                uint8_t assignedSlots = 0;
                for (axis_size_t k = 0; k < employeeCount; ++k) {
                    if (!appState.state.get(j, k, i)) continue;
                    assignedSlots++;
                }

                if (assignedSlots < reqSlots || assignedSlots > maxSlots) {
                    dayCoverageValid[i] = false;
                    break;
                }
            }
        }
    }

    // Controls
    //----------------------------------------------------------------------------------

    if (IsKeyDown(KEY_ONE)) {
        setScene<GUI::TimetableScene>();
    } else if (IsKeyDown(KEY_TWO)) {
        setScene<GUI::ScoreStatisticsScene>();
    }

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    mp_Scene->render(dt, elapsedTicks);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
