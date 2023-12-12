#pragma once

#include "defines.h"
#include "graphics/vulkan/vulkan_device.h"

namespace Posideon {
	struct Vertex;

	struct Mesh {
		std::vector<Vertex> vertices;
		VulkanBuffer vertex_buffer;
	};
}