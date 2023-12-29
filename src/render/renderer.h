#pragma once

#include "defines.h"

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "graphics/vulkan/vulkan_device.h"
#include "graphics/vulkan/vulkan_command_encoder.h"
#include "window/win32/win32_window.h"

namespace Posideon {
    static constexpr uint32_t FRAME_OVERLAP = 2;

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

        DescriptorAllocator global_descriptor_allocator;

        VulkanImage draw_image;
        VkDescriptorSet draw_image_set;
        VkDescriptorSetLayout draw_image_set_layout;
        
        VkPipelineLayout gradient_layout;
        VkPipeline gradient_pipeline;

        FrameData frames[FRAME_OVERLAP];
        size_t frame_number;

        void create_swapchain();
        void create_command_structures();
        void create_sync_structures();
        void create_descriptors();
        void create_pipelines();
        void create_background_pipelines();

        void render();
        void draw_background(const VulkanCommandEncoder& encoder) const;

        FrameData& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; }
    };

    Renderer init_renderer(uint32_t width, uint32_t height, Win32Window* window);
}
