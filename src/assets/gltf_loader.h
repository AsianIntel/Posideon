#pragma once

#include "defines.h"
#include <optional>
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

#include "graphics/vulkan/vulkan_types.h"

namespace Posideon {
    struct Renderer;
    
    struct GltfSurface {
        uint32_t start_index;
        uint32_t count;
    };

    struct GltfAsset {
        std::string name;
        std::vector<GltfSurface> surfaces;

        GPUMeshBuffers mesh_buffers;
    };
    
    std::optional<std::vector<std::shared_ptr<GltfAsset>>> load_gltf_meshes(Renderer* renderer, std::filesystem::path path);
}
