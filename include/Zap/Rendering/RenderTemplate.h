#pragma once

#include "Zap/Zap.h"

namespace Zap {
    class RenderTemplate {
    private:
        virtual void init() = 0;

        virtual void destroy() = 0;

        virtual void recordCommands(const vk::CommandBuffer* cmd, uint32_t imageIndex) = 0;

        friend class Renderer;
    };
}