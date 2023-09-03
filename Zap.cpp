#include "Zap.h"

namespace Zap {
	namespace GlobalSettings {
		VkFormat colorFormat;
		VkFormat getColorFormat() {
			return colorFormat;
		}
	}
}

void Zap::init()
{
	GlobalSettings::colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

	if (!glfwInit())
		std::runtime_error("Can't initialize GLFW");
}