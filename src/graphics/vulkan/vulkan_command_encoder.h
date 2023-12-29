#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>

namespace Posideon {
    struct VulkanCommandEncoder {
        VkCommandBuffer m_buffer;

        explicit VulkanCommandEncoder(VkCommandBuffer buffer): m_buffer(buffer) {}

        void reset() const;
        void begin() const;
        void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
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