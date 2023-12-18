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
        VkImageAspectFlags aspect_mask;
    };

    class VulkanCommandEncoder {
        VkCommandBuffer m_buffer;

    public:
        explicit VulkanCommandEncoder(VkCommandBuffer buffer): m_buffer(buffer) {}

        void begin() const;
        void transition_image(VkImage image, const ImageTransitionDescriptor& descriptor) const;
        void start_rendering(VkRect2D render_area, const std::vector<VkRenderingAttachmentInfo>& attachments, const VkRenderingAttachmentInfo* depth_attachment, const VkRenderingAttachmentInfo* stencil_attachment) const;
        void set_viewport(uint32_t width, uint32_t height) const;
        void set_scissor(uint32_t width, uint32_t height) const;
        void bind_pipeline(VkPipeline pipeline) const;
        void bind_descriptor_set(VkPipelineLayout pipeline_layout, uint32_t set, const std::vector<VkDescriptorSet>& sets, const std::vector<uint32_t>& dynamic_offsets) const;
        void bind_vertex_buffer(VkBuffer buffer, VkDeviceSize offset) const;
        void bind_index_buffer(VkBuffer buffer) const;
        void draw(uint32_t vertex_count) const;
        void draw_indexed(uint32_t index_count) const;
        void end_rendering() const;
        [[nodiscard]] VkCommandBuffer finish() const;
    };
}