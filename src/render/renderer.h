#pragma once

#include "defines.h"

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "graphics/vulkan/vulkan_device.h"
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
        VkSurfaceKHR surface;
        VulkanPhysicalDevice physical_device;
        VulkanDevice device;
        VkQueue queue;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;

        FrameData frames[FRAME_OVERLAP];
        size_t frame_number;

        void create_swapchain();
        void create_command_structures();
        void create_sync_structures();

        void render();

        FrameData& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; }
    };

    Renderer init_renderer(uint32_t width, uint32_t height, Win32Window* window);
}
