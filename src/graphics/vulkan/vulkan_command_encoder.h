#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>

namespace Posideon {
    struct ImageTransitionDescriptor {
        VkAccessFlags src_access_mask;
        VkAccessFlags dst_access_mask;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        VkPipelineStageFlags src_stage_mask;
        VkPipelineStageFlags dst_stage_mask;
    };

    class VulkanCommandEncoder {
        VkCommandBuffer m_buffer;

    public:
        explicit VulkanCommandEncoder(VkCommandBuffer buffer): m_buffer(buffer) {}

        void begin() const;
        void transition_image(VkImage image, const ImageTransitionDescriptor& descriptor) const;
        void start_rendering(VkRect2D render_area, const std::vector<VkRenderingAttachmentInfo>& attachments) const;
        void set_viewport(uint32_t width, uint32_t height) const;
        void set_scissor(uint32_t width, uint32_t height) const;
        void end_rendering() const;
        VkCommandBuffer finish() const;
    };
}