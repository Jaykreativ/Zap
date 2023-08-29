#pragma once

#include "Zap.h"
#include "VulkanFramework.h";

namespace Zap
{
    class Window
    {
    public:
        Window(uint32_t width, uint32_t height, std::string title);
        ~Window();

        void init();

        bool shouldClose();

        void show();

        static void pollEvents();

    private:
        bool m_isInit = false;

        uint32_t m_width;
        uint32_t m_height;
        std::string m_title;

        GLFWwindow *m_window;
    };
}
