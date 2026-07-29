#include "Zap/Rendering/Renderer.h"

#include "VulkanUtils.h"

namespace Zap {
	Renderer::Renderer(VkExtent2D commonTargetExtent)
		: m_commonTargetExtent(commonTargetExtent)
	{}

	Renderer::~Renderer() {}

	void Renderer::init() {
		ZP_ASSERT(m_renderTaskMap.size(), "Renderer requires at least one RenderTask");

		m_commandBuffer.allocate();

		// Init DescriptorPool
		DescriptorPoolSizeList list;
		for (auto& taskPair : m_renderTaskMap) {
			taskPair.second->addDescriptorPoolSizes(list); // add the tasks descriptors to the pool size list
		}
		m_descriptorPool.addPoolSizes(list.m_poolSizes.data(), list.m_poolSizes.size());
		m_descriptorPool.setMaxSets(list.m_maxSets);
		if (list.m_maxSets > 0) {
			m_descriptorPool.init();
			m_isDescriptorPoolInit = true;
		}

		// Init recorded Tasks
		// transitions: (initial) -> (task) -> (final) for every target
		auto firstTask = getRenderTaskOrdered(0);
		if (firstTask)
			m_layoutTransitionHelper.addTransitions(firstTask->getLayoutTransitions()); // transition from initial layout to the requested layout of the first task
		for (uint32_t i = 0; i < m_renderTaskRecordOrder.size(); i++) {
			auto task = getRenderTaskOrdered(i);
			if (task) {
				if (i + 1 >= m_renderTaskRecordOrder.size()) {
					m_layoutTransitionHelper.addFinalTransitions(); // transition from the layout of this task to the final layout
				}
				else {
					auto nextTask = getRenderTaskOrdered(i+1);
					m_layoutTransitionHelper.addTransitions(nextTask->getLayoutTransitions()); // transition from the layout of this task to the next task's requested layout
				}

				m_layoutTransitionHelper.next(task->getLayoutTransitions());
				task->init(m_layoutTransitionHelper);
			}
		}
		m_layoutTransitionHelper.reset();

		vk::createFence(&m_renderComplete);
		m_isInit = true;
	}

	void Renderer::destroy() {
		vk::destroyFence(m_renderComplete);
		for (auto& taskPair : m_renderTaskMap) {
			taskPair.second->destroy();
		}
		m_commandBuffer.free();
		m_isInit = false;
	}

	void Renderer::resize(glm::vec2 size) {
		if (glm::vec2(m_commonTargetExtent.width, m_commonTargetExtent.height) == size)
			return;

		m_commonTargetExtent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };

		for (auto& targetPair : m_renderTargetMap) {
			targetPair.second->resizeInternal(m_commonTargetExtent); // call the internal resize function of all targets
		}

