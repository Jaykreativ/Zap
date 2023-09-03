#pragma once

#include "VulkanFramework.h"
#include "Window.h"
#include "glm.hpp"

namespace Zap {
    namespace GlobalSettings {
        VkFormat getColorFormat();
    }

    void init();
}