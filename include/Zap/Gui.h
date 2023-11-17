#pragma once

#include "Zap/Renderer.h"

namespace Zap {
    class Gui : Renderer {
    public:
        Gui(Window& window);
        ~Gui();

        void init();

        void recordCommandBuffers(){}

        void render(uint32_t cam);

    private:
        VkDescriptorPool m_imguiPool;
    };
}