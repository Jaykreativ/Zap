#include "Zap/Rendering/Renderer.h"
#include "VulkanUtils.h"

namespace Zap {
	Renderer::Renderer() {}

	Renderer::~Renderer() {
		if(m_pWindowTarget)
			m_pWindowTarget->getResizeEventHandler()->removeCallback(Renderer::onWindowResize, this);
	}

	void Renderer::init() {
		if (m_pTarget)
			m_commandBufferCount = 1;
		else if (m_pWindowTarget)
			m_commandBufferCount = m_pWindowTarget->m_swapchain.getImageCount();
		else
			ZP_ASSERT(false, "Renderer has no render target, use setTarget to set a window or image target");
		if(m_pTarget && m_pWindowTarget)
			ZP_ASSERT(false, "Renderer has window and image target, the Renderer can only have one target at a time");

		m_commandBuffers = new vk::CommandBuffer[m_commandBufferCount];
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].allocate();
		}

		// Init Tasks
		VkExtent3D targetExtent;
		uint32_t imageCount;
		if (m_pTarget) {
			imageCount = 1;
			targetExtent = m_pTarget->getExtent();
		}
		else {
			imageCount = m_pWindowTarget->getSwapchain()->getImageCount();
			targetExtent = { m_pWindowTarget->getWidth(), m_pWindowTarget->getHeight(), 1};
		}

		for (auto task : m_renderTasks) {
			task->init(targetExtent.width, targetExtent.height, imageCount);
		}

		vk::createFence(&m_renderComplete);
	}

	void Renderer::destroy() {
		for (uint32_t i = 0; i < m_commandBufferCount; i++) {
			m_commandBuffers[i].free();
		}
		for (auto task : m_renderTasks) {
			task->destroy();
		}
		vk::destroyFence(m_renderComplete);
	}

	void Renderer::resize() {
		if (!m_pTarget) return;

		VkExtent3D targetExtent = m_pTarget->getExtent();
		uint32_t imageCount = 1;

		for (auto task : m_renderTasks) {
			task->resize(targetExtent.width, targetExtent.height, imageCount);
		}
	}

	void Renderer::render() {
		if (m_pWindowTarget) {
			if (m_pWindowTarget->isIconified()) return;
		}

		uint32_t cmdBufferIndex;
		if (m_pTarget) {
			cmdBufferIndex = 0;
		}
		else {
			cmdBufferIndex = m_pWindowTarget->getSwapchainImageIndex();
		}

		for (RenderTaskTemplate* renderTemplate : m_renderTasks) {
			if (renderTemplate->m_isEnabled) {
				if(m_pTarget)
					renderTemplate->beforeRender(m_pTarget, 0);
				else
					renderTemplate->beforeRender(m_pWindowTarget->m_swapchain.getImage(cmdBufferIndex), cmdBufferIndex);
			}
		}

		// Render

		recordCommandBuffer();
		m_commandBuffers[cmdBufferIndex].submit(m_renderComplete);
		vk::waitForFence(m_renderComplete); // TODO use semaphores for parallelization

		for (RenderTaskTemplate* renderTemplate : m_renderTasks) {
			if (renderTemplate->m_isEnabled) {
				if (m_pTarget)
					renderTemplate->afterRender(m_pTarget, 0);
				else
					renderTemplate->afterRender(m_pWindowTarget->m_swapchain.getImage(cmdBufferIndex), cmdBufferIndex);
			}
		}
	}

	void Renderer::beginRecord() {
		m_recordedFunctions.clear();
		m_recordedParams.clear();
	}

	void Renderer::endRecord() {}

	void Renderer::recRenderTemplate(RenderTaskTemplate* renderTemplate) {
		m_recordedFunctions.push_back(eRENDER_TEMPLATE);
		m_recordedParams.resize(m_recordedParams.size()+sizeof(RenderTaskTemplate*));
		memcpy(&m_recordedParams[m_recordedParams.size()-sizeof(RenderTaskTemplate*)], &renderTemplate, sizeof(RenderTaskTemplate*));
	}

	void Renderer::recChangeImageLayout(Image* pImage, VkImageLayout layout, VkAccessFlags accessMask) {
		m_recordedFunctions.push_back(eCHANGE_IMAGE_LAYOUT);
		uint32_t oldSize = m_recordedParams.size();
		m_recordedParams.resize(m_recordedParams.size() + sizeof(Image*) + sizeof(VkImageLayout) + sizeof(VkAccessFlags));
		memcpy(&m_recordedParams[oldSize], &pImage, sizeof(Image*));
		memcpy(&m_recordedParams[oldSize+sizeof(Image*)], &layout, sizeof(VkImageLayout));
		memcpy(&m_recordedParams[oldSize+sizeof(Image*)+sizeof(VkImageLayout)], &accessMask, sizeof(VkAccessFlags));
	}

	void Renderer::addRenderTask(RenderTaskTemplate* renderTask) {
		m_renderTasks.push_back(renderTask);
		renderTask->m_pRenderer = this;
	}

	void Renderer::setTarget(vk::Image* imageTarget) {
		m_pTarget = imageTarget;
		if (m_pWindowTarget)
			m_pWindowTarget->getResizeEventHandler()->removeCallback(Renderer::onWindowResize, this);
		m_pWindowTarget = nullptr;
	}

	void Renderer::setTarget(Window* windowTarget) {
		m_pWindowTarget = windowTarget;
		m_pWindowTarget->getResizeEventHandler()->addCallback(Renderer::onWindowResize, this);
		m_pTarget = nullptr;
	}

	void Renderer::initRenderTaskTargetDependencies(RenderTaskTemplate* task) {
		VkExtent3D targetExtent;
		uint32_t imageCount;
		if (m_pTarget) {
			imageCount = 1;
			targetExtent = m_pTarget->getExtent();
		}
		else {
			imageCount = m_pWindowTarget->getSwapchain()->getImageCount();
			targetExtent = { m_pWindowTarget->getWidth(), m_pWindowTarget->getHeight(), 1 };
		}

		if (m_pTarget)
			task->initTargetDependencies(targetExtent.width, targetExtent.height, imageCount, m_pTarget, 0);
		else {
			for (uint32_t i = 0; i < imageCount; i++) {
				task->initTargetDependencies(targetExtent.width, targetExtent.height, imageCount, m_pWindowTarget->m_swapchain.getImage(i), i);
			}
		}
	}

	void Renderer::resizeRenderTaskTargetDependencies(RenderTaskTemplate* task) {
		VkExtent3D targetExtent;
		uint32_t imageCount;
		if (m_pTarget) {
			imageCount = 1;
			targetExtent = m_pTarget->getExtent();
		}
		else {
			imageCount = m_pWindowTarget->getSwapchain()->getImageCount();
			targetExtent = { m_pWindowTarget->getWidth(), m_pWindowTarget->getHeight(), 1 };
		}

		if(m_pTarget)
			task->resizeTargetDependencies(targetExtent.width, targetExtent.height, imageCount, m_pTarget, 0);
		else {
			for (uint32_t i = 0; i < imageCount; i++)
				task->resizeTargetDependencies(targetExtent.width, targetExtent.height, imageCount, m_pWindowTarget->m_swapchain.getImage(i), i);
		}
	}

	void Renderer::recordCommandBuffer() {
		if (!m_recordedFunctions.size()) return;

		uint32_t imageIndex;
		if (m_pTarget)
			imageIndex = 0;
		else
			imageIndex = m_pWindowTarget->getSwapchainImageIndex();

		vk::CommandBuffer* cmd = &m_commandBuffers[imageIndex];
		cmd->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		char* currentParam = &m_recordedParams[0];
		for (FunctionType function : m_recordedFunctions) {
			switch (function)
			{
			case eRENDER_TEMPLATE:
				RenderTaskTemplate* pRenderTemplate;
				memcpy(&pRenderTemplate, currentParam, sizeof(RenderTaskTemplate*)); // get render template from recorded parameters
				currentParam += sizeof(RenderTaskTemplate*);
				if (!pRenderTemplate->m_isEnabled) // dont record commands for disabled tasks
					break;
				if (m_pTarget)
					pRenderTemplate->recordCommands(cmd, m_pTarget, imageIndex);
				else
					pRenderTemplate->recordCommands(cmd, m_pWindowTarget->m_swapchain.getImage(imageIndex), imageIndex);
				break;
			case eCHANGE_IMAGE_LAYOUT:
				Image* pImage;
				memcpy(&pImage, currentParam, sizeof(Image*));
				currentParam += sizeof(Image*);

				VkImageLayout layout;
				memcpy(&layout, currentParam, sizeof(VkImageLayout));
				currentParam += sizeof(VkImageLayout);

				VkAccessFlags accessMask;
				memcpy(&accessMask, currentParam, sizeof(VkAccessFlags));
				currentParam += sizeof(VkAccessFlags);
				pImage->cmdChangeLayout(*cmd, layout, accessMask);
				break;
			default:
				ZP_ASSERT(false, "Unknown function type");
				break;
			}
		}

		cmd->end();
	}

	void Renderer::onWindowResize(ResizeEvent& eventParams, void* customParams) {
		Renderer* obj = (Renderer*)customParams;

		if (obj->m_pWindowTarget->isIconified()) return;

		VkExtent3D targetExtent;
		uint32_t imageCount;
		imageCount = obj->m_pWindowTarget->getSwapchain()->getImageCount();
		targetExtent = { obj->m_pWindowTarget->getWidth(), obj->m_pWindowTarget->getHeight(), 1 };

		for (auto task : obj->m_renderTasks) {
			task->resize(targetExtent.width, targetExtent.height, imageCount);
		}
	}
}