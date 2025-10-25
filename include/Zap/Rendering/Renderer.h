#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Vertex.h"
#include "Zap/Rendering/RenderTaskTemplate.h"
#include "Zap/Scene/Camera.h"
#include "glm.hpp"

namespace Zap {
	class RenderTargetHandle {
		friend class Renderer;
	public:
		RenderTargetHandle(const RenderTargetHandle& other);
		~RenderTargetHandle();

	private:
		RenderTargetHandle();
	};

	class RenderTarget : protected Image {
		friend class Renderer;
	public:
		RenderTarget();
		~RenderTarget();

		bool isValid();

	private:
		bool m_isValid = false;
	};

	template<class T>
	class RenderTaskHandle {
		friend class Renderer;
	public:
		RenderTaskHandle(const RenderTaskHandle& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}


		T* operator->() {
			return m_renderer.getRenderTask(m_handle);
		}

		operator bool() const {
			return m_renderer.getRenderTask(m_handle) != nullptr;
		}

	private:
		UUID m_handle;
		Renderer& m_renderer;

		RenderTaskHandle(UUID handle, Renderer& renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();
	
		void init();

		void destroy();

		// Only works on image targets
		// Has to be called when the target image gets resized
		void resize();

		void render();

		template<class T, class... Types>
		RenderTaskHandle<T> createRenderTask(Types&&... args) {
			static_assert(std::is_base_of_v<RenderTaskTemplate, T>, "Type has to be child class of RenderTaskTemplate");
			auto handle = UUID();
			m_renderTaskMap[handle] = std::make_unique<T>(std::forward<Types>(args)...);
			return RenderTaskHandle<T>(handle, *this);
		}

		template<class T>
		void destroyRenderTask(RenderTaskHandle<T> handle) {
			if(handle)
				m_renderTaskMap.erase(handle.m_handle);
		}

		RenderTargetHandle createRenderTarget();

		// record

		void beginRecord();

		void endRecord();

		void recRenderTemplate(RenderTaskTemplate* pRenderTemplate);

		void recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask);

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		bool m_isInit = false;

		std::unordered_map<UUID, std::unique_ptr<RenderTaskTemplate>> m_renderTaskMap = {};
		std::unordered_map<UUID, std::unique_ptr<RenderTarget>> m_renderTargetMap = {};

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

		RenderTaskTemplate* getRenderTask(UUID handle);

		static void onWindowResize(ResizeEvent& eventParams, void* customParams);
		
		template<class U>
		friend class RenderTaskHandle;
		friend class RenderTaskTemplate;
		friend class Window;
		friend class PBRenderer;//TODO add rendertoolkit for userdefined rendertasks
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class Gui;
	};
}

