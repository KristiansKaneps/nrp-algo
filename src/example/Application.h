#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <utility>
#include <chrono>
#include <limits>

#include "raylib.h"

#include "ConcurrentData.h"
#include "Constraints/Constraint.h"

struct AppState {
    Score::Score score;
    State::State<Shift, Employee, Day, Skill> state;
    std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> constraints;

    struct RenderCache {
        uint64_t *employeeTotalWorkDuration{};
        bool *dayCoverageValid{};
        BitArray::BitArray *xw;

        ~RenderCache() {
            delete[] employeeTotalWorkDuration;
            delete[] dayCoverageValid;
            delete xw;
        }
    };

    RenderCache renderCache;
};

inline AppState *gp_AppState = nullptr;

class Application {
public:
    Application(const int width, const int height, std::string title) : m_ScreenWidth(width),
                                                                        m_ScreenHeight(height),
                                                                        m_Title(std::move(title)),
                                                                        m_TargetFPS(60) {
        assert(gp_AppState != nullptr && "Application state should be initialized prior.");
    }

    ~Application() { close(); }

    static AppState &appState() { return *gp_AppState; }

    void start() {
        m_WindowIsAlreadyClosed = false;
        InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title.c_str());
        SetTargetFPS(60);
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
    const int m_ScreenWidth;
    const int m_ScreenHeight;
    const std::string m_Title;
    const int m_TargetFPS;

    bool m_WindowIsAlreadyClosed = true;

    void mainLoop(double dt, uint64_t elapsedTicks);

    void onStart();
    void onClose();
};


#endif //APPLICATION_H
