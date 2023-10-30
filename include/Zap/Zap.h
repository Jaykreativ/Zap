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

    class Base {
    public:
        Base(const char* applicationName);

        ~Base();

        void init();

        void terminate();

    private:
        bool m_isInit;

        const char* m_applicationName;

        static bool m_exists;
    };

}