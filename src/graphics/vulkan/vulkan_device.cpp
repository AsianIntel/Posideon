#include "vulkan_device.h"

namespace Posideon {
    std::optional<uint32_t> VulkanDevice::get_memory_type_index(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        for (uint32_t i = 0; i < m_physicalDevice.device_memory_properties.memoryTypeCount; i++) {
            if (typeFilter & (1 << i) && (m_physicalDevice.device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        return {};
    }

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

    VkDeviceAddress VulkanDevice::get_buffer_address(const VulkanBuffer& buffer) const {
        const VkBufferDeviceAddressInfo device_address_info {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer.buffer
        };
        const VkDeviceAddress address = vkGetBufferDeviceAddress(m_device, &device_address_info);
        return address;
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

    VkFence VulkanDevice::create_fence(bool signaled) const {
        VkFenceCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags(0),
        };

        VkFence fence;
        VkResult res = vkCreateFence(m_device, &create_info, nullptr, &fence);
        POSIDEON_ASSERT(res == VK_SUCCESS);
        return fence;
    }

    VkPipelineLayout VulkanDevice::create_pipeline_layout(const std::vector<VkDescriptorSetLayout>& set_layouts, const std::vector<VkPushConstantRange>& push_constants) const {
        const VkPipelineLayoutCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(push_constants.size()),
            .pPushConstantRanges = push_constants.data()
        };

        VkPipelineLayout pipeline_layout;
        VkResult res = vkCreatePipelineLayout(m_device, &create_info, nullptr, &pipeline_layout);
        POSIDEON_ASSERT(res == VK_SUCCESS);

        return pipeline_layout;
    }

    VkPipeline VulkanDevice::create_graphics_pipeline(const VkGraphicsPipelineCreateInfo& descriptor) const {
        VkPipeline pipeline;
        const VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &descriptor, nullptr, &pipeline);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        return pipeline;
    }

    VkPipeline VulkanDevice::create_compute_pipeline(const ComputePipelineDescriptor& descriptor) const {
        const VkComputePipelineCreateInfo pipeline_create_info {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = descriptor.shader_stage,
            .layout = descriptor.layout,
        };

        VkPipeline pipeline;
        const VkResult res = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        return pipeline;
    }

    VkShaderModule VulkanDevice::create_shader_module(const std::vector<char>& code) const {
        VkShaderModuleCreateInfo create_info{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };

        VkShaderModule shader_module;
        VkResult res = vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module);
        POSIDEON_ASSERT(res == VK_SUCCESS);

        return shader_module;
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

    VkResult VulkanDevice::wait_for_fence(VkFence fence) {
        return vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    }

    VkResult VulkanDevice::reset_fence(VkFence fence) {
        return vkResetFences(m_device, 1, &fence);
    }

    void VulkanDevice::destroy_swapchain(VkSwapchainKHR swapchain) const {
        vkDestroySwapchainKHR(m_device, swapchain, nullptr);
    }

    VkDescriptorPool VulkanDevice::create_descriptor_pool(const std::vector<VkDescriptorPoolSize> &pool_sizes) const {
        const VkDescriptorPoolCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 100,
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data()
        };
        VkDescriptorPool pool;
        VkResult res = vkCreateDescriptorPool(m_device, &create_info, nullptr, &pool);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return pool;
    }

