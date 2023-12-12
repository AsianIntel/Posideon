#pragma once

#include "defines.h"
#include <vulkan/vulkan.hpp>

namespace Posideon {
    VkInstance init_vulkan_instance();
    VkDebugUtilsMessengerEXT init_debug_messenger(VkInstance instance);
}