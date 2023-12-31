#pragma once

#include "defines.h"

#include <vulkan/vulkan.hpp>

namespace Posideon {
    struct GraphicsPipelineBuilder {
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        VkPipelineInputAssemblyStateCreateInfo input_assembly;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayout pipeline_layout;
        VkPipelineDepthStencilStateCreateInfo depth_stencil;
        VkPipelineRenderingCreateInfo render_info;
        VkFormat color_attachment_format;
        VkPipelineViewportStateCreateInfo viewport_state;
        VkPipelineColorBlendStateCreateInfo color_blending;
        VkPipelineVertexInputStateCreateInfo vertex_input;
        VkPipelineDynamicStateCreateInfo dynamic_state_info;

        std::vector<VkDynamicState> dynamic_state;

        GraphicsPipelineBuilder() { clear(); }

        void clear();
        VkGraphicsPipelineCreateInfo build();

        void set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader);
        void set_input_topology(VkPrimitiveTopology topology);
        void set_polygon_mode(VkPolygonMode mode);
        void set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);
        void set_multisampling_none();
        void disable_blending();
        void set_color_attachment_format(VkFormat format);
        void set_depth_format(VkFormat format);
        void disable_depth_test();
        void enable_depth_test(bool depth_write_enable, VkCompareOp op);
    };
}
