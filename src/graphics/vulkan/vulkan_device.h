#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>
#include <optional>

namespace Posideon {
    struct VulkanBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    struct PipelineDescriptor {
        uint32_t vertex_stride;
        std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        VkPipelineLayout layout;
        VkFormat swapchain_format;
    };

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

    private:
        std::optional<uint32_t> get_memory_type_index(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

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
        [[nodiscard]] VkFence create_fence(bool signaled) const;
        [[nodiscard]] VkPipelineLayout create_pipeline_layout(const std::vector<VkDescriptorSetLayout>& set_layouts) const;
        [[nodiscard]] VkPipeline create_pipeline(const PipelineDescriptor& descriptor) const;
        [[nodiscard]] VkShaderModule create_shader_module(const std::vector<char>& code) const;
        [[nodiscard]] VkDescriptorPool create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes) const;
        [[nodiscard]] VkDescriptorSetLayout create_descriptor_set_layout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const;

        uint32_t acquire_next_image(VkSwapchainKHR swapchain, VkSemaphore semaphore) const;
        VkResult wait_for_fence(VkFence fence);
        VkResult reset_fence(VkFence fence);
        std::vector<VkDescriptorSet> allocate_descriptor_sets(VkDescriptorPool descriptor_pool, const std::vector<VkDescriptorSetLayout>& descriptor_layouts) const;
        void update_descriptor_sets(VkDescriptorSet set, VkDescriptorType descriptor_type, uint32_t binding, VkDescriptorBufferInfo* buffer_info) const;
        void map_memory(VulkanBuffer buffer, VkDeviceSize size, void** data) const;
        void unmap_memory(VulkanBuffer buffer) const;

        void destroy_image_view(VkImageView image_view) const;
        void destroy_swapchain(VkSwapchainKHR swapchain) const;
        void destroy_buffer(VulkanBuffer buffer) const;

        VulkanBuffer create_buffer(VkBufferUsageFlags usage, VkDeviceSize size) {
            VkBufferCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = size,
                    .usage = usage
            };

            VkBuffer buffer;
            VkResult res = vkCreateBuffer(m_device, &createInfo, nullptr, &buffer);
            POSIDEON_ASSERT(res == VK_SUCCESS)

            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

            VkMemoryAllocateInfo allocInfo{
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize = memoryRequirements.size,
                    .memoryTypeIndex = get_memory_type_index(memoryRequirements.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT).value(),
            };

            VkDeviceMemory memory;
            res = vkAllocateMemory(m_device, &allocInfo, nullptr, &memory);
            POSIDEON_ASSERT(res == VK_SUCCESS)

            vkBindBufferMemory(m_device, buffer, memory, 0);

            return { buffer, memory };
        }

        template<typename T>
        VulkanBuffer create_buffer(VkBufferUsageFlags usage, VkDeviceSize size, T* data) {
            VkBufferCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = size * sizeof(T),
                    .usage = usage
            };

            VkBuffer buffer;
            VkResult res = vkCreateBuffer(m_device, &createInfo, nullptr, &buffer);
            POSIDEON_ASSERT(res == VK_SUCCESS)

            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

            VkMemoryAllocateInfo allocInfo{
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize = memoryRequirements.size,
                    .memoryTypeIndex = get_memory_type_index(memoryRequirements.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT).value(),
            };

            VkDeviceMemory memory;
            res = vkAllocateMemory(m_device, &allocInfo, nullptr, &memory);
            POSIDEON_ASSERT(res == VK_SUCCESS)

            void* bufferData;
            vkMapMemory(m_device, memory, 0, allocInfo.allocationSize, 0, &bufferData);
            memcpy(bufferData, data, createInfo.size);
            vkUnmapMemory(m_device, memory);

            vkBindBufferMemory(m_device, buffer, memory, 0);

            return { buffer, memory };
        }
    };
}