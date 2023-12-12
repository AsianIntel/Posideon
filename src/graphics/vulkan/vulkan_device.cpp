#include "vulkan_device.h"

namespace Posideon {
    VkQueue VulkanDevice::get_queue() const {
        VkQueue queue;
        vkGetDeviceQueue(m_device, m_physicalDevice.graphics_family_index, 0, &queue);
        return queue;
    }

    VkSurfaceCapabilitiesKHR VulkanDevice::get_physical_device_surface_capabilities(VkSurfaceKHR surface) const {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        const VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice.raw, surface, &surface_capabilities);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return surface_capabilities;
    }

    std::vector<VkPresentModeKHR> VulkanDevice::get_surface_present_modes(VkSurfaceKHR surface) const {
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice.raw, surface, &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice.raw, surface, &present_mode_count, present_modes.data());
        return present_modes;
    }

    std::vector<VkSurfaceFormatKHR> VulkanDevice::get_surface_formats(VkSurfaceKHR surface) const {
        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice.raw, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice.raw, surface, &format_count, formats.data());
        return formats;
    }

    std::vector<VkImage> VulkanDevice::get_swapchain_images(VkSwapchainKHR swapchain) const {
        uint32_t image_count = 0;
        VkResult res = vkGetSwapchainImagesKHR(m_device, swapchain, &image_count, nullptr);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        std::vector<VkImage> images(image_count);
        res = vkGetSwapchainImagesKHR(m_device, swapchain, &image_count, images.data());
        POSIDEON_ASSERT(res == VK_SUCCESS)

        return images;
    }

    std::vector<VkCommandBuffer> VulkanDevice::allocate_command_buffers(VkCommandPool command_pool, uint32_t buffer_count) const {
        VkCommandBufferAllocateInfo command_buffer_allocate_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = buffer_count
        };

        std::vector<VkCommandBuffer> buffers(buffer_count);
        const VkResult res = vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, buffers.data());
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return buffers;
    }

    VkSwapchainKHR VulkanDevice::create_swapchain(const VkSwapchainCreateInfoKHR& create_info) const {
        VkSwapchainKHR swapchain;
        const VkResult res = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &swapchain);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return swapchain;
    }

    VkImageView VulkanDevice::create_image_view(const VkImageViewCreateInfo&create_info) const {
        VkImageView image_view;
        const VkResult res = vkCreateImageView(m_device, &create_info, nullptr, &image_view);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return image_view;
    }

    VkCommandPool VulkanDevice::create_command_pool() const {
        const VkCommandPoolCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_physicalDevice.graphics_family_index
        };
        VkCommandPool pool;
        const VkResult res = vkCreateCommandPool(m_device, &create_info, nullptr, &pool);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return pool;
    }

    VkSemaphore VulkanDevice::create_semaphore() const {
        VkSemaphoreCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkSemaphore semaphore;
        const VkResult res = vkCreateSemaphore(m_device, &create_info, nullptr, &semaphore);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return semaphore;
    }

    uint32_t VulkanDevice::acquire_next_image(VkSwapchainKHR swapchain, VkSemaphore semaphore) const {
        uint32_t image_index;
        VkResult res = vkAcquireNextImageKHR(m_device, swapchain, UINT64_MAX, semaphore, nullptr, &image_index);
        POSIDEON_ASSERT((res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR))
        return image_index;
    }

    void VulkanDevice::destroy_image_view(VkImageView image_view) const {
        vkDestroyImageView(m_device, image_view, nullptr);
    }

    void VulkanDevice::destroy_swapchain(VkSwapchainKHR swapchain) const {
        vkDestroySwapchainKHR(m_device, swapchain, nullptr);
    }
}