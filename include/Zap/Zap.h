#pragma once
#define GLM_FORCE_LEFT_HANDED
#include "VulkanFramework.h"

//TODO add standart renderer for windows with no renderer
namespace Zap {
    class Window;

    namespace objects {
        static std::vector<Window*> windows;
    }

    namespace GlobalSettings {
        VkFormat getColorFormat();
        VkFormat getDepthStencilFormat();
    }

    void init(const char* applicationName);

    void terminate();
}