#include "application.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Posideon {
    Application::Application() {
        m_running = true;
    }

    void Application::initialize() {
        uint32_t width = 640;
        uint32_t height = 480;

        m_window = std::make_unique<Win32Window>(Win32Window(width, height));
        m_renderer = std::make_unique<Renderer>(init_renderer(width, height, m_window.get()));

        // Mesh mesh{
        //     .vertices = {
        //         { { -1.0f, -1.0f, -1.0f },  {0.0f, 0.0f, 1.0f, 1.0f} },
        //         { { 1.0f, -1.0f, -1.0f },   {0.0f, 1.0f, 1.0f, 1.0f} },
        //         { { 1.0f, 1.0f, -1.0f },    {0.0f, 1.0f, 0.0f, 1.0f} },
        //         { { -1.0f, 1.0f, -1.0f },   {1.0f, 0.0f, 0.0f, 1.0f} },
        //         { { -1.0f, -1.0f, 1.0f },   {1.0f, 0.0f, 1.0f, 1.0f} },
        //         { { 1.0f, -1.0f, 1.0f },    {1.0f, 1.0f, 1.0f, 1.0f} },
        //         { { 1.0f, 1.0f, 1.0f },     {1.0f, 1.0f, 0.0f, 1.0f} },
        //         { { -1.0f, 1.0f, 1.0f },    {0.0f, 0.0f, 0.0f, 1.0f} }
        //     },
        //     .indices = {
        //         0, 1, 3, 3, 1, 2,
        //         1, 5, 2, 2, 5, 6,
        //         5, 4, 6, 6, 4, 7,
        //         4, 0, 7, 7, 0, 3,
        //         3, 2, 7, 7, 2, 6,
        //         4, 5, 0, 0, 5, 1
        //     }
        // };
        //
        // {
        //     Transform transform { .model = glm::mat4(1.0f) };
        //     auto entity = m_world.entity();
        //     entity.set<Mesh>(mesh);
        //     entity.set<Transform>(transform);
        //     m_renderer->add_gpu_mesh(mesh);
        // }
        //
        // {
        //     float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        //     Transform transform { .model = glm::lookAt(glm::vec3(2.0f, -4.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) };
        //     Camera camera { .projection = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 1000.0f) };
        //     auto entity = m_world.entity();
        //     entity.set<Camera>(camera);
        //     entity.set<Transform>(transform);
        // }
        //
        // transform_query = m_world.query<Transform>();
    }

    void Application::run() {
        while (m_running) {
            m_window->run();

            m_renderer->render();
        }
    }
}