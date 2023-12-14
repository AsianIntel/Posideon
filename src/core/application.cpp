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

        Mesh mesh{
            .vertices = {
                Vertex {.position = { 0.0f, 0.25f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
                Vertex {.position = { -0.25f, -0.25f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }},
                Vertex {.position = { 0.25f, -0.25f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }
            }
        };
        Transform transform;
        m_renderer->add_mesh(mesh, transform);
    }

    void Application::run() {
        while (m_running) {
            m_window->run();

            m_renderer->transforms[0].uBufVS.displacement[0] += 0.008f;
            if (m_renderer->transforms[0].uBufVS.displacement[0] > 1.25f) {
                m_renderer->transforms[0].uBufVS.displacement[0] = -1.25f;
            }

            void* buffer_data;
            m_renderer->device.map_memory(m_renderer->transforms[0].buffer, 4 * sizeof(float), &buffer_data);
            memcpy(buffer_data, &m_renderer->transforms[0], 4 * sizeof(float));
            m_renderer->device.unmap_memory(m_renderer->transforms[0].buffer);

            m_renderer->render();
        }
    }
}