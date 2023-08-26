#include <iostream>

#include "VulkanRenderer.h"
#include "glm.hpp"

namespace renderer {
    GLFWwindow* window;

    uint32_t width = 1000;
    uint32_t height = 600;
    const char* TITLE = "Zap";

    std::vector<glm::vec3> vertexArray = {
        glm::vec3(0, 1, 2)
    };
}

int main()
{
    vkRenderer::Buffer vertexBuffer = vkRenderer::Buffer(renderer::vertexArray.size() * sizeof(glm::vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    if (!glfwInit()) std::runtime_error("Can't Init GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    renderer::window = glfwCreateWindow(renderer::width, renderer::height, renderer::TITLE, nullptr, nullptr);

    glfwShowWindow(renderer::window);
    
    initVulkan(renderer::window, renderer::width, renderer::height, renderer::TITLE);

    vertexBuffer.init(); vertexBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* rawData; vertexBuffer.map(&rawData);
    memcpy(rawData, renderer::vertexArray.data(), renderer::vertexArray.size() * sizeof(glm::vec3));
    vertexBuffer.unmap();

    system("pause");
}
