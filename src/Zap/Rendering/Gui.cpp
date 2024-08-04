#include "Zap/Rendering/Gui.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/stb_image.h"
#include "VulkanUtils.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h";
#include "backends/imgui_impl_glfw.h";

namespace Zap {
	bool Gui::isImGuiInit = false;
	VkDescriptorPool Gui::descriptorPool = VK_NULL_HANDLE;
	vk::RenderPass Gui::renderPass = vk::RenderPass();
	Window* Gui::pImGuiWindow = nullptr;

	Gui::Gui(){}

	Gui::~Gui(){}

	void Gui::init(uint32_t width, uint32_t height, uint32_t imageCount) {
		/*Framebuffer*/
		m_framebufferCount = imageCount;
		m_framebuffers = new vk::Framebuffer[m_framebufferCount]();

		m_textureSampler = vk::Sampler();
		m_textureSampler.init();

		RenderTaskTemplate::initTargetDependencies();
	}

	void Gui::initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		m_framebuffers[imageIndex].setWidth(width);
		m_framebuffers[imageIndex].setHeight(height);
		m_framebuffers[imageIndex].addAttachment(pTarget->getVkImageView());
		m_framebuffers[imageIndex].setRenderPass(renderPass);
		m_framebuffers[imageIndex].init();
	}

	void Gui::resize(uint32_t width, uint32_t height, uint32_t imageCount) {
		RenderTaskTemplate::resizeTargetDependencies();
	}

	void Gui::resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
		m_framebuffers[imageIndex].setWidth(width);
		m_framebuffers[imageIndex].setHeight(height);
		m_framebuffers[imageIndex].delAttachment(0);
		m_framebuffers[imageIndex].addAttachment(pTarget->getVkImageView());
		m_framebuffers[imageIndex].update();
	}

	void Gui::destroy() {
		for (uint32_t i = 0; i < m_framebufferCount; i++) {
			m_framebuffers[i].destroy();
		}

		for (auto image : m_textures)
			image.destroy();
		m_textureSampler.destroy();
	}

	void Gui::beforeRender(vk::Image* pTarget, uint32_t imageIndex) {
		ImGui::Render();
	}

	void Gui::afterRender(vk::Image* pTarget, uint32_t imageIndex) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Gui::recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) {
		pTarget->cmdChangeLayout(*cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		VkExtent3D targetExtent = pTarget->getExtent();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = m_framebuffers[imageIndex];
		renderPassBeginInfo.renderArea = { 0, 0, targetExtent.width, targetExtent.height };
		VkClearValue clearColor = { 0, 0, 0, 1 };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(*cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);

		vkCmdEndRenderPass(*cmd);
	}

	GuiTexture Gui::loadTexture(Zap::Image* pImage) {
		return ImGui_ImplVulkan_AddTexture(m_textureSampler, pImage->getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);
	}

	GuiTexture Gui::loadTexture(const char* texturePath) {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		auto data = stbi_load(texturePath, &width, &height, &channels, 4);
		ZP_ASSERT(data, "Image not loaded correctly");
		m_textures.push_back(vk::Image());
		vk::Image* image = &m_textures.back();
		image->setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		image->setExtent(VkExtent3D{ (uint32_t)width, (uint32_t)height, 1 });

		image->setFormat(VK_FORMAT_R8G8B8A8_UNORM); // TODO look for 1cmp formats
		image->setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		image->setType(VK_IMAGE_TYPE_2D);

		image->init();
		image->allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image->initView();

		image->changeLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

		image->uploadData(width * height * 4, data);

		image->changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);

		return loadTexture(image);
	}

	void Gui::unloadTexture(GuiTexture texture) {
		ImGui_ImplVulkan_RemoveTexture(texture);
	}

	void Gui::initImGui(Window* pWindow) { 
		if (isImGuiInit) return;

		pImGuiWindow = pWindow;

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		vkCreateDescriptorPool(vk::getDevice(), &poolInfo, nullptr, &descriptorPool);

		/*RenderPass*/
		{
			VkAttachmentDescription colorAttachment;// Color Attachment
			colorAttachment.flags = 0;
			colorAttachment.format = Zap::GlobalSettings::getColorFormat();
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//TODO lookup what this means
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			renderPass.addAttachmentDescription(colorAttachment);

			VkAttachmentReference* pColorAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 0;
				tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				pColorAttachmentReference = &tmp;

				renderPass.addAttachmentReference(&pColorAttachmentReference);
			}

			VkSubpassDescription subpassDescription;
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = pColorAttachmentReference;
			subpassDescription.pResolveAttachments = nullptr;
			subpassDescription.pDepthStencilAttachment = nullptr;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;

			renderPass.addSubpassDescription(subpassDescription);

			VkSubpassDependency subpassDependency;
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = 0;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dependencyFlags = 0;

			renderPass.addSubpassDependency(subpassDependency);

			renderPass.init();
		}

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGuiStyle& style = ImGui::GetStyle();

		style.DisplayWindowPadding = ImVec2(0, 0);

		ImGui_ImplVulkan_InitInfo imguiInitInfo{};
		imguiInitInfo.Instance = vk::getInstance();
		imguiInitInfo.PhysicalDevice = vk::getPhysicalDevice();
		imguiInitInfo.Device = vk::getDevice();
		imguiInitInfo.QueueFamily = vk::getQueueFamily();
		imguiInitInfo.Queue = vkUtils::queueHandler::getQueue();
		imguiInitInfo.DescriptorPool = descriptorPool;
		imguiInitInfo.RenderPass = renderPass;
		imguiInitInfo.MinImageCount = VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES;
		imguiInitInfo.ImageCount = VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES;
		imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&imguiInitInfo);

		ImGui_ImplGlfw_InitForVulkan(*pImGuiWindow, true);

		ImGui_ImplVulkan_NewFrame();

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		isImGuiInit = true;
	}

	void Gui::destroyImGui() {
		if (!isImGuiInit) return;

		ImGui::EndFrame();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		renderPass.destroy();
		vkDestroyDescriptorPool(vk::getDevice(), descriptorPool, nullptr);

		pImGuiWindow = nullptr;

		isImGuiInit = false;
	}
}