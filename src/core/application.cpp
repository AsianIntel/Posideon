#include "application.h"

namespace Posideon {
    Application::Application() {
        m_running = true;
    }

    void Application::initialize() {
        uint32_t width = 640;
        uint32_t height = 480;

        m_window = std::make_unique<Win32Window>(Win32Window(width, height));
        m_renderer = std::make_unique<ForwardRenderer>(init_forward_renderer(m_window->hInstance, m_window->m_hwnd, m_world, m_window->m_width, m_window->m_height));

        Mesh mesh{
            .vertices = {
                Vertex {.position = { 0.0f, 0.25f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
                Vertex {.position = { -0.25f, -0.25f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f }},
                Vertex {.position = { 0.25f, -0.25f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }
            }
        };
        Transform transform { .model = glm::mat4(0.0f) };

        {
            auto entity = m_world.entity();
            entity.set<Mesh>(mesh);
            entity.set<Transform>(transform);
            m_renderer->add_gpu_mesh(mesh);
        }

        transform_query = m_world.query<Transform>();
    }

    void Application::run() {
        while (m_running) {
            m_window->run();

            transform_query.each([](Transform& transform) {
                transform.model[0][0] += 0.008f;
                if (transform.model[0][0] > 1.25f) {
                    transform.model[0][0] = -1.25f;
                }
            });

            m_renderer->render();
        }
    }
}