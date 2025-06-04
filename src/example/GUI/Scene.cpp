#include "Scene.h"
#include "Application.h"
#include "ApplicationState.h"

namespace GUI {
    Application& Scene::app() const { return *mp_App; }
    const WindowDescriptor& Scene::window() const { return mp_App->windowDescriptor(); }
    // ReSharper disable once CppMemberFunctionMayBeStatic
    AppState& Scene::state() const { return Application::state(); } // NOLINT(*-convert-member-functions-to-static)
    RenderCache& Scene::renderCache() const { return state().renderCache; }
}