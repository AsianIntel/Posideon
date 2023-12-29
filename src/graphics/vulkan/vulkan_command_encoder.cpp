#include "vulkan_command_encoder.h"

namespace Posideon {
    void VulkanCommandEncoder::reset() const {
        vkResetCommandBuffer(m_buffer, 0);
    }

    void VulkanCommandEncoder::begin() const {
        constexpr VkCommandBufferBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        const VkResult res = vkBeginCommandBuffer(m_buffer, &begin_info);
        POSIDEON_ASSERT(res == VK_SUCCESS)
    }

    void VulkanCommandEncoder::transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const {
        const auto aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageMemoryBarrier2 image_barrier {
           .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
           .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
           .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
           .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
           .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
           .oldLayout = current_layout,
           .newLayout = new_layout,
            .image = image,
           .subresourceRange = {
               .aspectMask = static_cast<VkImageAspectFlags>(aspect_mask),
               .baseMipLevel = 0,
               .levelCount = 1,
               .baseArrayLayer = 0,
               .layerCount = 1,
           },
        };

        const VkDependencyInfo dependency_info {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_barrier,
        };

        vkCmdPipelineBarrier2(m_buffer, &dependency_info);
    }

    void VulkanCommandEncoder::start_rendering(VkRect2D render_area, const std::vector<VkRenderingAttachmentInfo>& attachments, const VkRenderingAttachmentInfo* depth_attachment, const VkRenderingAttachmentInfo* stencil_attachment) const {
        const VkRenderingInfo rendering_info {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = render_area,
            .layerCount = 1,
            .colorAttachmentCount = static_cast<uint32_t>(attachments.size()),
            .pColorAttachments = attachments.data(),
            .pDepthAttachment = depth_attachment,
            .pStencilAttachment = stencil_attachment
        };

        vkCmdBeginRendering(m_buffer, &rendering_info);
    }

    void VulkanCommandEncoder::set_viewport(uint32_t width, uint32_t height) const {
        const VkViewport viewport = {
            .width = static_cast<float>(width),
            .height = static_cast<float>(height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(m_buffer, 0, 1, &viewport);
    }

    void VulkanCommandEncoder::set_scissor(uint32_t width, uint32_t height) const {
        const VkRect2D scissor = {
            .offset = {
                .x = 0,
                .y = 0
            },
            .extent = {
                .width = width,
                .height = height
            }
        };
        vkCmdSetScissor(m_buffer, 0, 1, &scissor);
    }

    void VulkanCommandEncoder::bind_pipeline(VkPipeline pipeline) const {
        vkCmdBindPipeline(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void VulkanCommandEncoder::bind_descriptor_set(VkPipelineLayout pipeline_layout, uint32_t set, const std::vector<VkDescriptorSet> &sets, const std::vector<uint32_t>& dynamic_offsets) const {
        vkCmdBindDescriptorSets(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, set, static_cast<uint32_t>(sets.size()), sets.data(), static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
    }

    void VulkanCommandEncoder::bind_vertex_buffer(VkBuffer buffer, VkDeviceSize offset) const {
        vkCmdBindVertexBuffers(m_buffer, 0, 1, &buffer, &offset);
    }

    void VulkanCommandEncoder::bind_index_buffer(VkBuffer buffer) const {
        vkCmdBindIndexBuffer(m_buffer, buffer, 0, VK_INDEX_TYPE_UINT16);
    }

    void VulkanCommandEncoder::draw(uint32_t vertex_count) const {
        vkCmdDraw(m_buffer, vertex_count, 1, 0, 0);
    }

    void VulkanCommandEncoder::draw_indexed(uint32_t index_count) const {
        vkCmdDrawIndexed(m_buffer, index_count, 1, 0, 0, 0);
    }

    void VulkanCommandEncoder::end_rendering() const {
        vkCmdEndRendering(m_buffer);
    }

    VkCommandBuffer VulkanCommandEncoder::finish() const {
        const VkResult res = vkEndCommandBuffer(m_buffer);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return m_buffer;
    }
}