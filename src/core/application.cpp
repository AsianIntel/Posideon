#include "application.h"

namespace Posideon {
    Application::Application() {
        m_running = true;
    }

    void Application::initialize() {
        uint32_t width = 640;
        uint32_t height = 480;

        m_window = std::make_unique<Win32Window>(Win32Window(width, height));
        m_renderer = std::make_unique<ForwardRenderer>(init_forward_renderer(m_window->hInstance, m_window->m_hwnd, m_window->m_width, m_window->m_height));
    }

    void Application::run() {
        while (m_running) {
            m_window->run();
            m_renderer->render();
        }
    }
}