#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <utility>

#include "raylib.h"

class Application {
public:
    Application(const int width, const int height, std::string title) : m_ScreenWidth(width),
                                                                        m_ScreenHeight(height),
                                                                        m_Title(std::move(title)),
                                                                        m_TargetFPS(60) { }

    ~Application() { close(); }

    void start() {
        m_WindowIsAlreadyClosed = false;
        InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title.c_str());
        SetTargetFPS(60);

        while (!WindowShouldClose()) { mainLoop(); }

        m_WindowIsAlreadyClosed = true;
        CloseWindow();
    }

    void close() {
        if (m_WindowIsAlreadyClosed) return;
        m_WindowIsAlreadyClosed = true;
        CloseWindow();
    }

private:
    const int m_ScreenWidth;
    const int m_ScreenHeight;
    const std::string m_Title;
    const int m_TargetFPS;

    bool m_WindowIsAlreadyClosed = true;

    void mainLoop();
};


#endif //APPLICATION_H
