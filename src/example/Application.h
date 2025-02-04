#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <utility>

#include "raylib.h"

#include "ConcurrentData.h"
#include "Constraints/Constraint.h"

struct AppState {
    Score::Score score;
    State::State<Shift, Employee, Day, Skill> state;
    std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> constraints;
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

    void start() {
        m_WindowIsAlreadyClosed = false;
        InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title.c_str());
        SetTargetFPS(60);

        while (!WindowShouldClose()) { mainLoop(); }

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

    void mainLoop();

    static AppState &appState() { return *gp_AppState; }

    void onClose();
};


#endif //APPLICATION_H
