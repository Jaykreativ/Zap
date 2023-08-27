#include "Zap.h"

#include "VulkanFramework.h"

void Zap::init() {
    if (!glfwInit()) std::runtime_error("Can't initialize GLFW");
}