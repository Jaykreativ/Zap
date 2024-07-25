#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Vertex.h"
#include "Zap/Rendering/RenderTaskTemplate.h"
#include "Zap/Scene/Camera.h"
#include "glm.hpp"

namespace Zap {
	class Renderer
	{
	public:
		Renderer();
		~Renderer();
	
		void init();

		void destroy();

		//Only works on image targets
		//Has to be called when the target image gets resized
		void resize();

		void render();

		void beginRecord();

		void endRecord();

		void recRenderTemplate(RenderTaskTemplate* pRenderTemplate);

		void recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask);

		void addRenderTask(RenderTaskTemplate* pRenderTemplate);

		void setTarget(vk::Image* imageTarget);
		void setTarget(Window* windowTarget);

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		bool m_isInit = false;

		//Target
		Window* m_pWindowTarget = nullptr;
		Zap::Image* m_pTarget = nullptr;

		//CommandBuffers 
		uint32_t m_commandBufferCount;
		vk::CommandBuffer* m_commandBuffers;

		//Fences
		VkFence m_imageAvailable = VK_NULL_HANDLE;
		VkFence m_renderComplete = VK_NULL_HANDLE;

		std::vector<RenderTaskTemplate*> m_renderTasks;

		//Recording
		enum FunctionType {
			eRENDER_TEMPLATE = 0,
			eCHANGE_IMAGE_LAYOUT = 1
		};

		std::vector<FunctionType> m_recordedFunctions;
		std::vector<char> m_recordedParams = {};

		void initRenderTaskTargetDependencies(RenderTaskTemplate* task);

		void resizeRenderTaskTargetDependencies(RenderTaskTemplate* task);

		void recordCommandBuffer();

		static void onWindowResize(ResizeEvent& eventParams, void* customParams);

		friend class Window;
		friend class RenderTaskTemplate;
		friend class PBRenderer;//TODO add rendertoolkit for userdefined rendertasks
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class Gui;
	};
}

