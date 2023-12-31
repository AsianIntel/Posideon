#pragma once

#include "defines.h"

#include <cstdint>
#include <functional>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "assets/gltf_loader.h"
#include "graphics/vulkan/vulkan_device.h"
#include "graphics/vulkan/vulkan_command_encoder.h"
#include "window/win32/win32_window.h"
#include "graphics/vulkan/vulkan_types.h"

namespace Posideon {
    static constexpr uint32_t FRAME_OVERLAP = 2;

    struct GPUDrawPushConstants {
        glm::mat4 world_matrix;
        VkDeviceAddress vertex_buffer;
    };

    struct FrameData {
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;
        VkSemaphore swapchain_semaphore;
        VkSemaphore render_semaphore;
        VkFence render_fence;
    };

    struct Renderer {
        uint32_t width;
        uint32_t height;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkSurfaceKHR surface;
        VulkanPhysicalDevice physical_device;
        VulkanDevice device;
        VkQueue queue;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
        VkExtent2D swapchain_extent;
        VkFence immediate_fence;
        VkCommandPool immediate_command_pool;
        VkCommandBuffer immediate_command_buffer;

        DescriptorAllocator global_descriptor_allocator;

        VulkanImage draw_image;
        VulkanImage depth_image;
        VkDescriptorSet draw_image_set;
        VkDescriptorSetLayout draw_image_set_layout;
        
        VkPipelineLayout gradient_layout;
        VkPipeline gradient_pipeline;

        VkPipelineLayout triangle_pipeline_layout;
        VkPipeline triangle_pipeline;

        VkPipelineLayout mesh_pipeline_layout;
        VkPipeline mesh_pipeline;

        FrameData frames[FRAME_OVERLAP];
        size_t frame_number;

        GPUMeshBuffers rectangle;
        std::vector<std::shared_ptr<GltfAsset>> test_meshes;

        void create_swapchain();
        void create_command_structures();
        void create_sync_structures();
        void create_descriptors();
        void create_pipelines();
        void create_background_pipelines();
        void create_triangle_pipeline();
        void create_mesh_pipeline();
        GPUMeshBuffers create_mesh(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices);
        void init_default_data();

        void immediate_submit(std::function<void(VulkanCommandEncoder encoder)>&& function);
        void render();
        void draw_background(const VulkanCommandEncoder& encoder) const;
        void draw_geometry(const VulkanCommandEncoder& encoder) const;
        
        FrameData& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; }
    };

    Renderer init_renderer(uint32_t width, uint32_t height, Win32Window* window);
}
