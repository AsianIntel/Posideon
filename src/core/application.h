#pragma once

#include "defines.h"
#include <memory>
#include <flecs.h>
#include "window/win32/win32_window.h"
#include "render/renderer.h"

namespace Posideon {
    class Application {
        std::unique_ptr<Win32Window> m_window;
        std::unique_ptr<Renderer> m_renderer;
        flecs::world m_world;

        bool m_running;
    public:
        Application();

        void initialize();
        void run();
    };
}