		RenderEvent::Resize resizeEvent;
		resizeEvent.size = size;
		m_eventHandler.pushEvent(resizeEvent);
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
		if (m_isInit)
			destroy();
		m_recordedFunctors.clear();
		m_renderTaskRecordOrder.clear();
	}

	void Renderer::endRecord() {
		if (!m_isInit)
			init();
	}

	class Renderer::RecRenderTask : public Renderer::RecordFunctor {
	public:
		RecRenderTask(RenderTaskHandle<> taskHandle)
			: m_taskHandle(taskHandle)
		{}

		virtual void operator()(const vk::CommandBuffer& cmd) override;

	private:
		RenderTaskHandle<> m_taskHandle;
	};

	void Renderer::recRenderTask(RenderTaskHandle<RenderTask> taskHandle) {
		m_recordedFunctors.push_back(std::make_unique<RecRenderTask>(taskHandle));
		m_renderTaskRecordOrder.push_back(taskHandle.m_handle); // remember the order of render tasks
	}
	void Renderer::RecRenderTask::operator()(const vk::CommandBuffer& cmd) {
		if (!m_taskHandle->m_isEnabled) // dont record commands for disabled tasks
			return;
		m_taskHandle->recordCommands(&cmd);
	}

	void Renderer::recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask) {

	}

	VkExtent2D Renderer::getCommonTargetExtent() {
		return m_commonTargetExtent;
	}

	void Renderer::recordCommandBuffer() {
		if (!m_recordedFunctors.size()) return;

		m_commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		m_layoutTransitionHelper.recInitialTransitions(m_commandBuffer);

		for (auto& pFunctor : m_recordedFunctors) {
			(*pFunctor)(m_commandBuffer);
		}

		m_commandBuffer.end();
	}

	RenderTarget* Renderer::getRenderTarget(UUID handle) {
		if (m_renderTargetMap.count(handle))
			return m_renderTargetMap.at(handle).get();
		return nullptr;
	}

	Framebuffer* Renderer::getFramebuffer(UUID handle) {
		if (m_framebufferMap.count(handle))
			return m_framebufferMap.at(handle).get();
		return nullptr;
	}

	DescriptorSet* Renderer::getDescriptorSet(UUID handle) {
		if (m_descriptorSetMap.count(handle))
			return m_descriptorSetMap.at(handle).get();
		return nullptr;
	}

	VkDescriptorPool Renderer::getDescriptorPool() {
		return m_descriptorPool;
	}

	RenderTask* Renderer::getRenderTask(UUID handle) {
		if (m_renderTaskMap.count(handle))
			return m_renderTaskMap.at(handle).get();
		return nullptr;
	}

	RenderTask* Renderer::getRenderTaskOrdered(uint32_t index) {
		UUID currentTaskHandle = m_renderTaskRecordOrder[index];
		if (m_renderTaskMap.count(currentTaskHandle)) {
			return getRenderTask(currentTaskHandle);
		}
		return nullptr;
	}

	void TaskLayoutTransitions::addLayout(RenderTargetHandle<> target, VkImageLayout layout) {
		m_layouts.push_back({target, layout});
	}

	void LayoutTransitionHelper::reset() {
		for (auto& pair : m_transitionMap) {
			pair.second.reset();
		}
	}

	void LayoutTransitionHelper::next(const TaskLayoutTransitions& transitions) {
		for (auto& pair : transitions.m_layouts) {
			if (m_transitionMap.count(pair.target.m_handle)) {
				m_transitionMap.at(pair.target.m_handle).next();
			}
		}
	}

	void LayoutTransitionHelper::recInitialTransitions(vk::CommandBuffer& cmd) {
		reset();
		for (auto& transitionPair : m_transitionMap) {
			auto transition = transitionPair.second.getTransition();
			transitionPair.second.getTarget()->recLayoutTransition(cmd, transition.oldLayout, transition.newLayout, VK_ACCESS_NONE, VK_ACCESS_MEMORY_WRITE_BIT);
		}
	}

	void LayoutTransitionHelper::addTransitions(const TaskLayoutTransitions& transitions) {
		for (auto& taskLayout : transitions.m_layouts) {
			auto& handle = taskLayout.target.m_handle;
			if (m_transitionMap.count(handle)) {
				m_transitionMap.at(handle).addLayout(taskLayout.layout);
			}
			else {
				m_transitionMap[handle] = TransitionList(taskLayout.target); // create new target entry
				m_transitionMap.at(handle).addLayout(taskLayout.target->getInitialLayout());
				m_transitionMap.at(handle).addLayout(taskLayout.layout);
			}
		}
	}

	void LayoutTransitionHelper::addFinalTransitions() {
		TaskLayoutTransitions transitions;
		for (auto& transitionPair : m_transitionMap) {
			transitions.addLayout(transitionPair.second.getTarget(), transitionPair.second.getTarget()->getFinalLayout());
		}
		addTransitions(transitions);
	}

	bool LayoutTransitionHelper::hasTransition(RenderTargetHandle<> handle) const {
		return m_transitionMap.count(handle.m_handle);
	}

	LayoutTransitionHelper::LayoutTransition LayoutTransitionHelper::getTransition(RenderTargetHandle<> handle) const {
		return m_transitionMap.at(handle.m_handle).getTransition();
	}
}