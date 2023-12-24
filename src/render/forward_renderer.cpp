#include "forward_renderer.h"
#include <iostream>
#include <fstream>
#include "graphics/vulkan/vulkan_command_encoder.h"

#undef min
#undef max

namespace Posideon {
    bool check_physical_device(VulkanPhysicalDevice& device, VkSurfaceKHR surface);

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        POSIDEON_ASSERT(file.is_open())

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    ForwardRenderer init_forward_renderer(HINSTANCE hinstance, HWND hwnd, flecs::world& world, uint32_t width, uint32_t height) {
        VkInstance instance = init_vulkan_instance();
        VkDebugUtilsMessengerEXT debug_messenger = init_debug_messenger(instance);

        const VkWin32SurfaceCreateInfoKHR surface_create_info {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = hinstance,
            .hwnd = hwnd
        };
        VkSurfaceKHR surface;
        VkResult res = vkCreateWin32SurfaceKHR(instance, &surface_create_info, nullptr, &surface);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        uint32_t gpu_count;
        vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices.data());

        VulkanPhysicalDevice physical_device(nullptr);
        for (auto& device: physical_devices) {
            VulkanPhysicalDevice p_device(device);
            if (check_physical_device(p_device, surface)) {
                physical_device = p_device;
                break;
            }
        }
        POSIDEON_ASSERT(physical_device.raw != nullptr)

        vkGetPhysicalDeviceProperties(physical_device.raw, &physical_device.device_properties);
        vkGetPhysicalDeviceFeatures(physical_device.raw, &physical_device.device_features);
        vkGetPhysicalDeviceMemoryProperties(physical_device.raw, &physical_device.device_memory_properties);

