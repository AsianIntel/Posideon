#pragma once

#include "defines.h"
#include "graphics/vulkan/vulkan_device.h"

namespace Posideon {
	struct Vertex;

	struct Mesh {
		std::vector<Vertex> vertices;
	};

    struct GPUMesh {
        VulkanBuffer vertex_buffer;
        std::optional<VulkanBuffer> index_buffer;
        uint32_t vertex_count = 0;
        uint32_t index_count = 0;
    };
}