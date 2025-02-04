#include "Application.h"

void Application::onStart() {

}

void Application::onClose() {
    g_LocalSearchShouldStop = true;
}

void Application::mainLoop() {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    if (g_UpdateFlag == LocalSearchUpdateFlag::NEW_BEST_AVAILABLE) {
        if (g_ConcurrentDataMutex.try_lock()) {
            g_UpdateFlag = LocalSearchUpdateFlag::NONE;
            // ReSharper disable CppDFANullDereference
            gp_AppState->state = gp_Update->state;
            gp_AppState->score = gp_Update->score;
            // ReSharper restore CppDFANullDereference
            g_ConcurrentDataMutex.unlock();
        }
    }

    const AppState appState = Application::appState();

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    constexpr int colIdHeight = 20;
    constexpr int rowIdWidth = 100;

    const axis_size_t shiftCount = appState.state.sizeX();
    const axis_size_t employeeCount = appState.state.sizeY();
    const axis_size_t dayCount = appState.state.sizeZ();
    const axis_size_t skillCount = appState.state.sizeW();

    int rowHeight = static_cast<int>(static_cast<axis_size_t>(m_ScreenHeight - colIdHeight) / employeeCount);
    int colWidth = static_cast<int>(static_cast<axis_size_t>(m_ScreenWidth - rowIdWidth) / dayCount);

    for (axis_size_t i = 0; i < employeeCount; ++i) {
        const Employee &e = appState.state.y().entities()[i];

        const int x = 0;
        const int y = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + i * static_cast<axis_size_t>(rowHeight));

        DrawText(std::to_string(e.index()).c_str(), x + 5, y + 2, 10, BLACK);
        DrawLine(0, y, m_ScreenWidth, y, GRAY);
    }

    for (axis_size_t i = 0; i < dayCount; ++i) {
        const Day &d = appState.state.z().entities()[i];

        const int x = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth));
        const int y = 0;

        DrawText(std::to_string(d.index()).c_str(), x + 3, y + 5, 10, BLACK);
        DrawLine(x, 0, x, m_ScreenHeight, GRAY);
    }

    auto xw = BitArray::BitArray(appState.state.sizeX() * appState.state.sizeW());
    for (axis_size_t i = 0; i < dayCount; ++i) {
        const int ox = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth)) + 2;

        for (axis_size_t j = 0; j < employeeCount; ++j) {
            const int oy = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + j * static_cast<axis_size_t>(rowHeight)) + 2;

            appState.state.getPlaneXW(xw, j, i);

            int inlineOffset = 0;

            for (axis_size_t k = 0; k < shiftCount; ++k) {
                for (axis_size_t l = 0; l < skillCount; ++l) {
                    if (xw.get(k * skillCount + l)) {
                        const Shift &s = appState.state.x().entities()[k];
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
