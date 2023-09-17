#pragma once

#include "VulkanFramework.h"

//TODO add standart renderer for windows with no renderer

namespace Zap {
    namespace GlobalSettings {
        VkFormat getColorFormat();
        VkFormat getDepthStencilFormat();
    }

    void init(const char* applicationName);

    void terminate();
}