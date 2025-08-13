#ifndef APPLICATION_H
#define APPLICATION_H

#define DISABLE_RAYLIB_LOGGING

#include <string>
#include <utility>
#include <chrono>

#include "raylib.h"

#include "GUI/Scene.h"

#include "ApplicationState.h"

struct WindowDescriptor {
    int width;
    int height;
    int targetFPS;
    std::string title;
};

class Application {
public:
    Application(const int width, const int height, std::string title) : m_WindowDescriptor(WindowDescriptor{width, height, 60, std::move(title)}), mp_Scene(new GUI::EmptyScene(this)) {
        assert(gp_AppState != nullptr && "Application state should be initialized prior.");
    }

    ~Application() {
        close();
        delete mp_Scene;
    }

    [[nodiscard]] const WindowDescriptor& windowDescriptor() const { return m_WindowDescriptor; }

    [[nodiscard]] static AppState &state() { return *gp_AppState; }

    template<typename SceneType, typename... Args>
    void setScene(Args&&... args) {
        static_assert(std::is_base_of_v<GUI::Scene, SceneType>, "SceneType must derive from GUI::Scene");
        const GUI::Scene *oldScene = mp_Scene;
        mp_Scene = new SceneType(this, std::forward<Args>(args)...);
        delete oldScene;
    }

    void start(const bool exitOnFinish = false) {
        m_WindowIsAlreadyClosed = false;

        #ifdef DISABLE_RAYLIB_LOGGING
        SetTraceLogLevel(LOG_NONE);
        #endif

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(m_WindowDescriptor.width, m_WindowDescriptor.height, m_WindowDescriptor.title.c_str());
        const int targetMonitor = getCurrentlyActiveMonitorByCursorPosition();
        // ReSharper disable once CppUseStructuredBinding
        const Vector2 monitorPosition = GetMonitorPosition(targetMonitor);
        // ReSharper disable CppTooWideScopeInitStatement
        const int monitorWidth = GetMonitorWidth(targetMonitor);
        const int monitorHeight = GetMonitorHeight(targetMonitor);
        // ReSharper restore CppTooWideScopeInitStatement
        if (monitorWidth != 0 && monitorHeight != 0) {
            SetWindowPosition(
                static_cast<int>(monitorPosition.x) + ((monitorWidth - m_WindowDescriptor.width) >> 1),
                static_cast<int>(monitorPosition.y) + ((monitorHeight - m_WindowDescriptor.height) >> 1)
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

            if (IsWindowResized()) [[unlikely]] {
                m_WindowDescriptor.width = GetScreenWidth();
                m_WindowDescriptor.height = GetScreenHeight();
                m_WindowDescriptor.targetFPS = getPreferableTargetFPS();
            }

            mainLoop(dt, elapsedTicks++);

            if (exitOnFinish && gp_AppState->localSearchDone) break;
        }

        close();
    }

    void close() {
        if (m_WindowIsAlreadyClosed) return;
        m_WindowIsAlreadyClosed = true;
        CloseWindow();
        onClose();
    }

private:
    WindowDescriptor m_WindowDescriptor;

    bool m_WindowIsAlreadyClosed = true;

    GUI::Scene* mp_Scene;

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