    VkDescriptorSetLayout VulkanDevice::create_descriptor_set_layout(const std::vector<VkDescriptorSetLayoutBinding> &bindings) const {
        const VkDescriptorSetLayoutCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };
        VkDescriptorSetLayout layout;
        VkResult res = vkCreateDescriptorSetLayout(m_device, &create_info, nullptr, &layout);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return layout;
    }

    VulkanImage VulkanDevice::create_image(const ImageDescriptor &descriptor) const {
        const VkImageCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = descriptor.image_type,
            .format = descriptor.format,
            .extent = VkExtent3D {
                .width = descriptor.width,
                .height = descriptor.height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = descriptor.usage,
            .initialLayout = descriptor.initial_layout,
        };

        constexpr VmaAllocationCreateInfo image_allocation_info {
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        };

        VkImage image;
        VmaAllocation allocation;
        vmaCreateImage(m_allocator, &create_info, &image_allocation_info, &image, &allocation, nullptr);

        VkImageView image_view = create_image_view(image, {
            .image_view_type = descriptor.image_view_type,
            .format = descriptor.format,
            .aspect_mask = descriptor.aspect_mask
        });

        return { image, image_view, allocation, create_info.extent, descriptor.format };
    }

    VkImageView VulkanDevice::create_image_view(VkImage image, const ImageViewDescriptor &descriptor) const {
        VkImageViewCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = descriptor.image_view_type,
            .format = descriptor.format,
            .subresourceRange = {
                .aspectMask = descriptor.aspect_mask,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VkImageView image_view;
        VkResult res = vkCreateImageView(m_device, &create_info, nullptr, &image_view);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        return image_view;
    }

    VulkanBuffer VulkanDevice::create_buffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage) const {
        VkBufferCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = alloc_size,
            .usage = usage
        };

        VmaAllocationCreateInfo alloc_info {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = memory_usage,
        };

        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;
        const VkResult res = vmaCreateBuffer(m_allocator, &create_info, &alloc_info, &buffer, &allocation, &allocation_info);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        return { buffer, allocation, allocation_info };
    }
    
    std::vector<VkDescriptorSet> VulkanDevice::allocate_descriptor_sets(VkDescriptorPool descriptor_pool, const std::vector<VkDescriptorSetLayout> &descriptor_layouts) const {
        VkDescriptorSetAllocateInfo alloc_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptor_pool,
            .descriptorSetCount = static_cast<uint32_t>(descriptor_layouts.size()),
            .pSetLayouts = descriptor_layouts.data(),
        };
        std::vector<VkDescriptorSet> descriptor_sets(descriptor_layouts.size());
        VkResult res = vkAllocateDescriptorSets(m_device, &alloc_info, descriptor_sets.data());
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return descriptor_sets;
    }

    void VulkanDevice::update_descriptor_sets(VkDescriptorSet set, VkDescriptorType descriptor_type, uint32_t binding, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info) const {
        const VkWriteDescriptorSet write_info {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = descriptor_type,
            .pImageInfo = image_info,
            .pBufferInfo = buffer_info,
        };
        vkUpdateDescriptorSets(m_device, 1, &write_info, 0, nullptr);
    }

    void VulkanDevice::map_memory(VulkanBuffer buffer, VkDeviceSize size, void** data) const {
        //vkMapMemory(m_device, buffer.memory, 0, size, 0, data);
    }

    void VulkanDevice::unmap_memory(VulkanBuffer buffer) const {
        //vkUnmapMemory(m_device, buffer.memory);
    }

    void VulkanDevice::destroy_buffer(VulkanBuffer buffer) const {
        vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
    }

    void VulkanDevice::reset_descriptor_pool(VkDescriptorPool pool) const {
        vkResetDescriptorPool(m_device, pool, 0);   
    }

    void VulkanDevice::destroy_descriptor_pool(VkDescriptorPool pool) const {
        vkResetDescriptorPool(m_device, pool, 0);   
    }

    void DescriptorAllocator::init_pool(const VulkanDevice& device, uint32_t max_sets, const std::vector<PoolSizeRatio>& pool_ratios) {
        std::vector<VkDescriptorPoolSize> pool_sizes;
        for (const PoolSizeRatio& ratio: pool_ratios) {
            pool_sizes.emplace_back(VkDescriptorPoolSize {
                .type = ratio.type,
                .descriptorCount = static_cast<uint32_t>(ratio.ratio * max_sets)
            });
        }

        pool = device.create_descriptor_pool(pool_sizes);
    }

    void DescriptorAllocator::clear_descriptors(const VulkanDevice& device) const {
        device.reset_descriptor_pool(pool);
    }

    void DescriptorAllocator::destroy_pool(const VulkanDevice& device) const {
        device.destroy_descriptor_pool(pool);
    }

    VkDescriptorSet DescriptorAllocator::allocate(const VulkanDevice& device, VkDescriptorSetLayout layout) const {
        return device.allocate_descriptor_sets(pool, { layout })[0];   
    }
}