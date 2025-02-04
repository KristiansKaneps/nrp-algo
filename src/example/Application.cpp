#include "Application.h"

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
            gp_AppState->state = gp_Update->state;
            gp_AppState->score = gp_Update->score;
            g_ConcurrentDataMutex.unlock();
        }
    }

    const AppState appState = Application::appState();

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
