#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTemplate.h"

namespace Zap {
    class Gui {
    public:
        Gui(Renderer& renderer);
        ~Gui();

        void init();

        void render();

    private:

        bool m_isInit = false;

        Renderer& m_renderer;

        VkDescriptorPool m_imguiPool;

        vk::RenderPass m_renderPass = vk::RenderPass();
        uint32_t m_framebufferCount = 0;
        vk::Framebuffer* m_framebuffers = nullptr;
    };
}