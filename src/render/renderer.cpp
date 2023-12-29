#include "renderer.h"

#include "graphics/vulkan/vulkan_command_encoder.h"
#include "graphics/vulkan/vulkan_instance.h"

namespace Posideon {
    bool check_physical_device(VulkanPhysicalDevice& device, VkSurfaceKHR surface);

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

        const VulkanDevice vulkan_device(physical_device, device);
        VkQueue graphics_queue = vulkan_device.get_queue();

        Renderer renderer {
            .width = width,
            .height = height,
            .instance = instance,
            .surface = surface,
            .physical_device = physical_device,
            .device = vulkan_device,
            .queue = graphics_queue
        };

        renderer.create_swapchain();
        renderer.create_sync_structures();
        renderer.create_command_structures();

        return renderer;
    }

    void Renderer::create_swapchain() {
        const VkSurfaceCapabilitiesKHR surface_capabilities = device.get_physical_device_surface_capabilities(surface);

        uint32_t swapchain_image_count = surface_capabilities.minImageCount + 1;
        VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkExtent2D swapchain_extent = surface_capabilities.currentExtent;
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkSwapchainCreateInfoKHR swapchain_create_info {
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

    void Renderer::render() {
        device.wait_for_fence(get_current_frame().render_fence);
        device.reset_fence(get_current_frame().render_fence);

        uint32_t image_index = device.acquire_next_image(swapchain, get_current_frame().swapchain_semaphore);

        const VulkanCommandEncoder command_encoder(get_current_frame().command_buffer);
        command_encoder.reset();
        command_encoder.begin();

        command_encoder.transition_image(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        VkClearColorValue clear_value = { { 0.0f, 0.0f, abs(sin(frame_number / 120.f)), 1.0f } };
        VkImageSubresourceRange clear_range {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        vkCmdClearColorImage(command_encoder.m_buffer, swapchain_images[image_index], VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,  &clear_range);

        command_encoder.transition_image(swapchain_images[image_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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
        VkSubmitInfo2 submit {
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

        VkPresentInfoKHR present_info {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &get_current_frame().render_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index
        };
        res = vkQueuePresentKHR(queue, &present_info);

        frame_number++;
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
