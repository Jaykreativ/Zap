#include "Zap.h"

namespace Zap {
	namespace GlobalSettings {
		VkFormat colorFormat;
		VkFormat getColorFormat() {
			return colorFormat;
		}

		VkFormat depthStencilFormat;
		VkFormat getDepthStencilFormat() {
			return depthStencilFormat;
		}
	}
}

void Zap::init(const char* applicationName)
{
	GlobalSettings::colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	GlobalSettings::depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

	if (!glfwInit())
		std::runtime_error("Can't initialize GLFW");

	initVulkan(applicationName);
}

void Zap::terminate() {
	terminateVulkan();
	glfwTerminate();
}