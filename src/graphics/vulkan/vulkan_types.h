#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "vulkan_device.h"

namespace Posideon {
    struct Vertex {
        glm::vec3 position;
        float uv_x;
        glm::vec3 normal;
        float uv_y;
        glm::vec4 color;
    };

    struct GPUMeshBuffers {
        VulkanBuffer index_buffer;
        VulkanBuffer vertex_buffer;
        VkDeviceAddress vertex_buffer_address;
    };
}
