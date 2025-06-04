#ifndef TIMETABLESCENE_H
#define TIMETABLESCENE_H

#include "Application.h"

namespace GUI {
    class TimetableScene : public Scene {
    public:
        explicit TimetableScene(Application* app) : Scene(app) {}
        ~TimetableScene() override = default;

        void render(double dt, uint64_t elapsedTicks) override;
    };
}

#endif //TIMETABLESCENE_H
