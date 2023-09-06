#include "Zap.h"

namespace Zap {
	namespace GlobalSettings {
		VkFormat colorFormat;
		VkFormat getColorFormat() {
			return colorFormat;
		}
	}

	vk::DescriptorPool descriptorPool = vk::DescriptorPool();
}

vk::DescriptorPool& Zap::getDescriptorPool() {
	return descriptorPool;
}

void Zap::init(const char* applicationName)
{
	GlobalSettings::colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

	if (!glfwInit())
		std::runtime_error("Can't initialize GLFW");

	initVulkan(applicationName);
}

void Zap::terminate() {
	descriptorPool.~DescriptorPool();

	terminateVulkan();
	glfwTerminate();
}