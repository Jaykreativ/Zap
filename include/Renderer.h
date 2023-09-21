#pragma once

#include "glm.hpp"
#include "Zap.h"
#include "Window.h"
#include "Vertex.h"
#include "VisibleActor.h"
#include "Camera.h"

namespace Zap {
	class Renderer
	{
	public:
		Renderer(Window& window);
		~Renderer();
	
		void init();

		void render(Camera* cam);

		void addActor(VisibleActor& actor);

		void setViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y);

		//TODO make actor system


	private:
		bool m_isInit = false;

		Window& m_window;
		VkViewport m_viewport;
		VkRect2D m_scissor;
		std::vector<VisibleActor*> m_actors;

		vk::DescriptorPool m_descriptorPool = vk::DescriptorPool();

		vk::Shader m_vertexShader = vk::Shader();
		vk::Shader m_fragmentShader = vk::Shader();
		vk::Pipeline m_pipeline = vk::Pipeline();

		//Fences
		VkFence m_renderComplete;

		//Buffers
		struct UniformBufferObject {// definition of the uniform buffer layout
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 perspective;
			glm::vec3 color;
		};

		UniformBufferObject m_ubo{};// the host uniform buffer
		vk::Buffer m_uniformBuffer = vk::Buffer();// the vulkan uniform buffer
	};
}

