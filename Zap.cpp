#include "Zap.h"

void Zap::init()
{
    if (!glfwInit())
        std::runtime_error("Can't initialize GLFW");
}