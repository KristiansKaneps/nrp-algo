#include "Application.h"

using State::state_size_t;

struct DayAvailability {
    struct Region {
        float start, width;
    };

    Availability::Type type;
    Region region;
};

std::vector<DayAvailability>* employeeAvailabilityPerDay;

void Application::onStart() {
    appState().renderCache.employeeTotalWorkDuration = new uint64_t[appState().state.sizeY()];
    appState().renderCache.dayCoverageValid = new bool[appState().state.sizeZ()];
    appState().renderCache.xw = new BitArray::BitArray(appState().state.sizeX() * appState().state.sizeW());

    employeeAvailabilityPerDay = new std::vector<DayAvailability>[appState().state.sizeY() * appState().state.sizeZ()];
    for (axis_size_t z = 0; z < appState().state.sizeZ(); ++z) {
        const auto &dayRange = appState().state.range().getDayRangeAt(z, appState().state.timeZone());
        for (axis_size_t y = 0; y < appState().state.sizeY(); ++y) {
            const auto &e = appState().state.y()[y];

            std::vector<DayAvailability> dayAvailabilities;

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

            *(employeeAvailabilityPerDay + y * appState().state.sizeZ() + z) = dayAvailabilities;

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

    delete[] employeeAvailabilityPerDay;
}

void Application::mainLoop(const double dt, const uint64_t elapsedTicks) {
    // if (dt > 0.02)
    //     std::cout << "Delta time: " << dt << std::endl;

    bool stateUpdated = false;

    if (elapsedTicks % 30 == 0) {
        if (g_UpdateFlag == LocalSearchUpdateFlag::NEW_BEST_AVAILABLE) {
            if (g_ConcurrentDataMutex.try_lock()) {
                g_UpdateFlag = LocalSearchUpdateFlag::NONE;
                // ReSharper disable CppDFANullDereference
                gp_AppState->state = gp_Update->state;
                gp_AppState->score = gp_Update->score;
                // ReSharper restore CppDFANullDereference
                g_ConcurrentDataMutex.unlock();

                stateUpdated = true;
            }
        }
    }

    AppState& appState = Application::appState();

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

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    constexpr int colIdHeight = 20;
    constexpr int rowIdWidth = 100;

    int rowHeight = static_cast<int>(static_cast<axis_size_t>(m_WindowHeight - colIdHeight) / employeeCount);
    int colWidth = static_cast<int>(static_cast<axis_size_t>(m_WindowWidth - rowIdWidth) / dayCount);

    for (axis_size_t i = 0; i < employeeCount; ++i) {
        const Employee& e = appState.state.y()[i];

        const int x = 0;
        const int y = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + i * static_cast<axis_size_t>(rowHeight));
        DrawLine(0, y, m_WindowWidth, y, GRAY);

        const auto &name = e.name();
        DrawText(name.c_str(), x + 5, y + 2, 10, BLACK);

        const uint64_t totalMinutes = employeeTotalWorkDuration[i];
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << static_cast<double>(totalMinutes) / 60.0 << 'h';
        std::string durationStr = stream.str();

        constexpr uint8_t baseChannel = 130;
        constexpr uint8_t channelSpace = std::numeric_limits<uint8_t>::max() - baseChannel;
        const double interp = static_cast<double>(totalMinutes - minTotalWorkDuration) / static_cast<double>(
            maxTotalWorkDuration - minTotalWorkDuration);
        auto durationColor = Color {
            .r = static_cast<uint8_t>(baseChannel + channelSpace * interp),
            .g = baseChannel,
            .b = baseChannel,
            .a = 255
        };

        DrawText(durationStr.c_str(), x + 26 + 4 * name.size(), y + 2, 10, durationColor);
    }

    for (axis_size_t i = 0; i < dayCount; ++i) {
        const Day& d = appState.state.z().entities()[i];

        const int x = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth));
        const int y = 0;
        DrawLine(x, 0, x, m_WindowHeight, GRAY);

        DrawText(std::to_string(d.index() + 1).c_str(), x + 3, y + 5, 10, dayCoverageValid[i] ? GREEN : RED);
    }

    auto xw = *appState.renderCache.xw;
    for (axis_size_t i = 0; i < dayCount; ++i) {
        const int ox = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth));

        for (axis_size_t j = 0; j < employeeCount; ++j) {
            const int oy = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + j * static_cast<axis_size_t>(rowHeight));

            const auto &availabilities = employeeAvailabilityPerDay[j * appState.state.sizeZ() + i];
            for (const auto &availability : availabilities) {
                if (availability.type != Availability::Type::AVAILABLE) {
                    const Color color = availability.type == Availability::Type::UNAVAILABLE ? Color {255, 127, 127, 63} : Color {127, 255, 127, 63};

                    const int rectX = ox + static_cast<int>(static_cast<float>(colWidth) * availability.region.start);
                    int rectW = static_cast<int>(static_cast<float>(colWidth) * availability.region.width);

                    if (availability.region.width == 1.0f) rectW -= 1;

                    DrawRectangle(rectX, oy, rectW, rowHeight - 1, color);
                }
            }


            appState.state.getPlaneXW(xw, j, i);

            int inlineOffset = 0;

            for (axis_size_t k = 0; k < shiftCount; ++k) {
                for (axis_size_t l = 0; l < skillCount; ++l) {
                    if (xw.get(k * skillCount + l)) {
                        const Shift& s = appState.state.x().entities()[k];
                        DrawText(s.name().c_str(), ox + inlineOffset + 2, oy + 2, 8, BLACK);
                        inlineOffset += 7;
                        break;
                    }
                }
            }
        }
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
}
