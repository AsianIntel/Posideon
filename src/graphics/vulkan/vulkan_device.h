#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>

namespace Posideon {
    struct VulkanPhysicalDevice {
        VkPhysicalDevice raw;
        VkPhysicalDeviceProperties device_properties{};
        VkPhysicalDeviceFeatures device_features{};
        VkPhysicalDeviceMemoryProperties device_memory_properties{};
        uint32_t graphics_family_index = -1;

        explicit VulkanPhysicalDevice(VkPhysicalDevice device): raw(device) {}
    };

    class VulkanDevice {
        VulkanPhysicalDevice m_physicalDevice;
        VkDevice m_device;

    public:
        VulkanDevice(VulkanPhysicalDevice const& physical_device, VkDevice device): m_physicalDevice(physical_device), m_device(device) {}

        [[nodiscard]] VkQueue get_queue() const;
        [[nodiscard]] VkSurfaceCapabilitiesKHR get_physical_device_surface_capabilities(VkSurfaceKHR surface) const;
        [[nodiscard]] std::vector<VkPresentModeKHR> get_surface_present_modes(VkSurfaceKHR surface) const;
        [[nodiscard]] std::vector<VkSurfaceFormatKHR> get_surface_formats(VkSurfaceKHR surface) const;
        [[nodiscard]] std::vector<VkImage> get_swapchain_images(VkSwapchainKHR swapchain) const;

        [[nodiscard]] std::vector<VkCommandBuffer> allocate_command_buffers(VkCommandPool command_pool, uint32_t buffer_count) const;
        [[nodiscard]] VkSwapchainKHR create_swapchain(const VkSwapchainCreateInfoKHR& create_info) const;
        [[nodiscard]] VkImageView create_image_view(const VkImageViewCreateInfo& create_info) const;
        [[nodiscard]] VkCommandPool create_command_pool() const;
        [[nodiscard]] VkSemaphore create_semaphore() const;

        uint32_t acquire_next_image(VkSwapchainKHR swapchain, VkSemaphore semaphore) const;

        void destroy_image_view(VkImageView image_view) const;
        void destroy_swapchain(VkSwapchainKHR swapchain) const;
    };
}