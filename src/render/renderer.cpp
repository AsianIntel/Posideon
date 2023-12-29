#include "renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <fstream>

#include "graphics/vulkan/vulkan_command_encoder.h"
#include "graphics/vulkan/vulkan_instance.h"
#include "graphics/vulkan/vulkan_pipeline.h"

namespace Posideon {
    bool check_physical_device(VulkanPhysicalDevice& device, VkSurfaceKHR surface);
    std::vector<char> readFile(const std::string& filename);

    Renderer init_renderer(uint32_t width, uint32_t height, Win32Window* window) {
        VkInstance instance = init_vulkan_instance();
        VkDebugUtilsMessengerEXT debug_messenger = init_debug_messenger(instance);

        const VkWin32SurfaceCreateInfoKHR surface_create_info {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = window->hInstance,
            .hwnd = window->m_hwnd
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
        std::vector device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkPhysicalDeviceVulkan13Features features13 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = true,
            .dynamicRendering = true,
        };
        VkPhysicalDeviceVulkan12Features features12 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features13,
            .descriptorIndexing = true,
            .bufferDeviceAddress = true,
        };
        const VkDeviceCreateInfo device_create_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features12,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_info,
            .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data()
        };

        VkDevice device;
        res = vkCreateDevice(physical_device.raw, &device_create_info, nullptr, &device);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        VmaAllocatorCreateInfo allocator_create_info {
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physical_device.raw,
            .device = device,
            .instance = instance,
        };
        VmaAllocator allocator;
        res = vmaCreateAllocator(&allocator_create_info, &allocator);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        const VulkanDevice vulkan_device(physical_device, device, allocator);
        VkQueue graphics_queue = vulkan_device.get_queue();

        Renderer renderer {
            .width = width,
            .height = height,
            .instance = instance,
            .debug_messenger = debug_messenger,
            .surface = surface,
            .physical_device = physical_device,
            .device = vulkan_device,
            .queue = graphics_queue
        };

        renderer.create_swapchain();
        renderer.create_sync_structures();
        renderer.create_command_structures();
        renderer.create_descriptors();
        renderer.create_pipelines();

        return renderer;
    }

    void Renderer::create_swapchain() {
        const VkSurfaceCapabilitiesKHR surface_capabilities = device.get_physical_device_surface_capabilities(surface);

        const uint32_t swapchain_image_count = surface_capabilities.minImageCount + 1;
        constexpr VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
        constexpr VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchain_extent = surface_capabilities.currentExtent;
        constexpr VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        constexpr VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        const VkSwapchainCreateInfoKHR swapchain_create_info {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = swapchain_image_count,
            .imageFormat = swapchain_format,
            .imageColorSpace = color_space,
            .imageExtent = swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage = image_usage,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = surface_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = true,
            .oldSwapchain = nullptr,
        };
        swapchain = device.create_swapchain(swapchain_create_info);
        swapchain_images = device.get_swapchain_images(swapchain);

        swapchain_image_views.resize(swapchain_images.size());
        for (uint32_t i = 0; i < swapchain_images.size(); i++) {
            swapchain_image_views[i] = device.create_image_view(swapchain_images[i], ImageViewDescriptor {
                .image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchain_format,
                .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
            });
        }

        constexpr auto draw_image_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        draw_image = device.create_image({
            .image_type = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .width = width,
            .height = height,
            .usage = static_cast<VkImageUsageFlags>(draw_image_usage),
            .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            .image_view_type = VK_IMAGE_VIEW_TYPE_2D,
            .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
        });
    }

    void Renderer::create_command_structures() {
        for (auto& frame : frames) {
            frame.command_pool = device.create_command_pool();
            frame.command_buffer = device.allocate_command_buffers(frame.command_pool, 1)[0];
        }
    }

    void Renderer::create_sync_structures() {
        for (auto& frame : frames) {
            frame.render_fence = device.create_fence(true);
            frame.render_semaphore = device.create_semaphore();
            frame.swapchain_semaphore = device.create_semaphore();
        }
    }

    void Renderer::create_descriptors() {
        std::vector<DescriptorAllocator::PoolSizeRatio> sizes {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };
        global_descriptor_allocator.init_pool(device, 10, sizes);
        draw_image_set_layout = device.create_descriptor_set_layout({
            VkDescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            }
        });
        draw_image_set = global_descriptor_allocator.allocate(device, draw_image_set_layout);

        VkDescriptorImageInfo image_info {
            .imageView = draw_image.image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        device.update_descriptor_sets(draw_image_set, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, nullptr, &image_info);
    }

    void Renderer::create_pipelines() {
        create_background_pipelines();
        create_triangle_pipeline();
    }

    void Renderer::create_background_pipelines() {
        gradient_layout = device.create_pipeline_layout({ draw_image_set_layout });

        const std::vector<char> gradient_shader_code = readFile("../assets/shaders/gradient.comp.spv");
        const VkShaderModule gradient_shader = device.create_shader_module(gradient_shader_code);

        const VkPipelineShaderStageCreateInfo shader_stage {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = gradient_shader,
            .pName = "main"
        };
        gradient_pipeline = device.create_compute_pipeline({
            .shader_stage = shader_stage,
            .layout = gradient_layout,
        });
    }

    void Renderer::create_triangle_pipeline() {
        const std::vector<char> vertex_shader_code = readFile("../assets/shaders/shader.vert.spv");
        const VkShaderModule vertex_shader = device.create_shader_module(vertex_shader_code);

        const std::vector<char> fragment_shader_code = readFile("../assets/shaders/shader.frag.spv");
        const VkShaderModule fragment_shader = device.create_shader_module(fragment_shader_code);

        triangle_pipeline_layout = device.create_pipeline_layout({});
        GraphicsPipelineBuilder pipeline_builder;
        pipeline_builder.pipeline_layout = triangle_pipeline_layout;
        pipeline_builder.set_shaders(vertex_shader, fragment_shader);
        pipeline_builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        pipeline_builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
        pipeline_builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
        pipeline_builder.set_multisampling_none();
        pipeline_builder.disable_blending();
        pipeline_builder.disable_depth_test();
        pipeline_builder.set_color_attachment_format(draw_image.format);
        pipeline_builder.set_depth_format(VK_FORMAT_UNDEFINED);
        triangle_pipeline = device.create_graphics_pipeline(pipeline_builder.build());
    }
    
    void Renderer::render() {
        device.wait_for_fence(get_current_frame().render_fence);
        device.reset_fence(get_current_frame().render_fence);

        uint32_t image_index = device.acquire_next_image(swapchain, get_current_frame().swapchain_semaphore);

        const VulkanCommandEncoder command_encoder(get_current_frame().command_buffer);
        command_encoder.reset();
        command_encoder.begin();

        VkExtent2D draw_extent { draw_image.extent.width, draw_image.extent.height };

        command_encoder.transition_image(draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        draw_background(command_encoder);

        command_encoder.transition_image(draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        draw_geometry(command_encoder);
        
        command_encoder.transition_image(draw_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        command_encoder.transition_image(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        command_encoder.copy_image_to_image(draw_image.image, swapchain_images[image_index], draw_extent, swapchain_extent);

        command_encoder.transition_image(swapchain_images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        
        VkCommandBuffer command_buffer = command_encoder.finish();

        VkCommandBufferSubmitInfo command_submit_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = command_buffer,
            .deviceMask = 0
        };
        VkSemaphoreSubmitInfo wait_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = get_current_frame().swapchain_semaphore,
            .value = 1,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
            .deviceIndex = 0,
        };
        VkSemaphoreSubmitInfo signal_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = get_current_frame().render_semaphore,
            .value = 1,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
            .deviceIndex = 0,
        };
        const VkSubmitInfo2 submit {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &wait_info,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &command_submit_info,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signal_info,
        };
        VkResult res = vkQueueSubmit2(queue, 1, &submit, get_current_frame().render_fence);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        const VkPresentInfoKHR present_info {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &get_current_frame().render_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index
        };
        res = vkQueuePresentKHR(queue, &present_info);
        POSIDEON_ASSERT(res == VK_SUCCESS)

        frame_number++;
    }

    void Renderer::draw_background(const VulkanCommandEncoder& encoder) const {
        encoder.bind_pipeline(VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline);
        encoder.bind_descriptor_set(VK_PIPELINE_BIND_POINT_COMPUTE, gradient_layout, 0, { draw_image_set }, {});
        encoder.dispatch(std::ceil(draw_image.extent.width / 16.0f), std::ceil(draw_image.extent.height / 16.0f));
    }

    void Renderer::draw_geometry(const VulkanCommandEncoder& encoder) const {
        const VkRect2D draw_extent { 0, 0, draw_image.extent.width, draw_image.extent.height };
        VkRenderingAttachmentInfo color_attachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = draw_image.image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        encoder.start_rendering(draw_extent, { color_attachment }, nullptr, nullptr);

        encoder.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipeline);
        encoder.set_viewport(draw_image.extent.width, draw_image.extent.height);
        encoder.set_scissor(draw_image.extent.width, draw_image.extent.height);

        encoder.draw(3);
        encoder.end_rendering();
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

    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}
