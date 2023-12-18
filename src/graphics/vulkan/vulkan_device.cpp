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

    VkPipelineLayout VulkanDevice::create_pipeline_layout(const std::vector<VkDescriptorSetLayout>& set_layouts) const {
        const VkPipelineLayoutCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data()
        };

        VkPipelineLayout pipeline_layout;
        VkResult res = vkCreatePipelineLayout(m_device, &create_info, nullptr, &pipeline_layout);
        POSIDEON_ASSERT(res == VK_SUCCESS);

        return pipeline_layout;
    }

    VkPipeline VulkanDevice::create_pipeline(const PipelineDescriptor& descriptor) const {
        VkVertexInputBindingDescription vertex_input_binding{
            .binding = 0,
            .stride = descriptor.vertex_stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertex_input_binding,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(descriptor.vertex_input_attributes.size()),
            .pVertexAttributeDescriptions = descriptor.vertex_input_attributes.data()
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        };

        VkPipelineRasterizationStateCreateInfo rasterization{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = VkStencilOpState {
                    .failOp = VK_STENCIL_OP_KEEP,
                    .passOp = VK_STENCIL_OP_KEEP,
                    .compareOp = VK_COMPARE_OP_ALWAYS
            },
            .back = VkStencilOpState {
                .failOp = VK_STENCIL_OP_KEEP,
                .passOp = VK_STENCIL_OP_KEEP,
                .compareOp = VK_COMPARE_OP_ALWAYS
            },
        };

        VkPipelineColorBlendAttachmentState blend_attachment{
            .blendEnable = VK_FALSE,
            .colorWriteMask = 0xf
        };

        VkPipelineColorBlendStateCreateInfo color_blend_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &blend_attachment
        };

        std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        VkPipelineViewportStateCreateInfo viewport_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };

        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };

        VkPipelineRenderingCreateInfo pipeline_rendering{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &descriptor.swapchain_format,
            .depthAttachmentFormat = descriptor.depth_format,
            .stencilAttachmentFormat = descriptor.stencil_format,
        };

        VkGraphicsPipelineCreateInfo pipeline_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipeline_rendering,
            .stageCount = static_cast<uint32_t>(descriptor.shader_stages.size()),
            .pStages = descriptor.shader_stages.data(),
            .pVertexInputState = &vertex_input_state,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depth_stencil,
            .pColorBlendState = &color_blend_state,
            .pDynamicState = &dynamic_state,
            .layout = descriptor.layout,
        };

        VkPipeline pipeline;
        VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
        POSIDEON_ASSERT(res == VK_SUCCESS);

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
        VkImageCreateInfo create_info {
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
        VkImage image;
        VkResult res = vkCreateImage(m_device, &create_info, nullptr, &image);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(m_device, image, &memory_requirements);

        VkMemoryAllocateInfo alloc_info {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = get_memory_type_index(memory_requirements.memoryTypeBits, descriptor.memory_flags).value()
        };
        VkDeviceMemory memory;
        res = vkAllocateMemory(m_device, &alloc_info, nullptr, &memory);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        res = vkBindImageMemory(m_device, image, memory, 0);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        return { image, memory };
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

    void VulkanDevice::update_descriptor_sets(VkDescriptorSet set, VkDescriptorType descriptor_type, uint32_t binding, VkDescriptorBufferInfo* buffer_info) const {
        VkWriteDescriptorSet write_info {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .descriptorCount = 1,
            .descriptorType = descriptor_type,
            .pBufferInfo = buffer_info
        };
        vkUpdateDescriptorSets(m_device, 1, &write_info, 0, nullptr);
    }

    void VulkanDevice::map_memory(VulkanBuffer buffer, VkDeviceSize size, void** data) const {
        vkMapMemory(m_device, buffer.memory, 0, size, 0, data);
    }

    void VulkanDevice::unmap_memory(VulkanBuffer buffer) const {
        vkUnmapMemory(m_device, buffer.memory);
    }

    void VulkanDevice::destroy_buffer(VulkanBuffer buffer) const {
        vkFreeMemory(m_device, buffer.memory, nullptr);
        vkDestroyBuffer(m_device, buffer.buffer, nullptr);
    }
}