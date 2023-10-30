#include "Zap/Zap.h"
#include "Zap/Scene/MeshComponent.h"

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

	Base::Base(const char* applicationName) {
		char* str = new char[strlen(applicationName) + 1];
		strcpy(str, applicationName);

		m_applicationName = str;
	};

	Base::~Base() {
		delete[] m_applicationName;
	}

	void Base::init()
	{
		GlobalSettings::colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		GlobalSettings::depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

		if (!glfwInit())
			std::runtime_error("Can't initialize GLFW");

		initVulkan(m_applicationName);
	}

	void Base::terminate() {
		for (MeshComponent mc : MeshComponent::all) mc.m_pMesh->~Mesh();

		terminateVulkan();
		glfwTerminate();
	}
}