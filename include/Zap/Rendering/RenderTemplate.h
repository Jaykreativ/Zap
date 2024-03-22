#pragma once

#include "Zap/Zap.h"

namespace Zap {
    class RenderTemplate {
    public:
        void disable() { m_isEnabled = false; }

        void enable() { m_isEnabled = true; }

        virtual void setRenderTarget(Image* target) = 0;

        virtual void setDefaultRenderTarget() = 0;

        virtual Image* getRenderTarget() = 0;

    private:
        bool m_isEnabled = true;

        virtual void onRendererInit() = 0;

        virtual void destroy() = 0;

        virtual void beforeRender() = 0;

        virtual void afterRender() = 0;

        virtual void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex) = 0;

        virtual void onWindowResize(int width, int height) = 0;

        friend class Renderer;
    };
}