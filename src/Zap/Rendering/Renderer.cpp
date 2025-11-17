#include "Zap/Rendering/Renderer.h"

#include "Zap/Rendering/RenderTargets.h"
#include "Zap/Rendering/Framebuffer.h"
#include "Zap/Rendering/RenderTask.h"
#include "VulkanUtils.h"

namespace Zap {
	Renderer::Renderer() {}

	Renderer::~Renderer() {}

	void Renderer::init() {
		m_commandBuffer.allocate();

		// Init Tasks
		for (auto& taskPair : m_renderTaskMap) {
			taskPair.second->init();
		}

		vk::createFence(&m_renderComplete);
	}

	void Renderer::destroy() {
		m_commandBuffer.free();
		for (auto& taskPair : m_renderTaskMap) {
			taskPair.second->destroy();
		}
		vk::destroyFence(m_renderComplete);
	}

	void Renderer::resize(glm::vec2 size) {
		for (auto& targetPair : m_renderTargetMap) {
			targetPair.second->resizeInternal(size); // call the internal resize function of all targets
		}
		for (auto& framebufferPair : m_framebufferMap) {
			framebufferPair.second->update(); // update all framebuffers after resizing their contents
		}
	}

	void Renderer::render() {
		for (auto& taskPair : m_renderTaskMap) {
			auto& task = taskPair.second;
			if (task->m_isEnabled) {
				task->beforeRender();
			}
		}

		// Render

		recordCommandBuffer();
		m_commandBuffer.submit(m_renderComplete);
		vk::waitForFence(m_renderComplete); // TODO use semaphores for parallelization

		for (auto& taskPair : m_renderTaskMap) {
			auto& task = taskPair.second;
			if (task->m_isEnabled) {
				task->afterRender();
			}
		}
	}

	void Renderer::beginRecord() {
		m_recordedFunctors.clear();
	}

	void Renderer::endRecord() {}

	class Renderer::RecRenderTask : public Renderer::RecordFunctor {
	public:
		RecRenderTask(RenderTaskHandle<> taskHandle)
			: m_taskHandle(taskHandle)
		{}

		virtual void operator()(const vk::CommandBuffer& cmd) override;

	private:
		RenderTaskHandle<> m_taskHandle;
	};

	void Renderer::recRenderTemplate(RenderTaskHandle<RenderTask> taskHandle) {
		m_recordedFunctors.push_back(std::make_unique<RecRenderTask>(taskHandle));
	}
	void Renderer::RecRenderTask::operator()(const vk::CommandBuffer& cmd) {
		if (!m_taskHandle->m_isEnabled) // dont record commands for disabled tasks
			return;
		m_taskHandle->recordCommands(&cmd);
	}

	void Renderer::recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask) {

	}

	void Renderer::recordCommandBuffer() {
		if (!m_recordedFunctors.size()) return;

		m_commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		for (auto& pFunctor : m_recordedFunctors) {
			(*pFunctor)(m_commandBuffer);
		}

		m_commandBuffer.end();
	}

	RenderTarget* Renderer::getRenderTarget(UUID handle) {
		if (m_renderTargetMap.count(handle))
			return m_renderTargetMap.at(handle).get();
		else
			return nullptr;
	}

	Framebuffer* Renderer::getFramebuffer(UUID handle) {
		if (m_framebufferMap.count(handle))
			return m_framebufferMap.at(handle).get();
		else
			return nullptr;
	}

	RenderTask* Renderer::getRenderTask(UUID handle) {
		if (m_renderTaskMap.count(handle))
			return m_renderTaskMap.at(handle).get();
		else
			return nullptr;
	}
}