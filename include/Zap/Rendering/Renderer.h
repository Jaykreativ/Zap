#pragma once

#include "Zap/Zap.h"
#include "Zap/Events.h"
#include "Zap/Rendering/Window.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"
#include "Zap/Rendering/RenderObjects/Framebuffer.h"
#include "Zap/Rendering/RenderObjects/RenderTask.h"
#include "Zap/Rendering/RenderObjects/DescriptorSet.h"
#include "Zap/Vertex.h"
#include "Zap/Scene/Camera.h"

#include "glm.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <array>

namespace Zap {
	class TaskLayoutTransitions {
		friend class LayoutTransitionHelper;
	public:
		struct TargetLayout {
			RenderTargetHandle<> target;
			VkImageLayout layout;
		};

		void addLayout(RenderTargetHandle<> target, VkImageLayout layout);

	private:
		std::vector<TargetLayout> m_layouts = {};
	};

	// transitions can be done using tasks and barriers
	class LayoutTransitionHelper {
	public:
		struct LayoutTransition {
			VkImageLayout oldLayout;
			VkImageLayout newLayout;
		};

		class TransitionList {
		public:
			TransitionList() {}
			TransitionList(RenderTargetHandle<> target)
				: m_target(target)
			{}

			void addLayout(VkImageLayout layout) { m_layouts.push_back(layout); }

			void next() { m_currentIndex++; }

			void reset() { m_currentIndex = 0; }

			LayoutTransition getTransition() const { return { m_layouts[m_currentIndex], m_layouts[(m_currentIndex + 1) % m_layouts.size()] }; }

			RenderTargetHandle<> getTarget() { return m_target; }

		private:
			RenderTargetHandle<> m_target;
			uint32_t m_currentIndex = 0;
			std::vector<VkImageLayout> m_layouts;
		};

		void reset();

		void next(TaskLayoutTransitions& transitions);

		void recInitialTransitions(vk::CommandBuffer& cmd);

		// replaces the newLayouts with the ones from the added transitions and stores them in the oldLayouts
		void addTransitions(TaskLayoutTransitions& transitions);

		void addFinalTransitions();

		bool hasTransition(RenderTargetHandle<> handle) const;

		LayoutTransition getTransition(RenderTargetHandle<> handle) const;
	private:
		std::unordered_map<UUID, TransitionList> m_transitionMap = {};
	};

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
			m_renderTaskMap[handle] = std::move(std::make_unique<T>(this, std::forward<Types>(args)...));
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
			m_renderTargetMap[handle] = std::move(std::make_unique<T>(this, std::forward<Types>(args)...));
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
			m_framebufferMap[handle] = std::move(std::make_unique<Framebuffer>(this, renderPass, targets));
			return FramebufferHandle(handle, this);
		}

		void destroyFramebuffer(FramebufferHandle handle) {
			if (handle)
				m_framebufferMap.erase(handle.m_handle);
		}

		template<class T, class... Types>
		DescriptorSetHandle<T> createDescriptorSet(Types&&... args) {
			static_assert(std::is_base_of_v<DescriptorSet, T>, "Type has to be child class of DescriptorSet");
			auto handle = UUID();
			m_descriptorSetMap[handle] = std::move(std::make_unique<T>(this, std::forward<Types>(args)...));
			m_descriptorSetMap.at(handle)->m_pRenderer = this;
			return DescriptorSetHandle<T>(handle, this);
		}

		template<class T>
		void destroyDescriptorSet(DescriptorSetHandle<T> handle) {
			if (handle)
				m_descriptorSetMap.erase(handle.m_handle);
		}

		// record

		void beginRecord();

		void endRecord();

		void recRenderTask(RenderTaskHandle<> taskHandle);

		void recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask);

#ifndef ZP_ALL_PUBLIC
	private:
#endif
		bool m_isInit = false;

		RenderEventHandler m_eventHandler;

		//DescriptorPool
		bool m_isDescriptorPoolInit = false;
		DescriptorPool m_descriptorPool;

		//CommandBuffer
		vk::CommandBuffer m_commandBuffer;

		std::unordered_map<UUID, std::unique_ptr<RenderTask>> m_renderTaskMap = {};
		std::unordered_map<UUID, std::unique_ptr<RenderTarget>> m_renderTargetMap = {};
		std::unordered_map<UUID, std::unique_ptr<Framebuffer>> m_framebufferMap = {};
		std::unordered_map<UUID, std::unique_ptr<DescriptorSet>> m_descriptorSetMap = {};

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

		std::vector<std::unique_ptr<RecordFunctor>> m_recordedFunctors = {};

		std::vector<UUID> m_renderTaskRecordOrder = {};
		LayoutTransitionHelper m_layoutTransitionHelper;

		void recordCommandBuffer();

		RenderTarget* getRenderTarget(UUID handle);

		Framebuffer* getFramebuffer(UUID handle);

		DescriptorSet* getDescriptorSet(UUID handle);

		VkDescriptorPool getDescriptorPool();

		RenderTask* getRenderTask(UUID handle);

		RenderTask* getRenderTaskOrdered(uint32_t index);
		
		friend class RenderObject;
		template<class T>
		friend class RenderTaskHandle;
		friend class RenderTask;
		template<class T>
		friend class RenderTargetHandle;
		friend class FramebufferHandle;
		template<class T>
		friend class DescriptorSetHandle;
		friend class DescriptorSet;
		friend class Window;
		friend class PBRenderer;
		friend class RaytracingRenderer;
		friend class PathTracer;
		friend class Gui;
	};
}

