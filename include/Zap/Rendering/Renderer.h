#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderTargets.h"
#include "Zap/Rendering/RenderTaskTemplate.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Vertex.h"
#include "Zap/Scene/Camera.h"
#include "glm.hpp"

namespace Zap {
	// handle to a RenderTarget stored in a Renderer
	// invalid if the Renderer was destroyed
	template<class T = RenderTarget>
	class RenderTargetHandle {
		friend class Renderer;
	public:
		RenderTargetHandle() = default;
		RenderTargetHandle(const RenderTargetHandle<T>& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}
		~RenderTargetHandle() = default;

		operator RenderTargetHandle<RenderTarget>() {
			return RenderTargetHandle<RenderTarget>(m_handle, m_renderer);
		}

		T* get() {
			return m_renderer->getRenderTarget(m_handle);
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_renderer != nullptr && get() != nullptr;
		}

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		RenderTargetHandle(UUID handle, Renderer* renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};

	// handle to a RenderTask stored in a Renderer
	// invalid if the Renderer was destroyed
	template<class T = RenderTask>
	class RenderTaskHandle {
		friend class Renderer;
	public:
		RenderTaskHandle() = default;
		RenderTaskHandle(const RenderTaskHandle<T>& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}
		~RenderTaskHandle() = default;

		operator RenderTaskHandle<RenderTask>() {
			return RenderTaskHandle<RenderTask>(m_handle, m_renderer);
		}

		T* get() {
			return m_renderer->getRenderTask(m_handle);
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_renderer != nullptr && get() != nullptr;
		}

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		RenderTaskHandle(UUID handle, Renderer* renderer)
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

		void resize();

		void render();

		template<class T, class... Types>
		RenderTaskHandle<T> createRenderTask(Types&&... args) {
			static_assert(std::is_base_of_v<RenderTask, T>, "Type has to be child class of RenderTask");
			auto handle = UUID();
			m_renderTaskMap[handle] = std::make_unique<T>(std::forward<Types>(args)...);
			m_renderTaskMap.at(handle)->m_pRenderer = this;
			return RenderTaskHandle<T>(handle, this);
		}

		template<class T>
		void destroyRenderTask(RenderTaskHandle<T> handle) {
			if(handle)
				m_renderTaskMap.erase(handle.m_handle);
		}

		template<class T, class... Types>
		RenderTargetHandle<T> createRenderTarget(Types&&... args) {
			static_assert(std::is_base_of_v<RenderTarget, T>, "Type has to be child class of RenderTarget");
			auto handle = UUID();
			m_renderTargetMap[handle] = std::make_unique<T>(std::forward<Types>(args)...);
			return RenderTargetHandle<T>(handle, *this);
		}

		template<class T>
		void destroyRenderTarget(RenderTargetHandle<T> handle) {
			if (handle)
				m_renderTargetMap.erase(handle.m_handle);
		}

		// record

		void beginRecord();

		void endRecord();

		void recRenderTemplate(RenderTaskHandle<> taskHandle);

		void recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask);

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		bool m_isInit = false;

		std::unordered_map<UUID, std::unique_ptr<RenderTask>> m_renderTaskMap = {};
		std::unordered_map<UUID, std::unique_ptr<RenderTarget>> m_renderTargetMap = {};

		//CommandBuffer
		vk::CommandBuffer m_commandBuffer;

		//Fences
		VkFence m_imageAvailable = VK_NULL_HANDLE;
		VkFence m_renderComplete = VK_NULL_HANDLE;

		//Recording
		class RecordFunctor {
		public:
			RecordFunctor() = default;
			virtual ~RecordFunctor() = default;

			virtual void operator()(const vk::CommandBuffer& cmd) = 0;
		};

		class RecRenderTask : public RecordFunctor {
		public:
			RecRenderTask(RenderTaskHandle<> taskHandle)
				: m_taskHandle(taskHandle)
			{}

			virtual void operator()(const vk::CommandBuffer& cmd) override;

		private:
			RenderTaskHandle<> m_taskHandle;
		};

		std::vector<std::unique_ptr<RecordFunctor>> m_recordedFunctors;

		void recordCommandBuffer();

		RenderTarget* getRenderTarget(UUID handle);

		RenderTask* getRenderTask(UUID handle);
		
		template<class T>
		friend class RenderTaskHandle;
		friend class RenderTask;
		template<class T>
		friend class RenderTargetHandle;
		friend class Window;
		friend class PBRenderer;//TODO add rendertoolkit for userdefined rendertasks
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class Gui;
	};
}

