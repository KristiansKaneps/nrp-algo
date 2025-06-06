#include "ScoreStatisticsScene.h"

#include <cmath>

namespace GUI {
    void ScoreStatisticsScene::render(const double dt, const uint64_t elapsedTicks) {
        constexpr int padding = 64;

        const int width = window().width - 2 * padding;
        const int height = window().height - 2 * padding;

        const auto &statistics = state().scoreStatistics;

        float x = padding;
        float y = padding;
        DrawLineBezier(Vector2{x - 1, static_cast<float>(height + padding)}, Vector2{x + static_cast<float>(width), static_cast<float>(height + padding)}, 2.0f, BLACK);
        DrawLineBezier(Vector2{x, y}, Vector2{x, static_cast<float>(height + padding) + 1}, 2.0f, BLACK);

        const auto maxValue = statistics.max().hard;
        const auto minValue = statistics.min().hard;
        const float ky = maxValue - minValue == 0 ? 0 : static_cast<float>(height / static_cast<double>(maxValue - minValue));

        const auto lastPoint = statistics.points()[statistics.points().size() - 1];
        const float kx = lastPoint.time == 0 ? 0 : static_cast<float>(width / std::log10(lastPoint.time));

        for (size_t i = 0; i < statistics.points().size() - 1; ++i) {
            const auto& point1 = statistics.points()[i];
            const auto& point2 = statistics.points()[i + 1];
            DrawLineV(
                Vector2{padding + static_cast<float>(point1.time == 0 ? 0 : std::log10(point1.time) * kx), window().height - (point1.score.hard - minValue) * ky - padding},
                Vector2{padding + static_cast<float>(point2.time == 0 ? 0 : std::log10(point2.time) * kx), window().height - (point2.score.hard - minValue) * ky - padding},
                RED
            );
        }
    }
}
