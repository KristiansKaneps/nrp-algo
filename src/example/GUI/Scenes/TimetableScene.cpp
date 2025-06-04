#include "TimetableScene.h"

#include "Domain/Entities/Employee.h"

void GUI::TimetableScene::render(const double dt, const uint64_t elapsedTicks) {
    constexpr int colIdHeight = 20;
    constexpr int rowIdWidth = 100;

    const axis_size_t shiftCount = state().state.sizeX();
    const axis_size_t employeeCount = state().state.sizeY();
    const axis_size_t dayCount = state().state.sizeZ();
    const axis_size_t skillCount = state().state.sizeW();

    const uint64_t maxTotalWorkDuration = state().renderCache.maxTotalWorkDuration;
    const uint64_t minTotalWorkDuration = state().renderCache.minTotalWorkDuration;
    const auto *employeeTotalWorkDuration = state().renderCache.employeeTotalWorkDuration;

    const auto *dayCoverageValid = state().renderCache.dayCoverageValid;

    const int rowHeight = static_cast<int>(static_cast<axis_size_t>(window().height - colIdHeight) / employeeCount);
    const int colWidth = static_cast<int>(static_cast<axis_size_t>(window().width - rowIdWidth) / dayCount);

    for (axis_size_t i = 0; i < employeeCount; ++i) {
        const Employee& e = state().state.y()[i];

        const int x = 0;
        const int y = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + i * static_cast<axis_size_t>(rowHeight));
        DrawLine(0, y, window().width, y, GRAY);

        const auto &name = e.name();
        DrawText(name.c_str(), x + 5, y + 2, 10, BLACK);

        const uint64_t totalMinutes = employeeTotalWorkDuration[i];
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << static_cast<double>(totalMinutes) / 60.0 << 'h';
        std::string durationStr = stream.str();

        constexpr uint8_t baseChannel = 130;
        constexpr uint8_t channelSpace = std::numeric_limits<uint8_t>::max() - baseChannel;
        const double interp = static_cast<double>(totalMinutes - minTotalWorkDuration) / static_cast<double>(
            maxTotalWorkDuration - minTotalWorkDuration);
        auto durationColor = Color {
            .r = static_cast<uint8_t>(baseChannel + channelSpace * interp),
            .g = baseChannel,
            .b = baseChannel,
            .a = 255
        };

        DrawText(durationStr.c_str(), x + 26 + 4 * name.size(), y + 2, 10, durationColor);
    }

    for (axis_size_t i = 0; i < dayCount; ++i) {
        const Day& d = state().state.z().entities()[i];

        const int x = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth));
        const int y = 0;
        DrawLine(x, 0, x, window().height, GRAY);

        if (renderCache().weekends[d.index()]) {
            DrawRectangle(x, y, colWidth - 1, colIdHeight - 1, Color {85, 85, 85, 63});
        }

        DrawText(std::to_string(d.index() + 1).c_str(), x + 3, y + 5, 10, dayCoverageValid[i] ? GREEN : RED);
    }

    auto xw = *state().renderCache.xw;
    for (axis_size_t i = 0; i < dayCount; ++i) {
        const int ox = static_cast<int>(static_cast<axis_size_t>(rowIdWidth) + i * static_cast<axis_size_t>(colWidth));

        for (axis_size_t j = 0; j < employeeCount; ++j) {
            const int oy = static_cast<int>(static_cast<axis_size_t>(colIdHeight) + j * static_cast<axis_size_t>(rowHeight));

            const auto &availabilities = renderCache().employeeAvailabilityPerDay[j * state().state.sizeZ() + i];
            for (const auto &availability : availabilities) {
                if (availability.type != Availability::Type::AVAILABLE) {
                    const Color color = availability.type == Availability::Type::UNAVAILABLE ? Color {255, 127, 127, 63} : Color {127, 255, 127, 63};

                    const int rectX = ox + static_cast<int>(static_cast<float>(colWidth) * availability.region.start);
                    int rectW = static_cast<int>(static_cast<float>(colWidth) * availability.region.width);

                    if (availability.region.width == 1.0f) rectW -= 1;

                    DrawRectangle(rectX, oy, rectW, rowHeight - 1, color);
                }
            }


            state().state.getPlaneXW(xw, j, i);

            int inlineOffset = 0;

            for (axis_size_t k = 0; k < shiftCount; ++k) {
                for (axis_size_t l = 0; l < skillCount; ++l) {
                    if (xw.get(k * skillCount + l)) {
                        const Shift& s = state().state.x().entities()[k];
                        const Skill& sk = state().state.w().entities()[l];
                        DrawText(s.name().c_str(), ox + inlineOffset + 2, oy + 2, 8, BLACK);
                        DrawText(sk.name().c_str(), ox + inlineOffset + 2, oy + 10, 8, GRAY);
                        inlineOffset += 7;
                        break;
                    }
                }
            }
        }
    }
}
