#ifndef APPLICATION_H
#define APPLICATION_H

#define DISABLE_RAYLIB_LOGGING

#include <string>
#include <utility>
#include <chrono>

#include "raylib.h"

#include "ApplicationState.h"

class Application {
public:
    Application(const int width, const int height, std::string title) : m_WindowWidth(width),
                                                                        m_WindowHeight(height),
                                                                        m_Title(std::move(title)),
                                                                        m_TargetFPS(60) {
        assert(gp_AppState != nullptr && "Application state should be initialized prior.");
    }

    ~Application() { close(); }

    static AppState &appState() { return *gp_AppState; }

    void start() {
        m_WindowIsAlreadyClosed = false;

        #ifdef DISABLE_RAYLIB_LOGGING
        SetTraceLogLevel(LOG_NONE);
        #endif

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(m_WindowWidth, m_WindowHeight, m_Title.c_str());
        const int targetMonitor = getCurrentlyActiveMonitorByCursorPosition();
        // ReSharper disable once CppUseStructuredBinding
        const Vector2 monitorPosition = GetMonitorPosition(targetMonitor);
        // ReSharper disable CppTooWideScopeInitStatement
        const int monitorWidth = GetMonitorWidth(targetMonitor);
        const int monitorHeight = GetMonitorHeight(targetMonitor);
        // ReSharper restore CppTooWideScopeInitStatement
        if (monitorWidth != 0 && monitorHeight != 0) {
            SetWindowPosition(
                static_cast<int>(monitorPosition.x) + ((monitorWidth - m_WindowWidth) >> 1),
                static_cast<int>(monitorPosition.y) + ((monitorHeight - m_WindowHeight) >> 1)
            );
        }
        SetTargetFPS(getPreferableTargetFPS());

        onStart();

        uint64_t elapsedTicks = 0;
        using clock = std::chrono::system_clock;
        auto before = clock::now();

        while (!WindowShouldClose()) {
            const auto now = clock::now();
            const std::chrono::duration<double> elapsedTime = now - before;
            before = now;
            const double dt = elapsedTime.count();
            mainLoop(dt, elapsedTicks++);

            if (IsWindowResized()) [[unlikely]] {
                m_WindowWidth = GetScreenWidth();
                m_WindowHeight = GetScreenHeight();
                m_TargetFPS = getPreferableTargetFPS();
            }
        }

        m_WindowIsAlreadyClosed = true;
        CloseWindow();
        onClose();
    }

    void close() {
        if (m_WindowIsAlreadyClosed) return;
        m_WindowIsAlreadyClosed = true;
        CloseWindow();
        onClose();
    }

private:
    int m_WindowWidth;
    int m_WindowHeight;
    const std::string m_Title;
    int m_TargetFPS;

    bool m_WindowIsAlreadyClosed = true;

    void mainLoop(double dt, uint64_t elapsedTicks);

    void onStart();
    void onClose();

    static int getPreferableTargetFPS() {
        int refreshRate = 60;
        if (const int monitorRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor()); monitorRefreshRate > 0)
            refreshRate = monitorRefreshRate;
        return refreshRate;
    }

    static int getCurrentlyActiveMonitorByCursorPosition() {
        // ReSharper disable once CppUseStructuredBinding
        const Vector2 mousePos = GetMousePosition(); // Does not work. GLFW does not provide global mouse position.
        const auto x = static_cast<int>(mousePos.x);
        const auto y = static_cast<int>(mousePos.y);
        const int monitorCount = GetMonitorCount();

        for (int i = 0; i < monitorCount; ++i) {
            // ReSharper disable once CppUseStructuredBinding
            const Vector2 monitorPos = GetMonitorPosition(i);
            // ReSharper disable CppTooWideScopeInitStatement
            const int width = GetMonitorWidth(i);
            const int height = GetMonitorHeight(i);
            const auto x0 = static_cast<int>(monitorPos.x);
            const auto y0 = static_cast<int>(monitorPos.y);
            const auto x1 = x0 + width;
            const auto y1 = y0 + height;
            // ReSharper restore CppTooWideScopeInitStatement

            if (x >= x0 && x < x1 && y >= y0 && y < y1)
                return i;
        }

        return 0;
    }
};


#endif //APPLICATION_H
