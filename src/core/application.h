#pragma once

#include "defines.h"
#include <memory>
#include <flecs.h>
#include "window/win32/win32_window.h"
#include "render/forward_renderer.h"

namespace Posideon {
    class Application {
        std::unique_ptr<Win32Window> m_window;
        std::unique_ptr<ForwardRenderer> m_renderer;
        flecs::world m_world;
        flecs::query<Transform> transform_query;

        bool m_running;
    public:
        Application();

        void initialize();
        void run();
    };
}