        constexpr float queue_priorities[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = physical_device.graphics_family_index,
            .queueCount = 1,
            .pQueuePriorities = queue_priorities,
        };
        std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
        VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
            .dynamicRendering = true,
        };
        const VkDeviceCreateInfo device_create_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &dynamic_rendering,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_info,
            .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data()
        };

        VkDevice device;
        res = vkCreateDevice(physical_device.raw, &device_create_info, nullptr, &device);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        const VulkanDevice vulkan_device(physical_device, device);
        VkQueue graphics_queue = vulkan_device.get_queue();

        VkFormat depth_format = VK_FORMAT_UNDEFINED;
        std::vector<VkFormat> depth_formats = {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM
        };
        VkFormatProperties format_properties;
        for (auto format: depth_formats) {
            vkGetPhysicalDeviceFormatProperties(physical_device.raw, format, &format_properties);
            if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                depth_format = format;
                break;
            }
        }
        POSIDEON_ASSERT(depth_format != VK_FORMAT_UNDEFINED)
        VulkanImage depth_image = vulkan_device.create_image(ImageDescriptor {
            .image_type = VK_IMAGE_TYPE_2D,
            .format = depth_format,
            .width = width,
            .height = height,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            .memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        });
        VkImageView depth_image_view = vulkan_device.create_image_view(depth_image.image, ImageViewDescriptor {
            .image_view_type = VK_IMAGE_VIEW_TYPE_2D,
            .format = depth_format,
            .aspect_mask = static_cast<VkImageAspectFlags>((depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT)
        });

        VkSemaphore image_available_semaphore = vulkan_device.create_semaphore();
        VkSemaphore rendering_finished_semaphore = vulkan_device.create_semaphore();
        VkFence fence = vulkan_device.create_fence(true);

        VkShaderModule vertex_shader = vulkan_device.create_shader_module(readFile("../../../assets/shaders/shader.vert.spv"));
        VkShaderModule fragment_shader = vulkan_device.create_shader_module(readFile("../../../assets/shaders/shader.frag.spv"));

        flecs::query<Mesh, Transform> mesh_query = world.query<Mesh, Transform>();
        flecs::query<Camera, Transform> view_query = world.query<Camera, Transform>();

        ForwardRenderer renderer {
            .instance = instance,
            .debug_messenger = debug_messenger,
            .surface = surface,
            .device = vulkan_device,
            .graphics_queue = graphics_queue,
            .image_available_semaphore = image_available_semaphore,
            .rendering_finished_semaphore = rendering_finished_semaphore,
            .fence = fence,
            .depth_format = depth_format,
            .depth_image = depth_image,
            .depth_image_view = depth_image_view,
            .width = width,
            .height = height,
            .vsync = false,
            .mesh_query = mesh_query,
            .view_query = view_query,
        };

        renderer.create_swapchain();
        renderer.allocate_command_buffers();
        renderer.create_descriptor_objects();

        VkDescriptorSetLayout model_set_layout = vulkan_device.create_descriptor_set_layout({
            VkDescriptorSetLayoutBinding {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
            }
        });
        VkDescriptorSet model_descriptor_set = vulkan_device.allocate_descriptor_sets(renderer.descriptor_pool, { model_set_layout })[0];
        VkDescriptorSetLayout view_set_layout = vulkan_device.create_descriptor_set_layout({
            VkDescriptorSetLayoutBinding {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
            }
        });
        VkDescriptorSet view_descriptor_set = vulkan_device.allocate_descriptor_sets(renderer.descriptor_pool, { view_set_layout })[0];

        VkPipelineLayout pipeline_layout = vulkan_device.create_pipeline_layout({ model_set_layout, view_set_layout });
        VkPipeline pipeline = vulkan_device.create_pipeline({
            .vertex_stride = sizeof(Vertex),
            .vertex_input_attributes = {
                VkVertexInputAttributeDescription {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = static_cast<uint32_t>(offsetof(Vertex, position))
                },
                VkVertexInputAttributeDescription {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .offset = static_cast<uint32_t>(offsetof(Vertex, color))
                },
            },
            .shader_stages = {
                VkPipelineShaderStageCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vertex_shader,
                    .pName = "main",
                },
                VkPipelineShaderStageCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = fragment_shader,
                    .pName = "main",
                }
            },
            .layout = pipeline_layout,
            .swapchain_format = renderer.swapchain_format,
            .depth_format = depth_format,
            .stencil_format = (depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) ? depth_format : VK_FORMAT_UNDEFINED
        });
        MeshPipeline mesh_pipeline {
            .model_descriptor_layout = model_set_layout,
            .model_set = model_descriptor_set,
            .view_descriptor_layout = view_set_layout,
            .view_set = view_descriptor_set,
            .pipeline_layout = pipeline_layout,
            .pipeline = pipeline
        };
        renderer.mesh_pipeline = mesh_pipeline;

        return renderer;
    }

    void ForwardRenderer::create_swapchain() {
        VkSwapchainKHR old_swapchain = swapchain;

        const VkSurfaceCapabilitiesKHR surface_capabilities = device.get_physical_device_surface_capabilities(surface);
        const std::vector<VkPresentModeKHR> present_modes = device.get_surface_present_modes(surface);

        // FIFO Present mode is guaranteed to exist per the spec
        VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        if (!vsync) {
            for (const auto present_mode: present_modes) {
                if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    swapchain_present_mode = present_mode;
                    break;
                }
            }
        }

        uint32_t swapchain_image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0 && swapchain_image_count > surface_capabilities.maxImageCount) {
            swapchain_image_count = surface_capabilities.maxImageCount;
        }

        VkSurfaceTransformFlagsKHR pre_transform;
        if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            pre_transform = surface_capabilities.currentTransform;
        }

        VkExtent2D swapchain_extent;
        if (surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            swapchain_extent.width = width;
            swapchain_extent.height = height;
        } else {
            swapchain_extent = surface_capabilities.currentExtent;
        }

        const std::vector<VkSurfaceFormatKHR> surface_formats = device.get_surface_formats(surface);
        VkColorSpaceKHR swapchain_color_space;
        bool format_found = false;
        for (auto&& surface_format: surface_formats) {
            if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM || surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
                swapchain_format = surface_format.format;
                swapchain_color_space = surface_format.colorSpace;
                format_found = true;
                break;
            }
        }
        if (!format_found) {
            swapchain_format = surface_formats[0].format;
            swapchain_color_space = surface_formats[0].colorSpace;
        }

        VkCompositeAlphaFlagsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (const auto& composite_alpha_flag: composite_alpha_flags) {
            if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flag) {
                composite_alpha = composite_alpha_flag;
                break;
            }
        }

        VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        VkSwapchainCreateInfoKHR swapchain_create_info {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = swapchain_image_count,
            .imageFormat = swapchain_format,
            .imageColorSpace = swapchain_color_space,
            .imageExtent = swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage = image_usage,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(pre_transform),
            .compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(composite_alpha),
            .presentMode = swapchain_present_mode,
            .clipped = true,
            .oldSwapchain = old_swapchain,
        };

        swapchain = device.create_swapchain(swapchain_create_info);

        if (old_swapchain != VK_NULL_HANDLE) {
            for (auto& swapchain_image_view : swapchain_image_views) {
                device.destroy_image_view(swapchain_image_view);
            }
            device.destroy_swapchain(old_swapchain);
        }

        swapchain_images = device.get_swapchain_images(swapchain);

        swapchain_image_views.resize(swapchain_images.size());
        for (uint32_t i = 0; i < swapchain_images.size(); i++) {
            swapchain_image_views[i] = device.create_image_view(swapchain_images[i], ImageViewDescriptor {
                .image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchain_format,
                .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
            });
        }
    }

    void ForwardRenderer::allocate_command_buffers() {
        command_pool = device.create_command_pool();
        command_buffers = device.allocate_command_buffers(command_pool, swapchain_image_views.size());
    }

    void ForwardRenderer::prepare_uniform_buffers() {
        mesh_query.iter([&](flecs::iter& it, Mesh* mesh, Transform* transform) {
            std::vector<glm::mat4> uniforms;
            for (auto i: it) {
                uniforms.push_back(transform->model);
            }
            size_t buffer_size = uniforms.size() * sizeof(glm::mat4);
            if (transform_dynamic.capacity < buffer_size) {
                if (transform_dynamic.buffer.buffer != nullptr) {
                    device.unmap_memory(transform_dynamic.buffer);
                    device.destroy_buffer(transform_dynamic.buffer);
                }
                transform_dynamic.capacity = buffer_size;
                transform_dynamic.buffer = device.create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, buffer_size);
                device.map_memory(transform_dynamic.buffer, buffer_size, &transform_dynamic.mapped);
                VkDescriptorBufferInfo buffer_info {
                    .buffer = transform_dynamic.buffer.buffer,
                    .offset = 0,
                    .range = buffer_size
                };
                device.update_descriptor_sets(mesh_pipeline.model_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0, &buffer_info);
            }

            memcpy(transform_dynamic.mapped, uniforms.data(), buffer_size);
        });

        view_query.iter([&](flecs::iter& it, Camera* camera, Transform* transform) {
            ExtractedView extracted_view {
                .projection = camera->projection,
                .view = transform->model,
            };
            if (view_buffer.buffer.buffer == nullptr) {
                view_buffer.buffer = device.create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ExtractedView));
                device.map_memory(view_buffer.buffer, sizeof(ExtractedView), &view_buffer.mapped);
                VkDescriptorBufferInfo buffer_info {
                    .buffer = view_buffer.buffer.buffer,
                    .offset = 0,
                    .range = sizeof(ExtractedView),
                };
                device.update_descriptor_sets(mesh_pipeline.view_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &buffer_info);
            }

            memcpy(view_buffer.mapped, &extracted_view, sizeof(ExtractedView));
        });
    }

    void ForwardRenderer::render() {
        device.wait_for_fence(fence);
        device.reset_fence(fence);

        prepare_uniform_buffers();

        uint32_t image_index = device.acquire_next_image(swapchain, image_available_semaphore);
        VulkanCommandEncoder command_encoder(command_buffers[image_index]);

        command_encoder.begin();

        command_encoder.transition_image(swapchain_images[image_index], {
            .dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
        });
        command_encoder.transition_image(depth_image.image, {
            .dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            .new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .aspect_mask = static_cast<VkImageAspectFlags>((depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT)
        });

        VkRenderingAttachmentInfo depth_attachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = depth_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = { .depthStencil = VkClearDepthStencilValue {1.0f, 0} }
        };
        VkRenderingAttachmentInfo* stencil_attachment = nullptr;
        if (depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) {
            stencil_attachment = &depth_attachment;
        }
        command_encoder.start_rendering(
            VkRect2D {
                .offset = VkOffset2D {0, 0},
                .extent = VkExtent2D {.width = width, .height = height}
            },
            {
                VkRenderingAttachmentInfo {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = swapchain_image_views[image_index],
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {.color = VkClearColorValue { 0.0f, 0.0f, 0.0f, 1.0f }}
                }
            },
            &depth_attachment,
            stencil_attachment
        );

        command_encoder.set_viewport(width, height);
        command_encoder.set_scissor(width, height);

        command_encoder.bind_pipeline(mesh_pipeline.pipeline);
        for (size_t i = 0; i < meshes.size(); i++) {
            command_encoder.bind_descriptor_set(mesh_pipeline.pipeline_layout, 0, { mesh_pipeline.model_set }, { static_cast<uint32_t>(i * sizeof(glm::mat4)) });
            command_encoder.bind_descriptor_set(mesh_pipeline.pipeline_layout, 1, { mesh_pipeline.view_set }, {});
            if (meshes[i].index_buffer.has_value()) {
                command_encoder.bind_index_buffer(meshes[i].index_buffer.value().buffer);
                command_encoder.bind_vertex_buffer(meshes[i].vertex_buffer.buffer, 0);
                command_encoder.draw_indexed(meshes[i].index_count);
            } else {
                command_encoder.bind_vertex_buffer(meshes[i].vertex_buffer.buffer, 0);
                command_encoder.draw(meshes[i].vertex_count);
            }
        }

        command_encoder.end_rendering();
        command_encoder.transition_image(swapchain_images[image_index], {
            .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
        });

        VkCommandBuffer command_buffer = command_encoder.finish();

        VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_available_semaphore,
            .pWaitDstStageMask = &wait_stage_mask,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &rendering_finished_semaphore,
        };
        VkResult res = vkQueueSubmit(graphics_queue, 1, &submit_info, fence);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        VkPresentInfoKHR present_info {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &rendering_finished_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index
        };
        res = vkQueuePresentKHR(graphics_queue, &present_info);
        POSIDEON_ASSERT(res == VK_SUCCESS)
    }

    void ForwardRenderer::create_descriptor_objects() {
        descriptor_pool = device.create_descriptor_pool({
            VkDescriptorPoolSize {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = 1
            },
            VkDescriptorPoolSize {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 10
            }
        });
    }

    void ForwardRenderer::add_gpu_mesh(Mesh& mesh) {
        std::optional<VulkanBuffer> index_buffer;
        if (!mesh.indices.empty()) {
            index_buffer = device.create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mesh.indices.size(), mesh.indices.data());
        }
        GPUMesh gpu_mesh {
            .vertex_buffer = device.create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh.vertices.size(), mesh.vertices.data()),
            .index_buffer = index_buffer,
            .vertex_count = static_cast<uint32_t>(mesh.vertices.size()),
            .index_count = static_cast<uint32_t>(mesh.indices.size()),
        };
        meshes.emplace_back(gpu_mesh);
    }

    bool check_physical_device(VulkanPhysicalDevice& device, VkSurfaceKHR surface) {
        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device.raw, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device.raw, &queue_family_count, queue_families.data());

        for (uint32_t i = 0; i < queue_family_count; i++) {
            VkBool32 present_support;
            vkGetPhysicalDeviceSurfaceSupportKHR(device.raw, i, surface, &present_support);
            if (queue_families[i].queueCount > 0 && (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                if (present_support) {
                    device.graphics_family_index = i;
                    return true;
                }
            }
        }
        return false;
    }
}