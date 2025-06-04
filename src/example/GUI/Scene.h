#ifndef SCENE_H
#define SCENE_H

#include "ApplicationState.h"

#include "State/State.h"

struct WindowDescriptor;
class Application;

namespace GUI {
    using state_size_t = State::state_size_t;
    using axis_size_t = State::axis_size_t;

    class Scene {
    public:
        explicit Scene(Application* app) : mp_App(app) {}
        virtual ~Scene() = default;

        [[nodiscard]] Application& app() const;
        [[nodiscard]] const WindowDescriptor& window() const;
        [[nodiscard]] AppState& state() const;
        [[nodiscard]] RenderCache& renderCache() const;

        virtual void render(double dt, uint64_t elapsedTicks) = 0;
    protected:
        Application* mp_App;
    };

    class EmptyScene final : public Scene {
    public:
        explicit EmptyScene(Application* app) : Scene(app) {}
        ~EmptyScene() override = default;
        void render(double dt, uint64_t elapsedTicks) override {}
    };
}

#endif //SCENE_H
