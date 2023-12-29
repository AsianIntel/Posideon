#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>
#include <optional>
#include <vk_mem_alloc.h>

namespace Posideon {
    class VulkanDevice;
    
    struct VulkanBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;
    };

    struct VulkanImage {
        VkImage image;
        VkImageView image_view;
        VmaAllocation allocation;
        VkExtent3D extent;
        VkFormat format;
    };

    struct GraphicsPipelineDescriptor {
        uint32_t vertex_stride;
        std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        VkPipelineLayout layout;
        VkFormat swapchain_format;
        VkFormat depth_format;
        VkFormat stencil_format;
    };

    struct ComputePipelineDescriptor {
        VkPipelineShaderStageCreateInfo shader_stage;
        VkPipelineLayout layout;
    };

    struct ImageDescriptor {
        VkImageType image_type;
        VkFormat format;
        uint32_t width;
        uint32_t height;
        VkImageUsageFlags usage;
        VkImageLayout initial_layout;
        VkImageViewType image_view_type;
        VkImageAspectFlags aspect_mask;
    };

    struct ImageViewDescriptor {
        VkImageViewType image_view_type;
        VkFormat format;
        VkImageAspectFlags aspect_mask;
    };

    struct DescriptorAllocator {
        struct PoolSizeRatio {
            VkDescriptorType type;
            float ratio;
        };

        VkDescriptorPool pool;

        void init_pool(const VulkanDevice& device, uint32_t max_sets, const std::vector<PoolSizeRatio>& pool_ratios);
        void clear_descriptors(const VulkanDevice& device) const;
        void destroy_pool(const VulkanDevice& device) const;

        VkDescriptorSet allocate(const VulkanDevice& device, VkDescriptorSetLayout layout) const;
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
        VmaAllocator m_allocator;

    private:
        [[nodiscard]] std::optional<uint32_t> get_memory_type_index(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    public:
        VulkanDevice(VulkanPhysicalDevice const& physical_device, VkDevice device, VmaAllocator allocator):
            m_physicalDevice(physical_device), m_device(device), m_allocator(allocator) {}

        [[nodiscard]] VkQueue get_queue() const;
        [[nodiscard]] VkSurfaceCapabilitiesKHR get_physical_device_surface_capabilities(VkSurfaceKHR surface) const;
        [[nodiscard]] std::vector<VkPresentModeKHR> get_surface_present_modes(VkSurfaceKHR surface) const;
        [[nodiscard]] std::vector<VkSurfaceFormatKHR> get_surface_formats(VkSurfaceKHR surface) const;
        [[nodiscard]] std::vector<VkImage> get_swapchain_images(VkSwapchainKHR swapchain) const;
        [[nodiscard]] VkDeviceAddress get_buffer_address(const VulkanBuffer& buffer) const; 

        [[nodiscard]] std::vector<VkCommandBuffer> allocate_command_buffers(VkCommandPool command_pool, uint32_t buffer_count) const;
        [[nodiscard]] VkSwapchainKHR create_swapchain(const VkSwapchainCreateInfoKHR& create_info) const;
        [[nodiscard]] VkCommandPool create_command_pool() const;
        [[nodiscard]] VkSemaphore create_semaphore() const;
        [[nodiscard]] VkFence create_fence(bool signaled) const;
        [[nodiscard]] VkPipelineLayout create_pipeline_layout(const std::vector<VkDescriptorSetLayout>& set_layouts,  const std::vector<VkPushConstantRange>& push_constants) const;
        [[nodiscard]] VkPipeline create_graphics_pipeline(const VkGraphicsPipelineCreateInfo& descriptor) const;
        [[nodiscard]] VkPipeline create_compute_pipeline(const ComputePipelineDescriptor& descriptor) const;
        [[nodiscard]] VkShaderModule create_shader_module(const std::vector<char>& code) const;
        [[nodiscard]] VkDescriptorPool create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes) const;
        [[nodiscard]] VkDescriptorSetLayout create_descriptor_set_layout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const;
        [[nodiscard]] VulkanImage create_image(const ImageDescriptor& descriptor) const;
        [[nodiscard]] VkImageView create_image_view(VkImage image, const ImageViewDescriptor& descriptor) const;
        [[nodiscard]] VulkanBuffer create_buffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage) const;

        uint32_t acquire_next_image(VkSwapchainKHR swapchain, VkSemaphore semaphore) const;
        VkResult wait_for_fence(VkFence fence);
        VkResult reset_fence(VkFence fence);
        void reset_descriptor_pool(VkDescriptorPool pool) const;
        std::vector<VkDescriptorSet> allocate_descriptor_sets(VkDescriptorPool descriptor_pool, const std::vector<VkDescriptorSetLayout>& descriptor_layouts) const;
        void update_descriptor_sets(VkDescriptorSet set, VkDescriptorType descriptor_type, uint32_t binding, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info) const;
        void map_memory(VulkanBuffer buffer, VkDeviceSize size, void** data) const;
        void unmap_memory(VulkanBuffer buffer) const;

        void destroy_image_view(VkImageView image_view) const;
        void destroy_swapchain(VkSwapchainKHR swapchain) const;
        void destroy_buffer(VulkanBuffer buffer) const;
        void destroy_descriptor_pool(VkDescriptorPool pool) const;
    };
}