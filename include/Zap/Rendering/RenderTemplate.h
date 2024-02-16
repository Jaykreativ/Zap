#pragma once

#include "Zap/Zap.h"

namespace Zap {
    class RenderTemplate {
    private:
        virtual void onRendererInit() = 0;

        virtual void destroy() = 0;

        virtual void beforeRender() = 0;

        virtual void afterRender() = 0;

        virtual void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex) = 0;

        virtual void onWindowResize(int width, int height) = 0;

        friend class Renderer;
    };
}