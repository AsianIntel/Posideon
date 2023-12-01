#pragma once

#include <memory>

#include "defines.h"
#include "window/win32/win32_window.h"

namespace Posideon {
    class Application {
        std::unique_ptr<Win32Window> m_window;
        bool m_running;
    public:
        Application();

        void initialize();
        void run();
    };
}