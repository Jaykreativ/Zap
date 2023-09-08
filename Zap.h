#pragma once

#include "VulkanFramework.h"

//TODO add standart renderer for windows with no renderer

namespace Zap {
    namespace GlobalSettings {
        VkFormat getColorFormat();
    }

    void init(const char* applicationName);

    void terminate();
}