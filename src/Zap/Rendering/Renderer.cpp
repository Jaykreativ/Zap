#include "Zap/Rendering/Renderer.h"
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

	void Renderer::resize() {} // TODO resize targets in this renderer

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

	RenderTask* Renderer::getRenderTask(UUID handle) {
		if (m_renderTaskMap.count(handle))
			return m_renderTaskMap.at(handle).get();
		else
			return nullptr;
	}
}