#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <flecs.h>
#include <glm/glm.hpp>
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_device.h"
#include "scene/mesh.h"

namespace Posideon {
    struct Vertex {
        float position[3];
        float color[4];
    };

    struct Transform {
        glm::mat4 model;
    };

    struct ForwardRenderer {
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VulkanDevice device;
        VkQueue graphics_queue = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
        VkCommandPool command_pool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> command_buffers;
        VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
        VkSemaphore rendering_finished_semaphore = VK_NULL_HANDLE;
        VkFence fence;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

        VkFormat swapchain_format;

        uint32_t width;
        uint32_t height;
        bool vsync;

        flecs::query<Mesh, Transform> query;
        struct {
            VulkanBuffer buffer;
            void* mapped;
            size_t capacity = 0;
        } transform_dynamic;
        std::vector<GPUMesh> meshes;

        void create_swapchain();
        void allocate_command_buffers();
        void create_descriptor_objects();
        void add_gpu_mesh(Mesh& mesh);

        void prepare_uniform_buffers();
        void render();
    };

    ForwardRenderer init_forward_renderer(HINSTANCE hinstance, HWND hwnd, flecs::world& world, uint32_t width, uint32_t height);
}