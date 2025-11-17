#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/RenderTargets.h"
#include "Zap/Rendering/Framebuffer.h"
#include "Zap/Rendering/RenderTask.h"
#include "Zap/Vertex.h"
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

		// user interface to resize all RenderTargets in this renderer
		void resize(glm::vec2 size);

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
			m_renderTargetMap.at(handle)->m_pRenderer = this;
			return RenderTargetHandle<T>(handle, this);
		}

		template<class T>
		void destroyRenderTarget(RenderTargetHandle<T> handle) {
			if (handle)
				m_renderTargetMap.erase(handle.m_handle);
		}

		FramebufferHandle createFramebuffer(VkRenderPass renderPass, std::initializer_list<RenderTargetHandle<>> targets) {
			auto handle = UUID();
			m_framebufferMap[handle] = std::make_unique<Framebuffer>(renderPass, targets);
			return FramebufferHandle(handle, this);
		}

		void destroyFramebuffer(FramebufferHandle handle) {
			if (handle)
				m_framebufferMap.erase(handle.m_handle);
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
		std::unordered_map<UUID, std::unique_ptr<Framebuffer>> m_framebufferMap = {};

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

		class RecRenderTask;

		std::vector<std::unique_ptr<RecordFunctor>> m_recordedFunctors;

		void recordCommandBuffer();

		RenderTarget* getRenderTarget(UUID handle);

		Framebuffer* getFramebuffer(UUID handle);

		RenderTask* getRenderTask(UUID handle);
		
		template<class T>
		friend class RenderTaskHandle;
		friend class RenderTask;
		template<class T>
		friend class RenderTargetHandle;
		friend class FramebufferHandle;
		friend class Window;
		friend class PBRenderer;//TODO add rendertoolkit for userdefined rendertasks
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class Gui;
	};
}

