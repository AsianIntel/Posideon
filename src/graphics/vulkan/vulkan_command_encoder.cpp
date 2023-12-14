#include "vulkan_command_encoder.h"

namespace Posideon {
    void VulkanCommandEncoder::begin() const {
        constexpr VkCommandBufferBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        const VkResult res = vkBeginCommandBuffer(m_buffer, &begin_info);
        POSIDEON_ASSERT(res == VK_SUCCESS)
    }

    void VulkanCommandEncoder::transition_image(VkImage image, const  ImageTransitionDescriptor& descriptor) const {
        const VkImageMemoryBarrier image_memory_barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = descriptor.src_access_mask,
            .dstAccessMask = descriptor.dst_access_mask,
            .oldLayout = descriptor.old_layout,
            .newLayout = descriptor.new_layout,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        vkCmdPipelineBarrier(m_buffer, descriptor.src_stage_mask, descriptor.dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    }

    void VulkanCommandEncoder::start_rendering(VkRect2D render_area, const std::vector<VkRenderingAttachmentInfo>& attachments) const {
        const VkRenderingInfo rendering_info {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = render_area,
            .layerCount = 1,
            .colorAttachmentCount = static_cast<uint32_t>(attachments.size()),
            .pColorAttachments = attachments.data()
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

    void VulkanCommandEncoder::bind_vertex_buffer(VkBuffer buffer, VkDeviceSize offset) const {
        vkCmdBindVertexBuffers(m_buffer, 0, 1, &buffer, &offset);
    }

    void VulkanCommandEncoder::draw(uint32_t vertex_count) const {
        vkCmdDraw(m_buffer, 3, 1, 0, 0);
    }

    void VulkanCommandEncoder::end_rendering() const {
        vkCmdEndRendering(m_buffer);
    }

    VkCommandBuffer VulkanCommandEncoder::finish() const {
        const VkResult res = vkEndCommandBuffer(m_buffer);
        POSIDEON_ASSERT(res == VK_SUCCESS)
        return m_buffer;
    }

    void VulkanCommandEncoder::bind_descriptor_set(VkPipelineLayout pipeline_layout, const std::vector<VkDescriptorSet> &sets) const {
        vkCmdBindDescriptorSets(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
    }
}