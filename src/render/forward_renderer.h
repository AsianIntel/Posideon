#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>
#include <vector>
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_device.h"

namespace Posideon {
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

        uint32_t width;
        uint32_t height;

        bool vsync;

        void create_swapchain();
        void allocate_command_buffers();
        void render();
    };

    ForwardRenderer init_forward_renderer(HINSTANCE hinstance, HWND hwnd, uint32_t width, uint32_t height);
}