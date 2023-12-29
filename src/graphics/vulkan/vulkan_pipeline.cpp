#include "vulkan_pipeline.h"

namespace Posideon {
    void GraphicsPipelineBuilder::clear() {
        input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        color_blend_attachment = {};
        multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        pipeline_layout = VK_NULL_HANDLE;
        depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        shader_stages.clear();
        viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };
        color_blending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment
        };
        vertex_input = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };
        dynamic_state = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        dynamic_state_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_state.size()),
            .pDynamicStates = dynamic_state.data()
        };
    }

    VkGraphicsPipelineCreateInfo GraphicsPipelineBuilder::build() {
        const VkGraphicsPipelineCreateInfo pipeline_info {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &render_info,
            .stageCount = static_cast<uint32_t>(shader_stages.size()),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depth_stencil,
            .pColorBlendState = &color_blending,
            .pDynamicState = &dynamic_state_info,
            .layout = pipeline_layout,
        };
        return pipeline_info;
    }

    void GraphicsPipelineBuilder::set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader) {
        shader_stages.clear();
        shader_stages.emplace_back(VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex_shader,
            .pName = "main",
        });
        shader_stages.emplace_back(VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment_shader,
            .pName = "main",
        });
    }

    void GraphicsPipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
        input_assembly.topology = topology;
        input_assembly.primitiveRestartEnable = VK_FALSE;
    }

    void GraphicsPipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
        rasterizer.polygonMode = mode;
        rasterizer.lineWidth = 1.0f;
    }

    void GraphicsPipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face) {
        rasterizer.cullMode = cull_mode;
        rasterizer.frontFace = front_face;
    }

    void GraphicsPipelineBuilder::set_multisampling_none() {
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;
    }

    void GraphicsPipelineBuilder::disable_blending() {
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;
    }

    void GraphicsPipelineBuilder::set_color_attachment_format(VkFormat format) {
        color_attachment_format = format;
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachmentFormats = &color_attachment_format;
    }

    void GraphicsPipelineBuilder::set_depth_format(VkFormat format) {
        render_info.depthAttachmentFormat = format;   
    }

    void GraphicsPipelineBuilder::disable_depth_test() {
        depth_stencil.depthTestEnable = VK_FALSE;
        depth_stencil.depthWriteEnable = VK_FALSE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {};
        depth_stencil.back = {};
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.0f;
    }
}
