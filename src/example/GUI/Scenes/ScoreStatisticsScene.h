#ifndef SCORESTATISTICSSCENE_H
#define SCORESTATISTICSSCENE_H

#include "Application.h"

namespace GUI {
    class ScoreStatisticsScene : public Scene {
    public:
        explicit ScoreStatisticsScene(Application *app) : Scene(app) { }
        ~ScoreStatisticsScene() override = default;

        void render(double dt, uint64_t elapsedTicks) override;
    };
}

#endif //SCORESTATISTICSSCENE_H
