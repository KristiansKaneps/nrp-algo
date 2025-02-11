#include "Application.h"

void Application::onStart() {
    appState().renderCache.employeeTotalWorkDuration = new uint64_t[appState().state.sizeY()];
    appState().renderCache.dayCoverageValid = new bool[appState().state.sizeZ()];
    appState().renderCache.xw = new BitArray::BitArray(appState().state.sizeX() * appState().state.sizeW());
}

void Application::onClose() { g_LocalSearchShouldStop = true; }

void Application::mainLoop(const double dt, const uint64_t elapsedTicks) {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    if (dt > 0.02)
        std::cout << "Delta time: " << dt << std::endl;

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
                const auto reqSlots = s.requiredSlotCount();
                const auto maxSlots = s.slotCount();
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

    int rowHeight = static_cast<int>(static_cast<axis_size_t>(m_ScreenHeight - colIdHeight) / employeeCount);
    int colWidth = static_cast<int>(static_cast<axis_size_t>(m_ScreenWidth - rowIdWidth) / dayCount);

    for (axis_size_t i = 0; i < employeeCount; ++i) {
        const Employee& e = appState.state.y()[i];

        const int x = 0;
        const int y = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + i * static_cast<axis_size_t>(rowHeight));
        DrawLine(0, y, m_ScreenWidth, y, GRAY);

        DrawText(std::to_string(e.index() + 1).c_str(), x + 5, y + 2, 10, BLACK);

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

        DrawText(durationStr.c_str(), x + 30, y + 2, 10, durationColor);
    }

    for (axis_size_t i = 0; i < dayCount; ++i) {
        const Day& d = appState.state.z().entities()[i];

        const int x = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth));
        const int y = 0;
        DrawLine(x, 0, x, m_ScreenHeight, GRAY);

        DrawText(std::to_string(d.index() + 1).c_str(), x + 3, y + 5, 10, dayCoverageValid[i] ? GREEN : RED);
    }

    auto xw = *appState.renderCache.xw;
    for (axis_size_t i = 0; i < dayCount; ++i) {
        const int ox = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth)) +
            2;

        for (axis_size_t j = 0; j < employeeCount; ++j) {
            const int oy = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + j * static_cast<axis_size_t>(
                rowHeight)) + 2;

            appState.state.getPlaneXW(xw, j, i);

            int inlineOffset = 0;

            for (axis_size_t k = 0; k < shiftCount; ++k) {
                for (axis_size_t l = 0; l < skillCount; ++l) {
                    if (xw.get(k * skillCount + l)) {
                        const Shift& s = appState.state.x().entities()[k];
                        DrawText(s.name().c_str(), ox + inlineOffset, oy, 8, BLACK);
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
