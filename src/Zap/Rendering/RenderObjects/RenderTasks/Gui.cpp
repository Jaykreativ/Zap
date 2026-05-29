#include "Zap/Rendering/RenderObjects/RenderTasks/Gui.h"
#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/stb_image.h"
#include "VulkanUtils.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h";
#include "backends/imgui_impl_glfw.h";

namespace Zap {
	std::vector<RenderTaskHandle<Gui>> Gui::m_contextStack = {};

	GuiImageRef::GuiImageRef(std::weak_ptr<Image2D> imageRef)
		: m_imageRef(imageRef)
	{
		if (auto spImage = m_imageRef.lock()) {
			m_texSampler = vk::Sampler();
			m_texSampler.init();
			m_guiTexture = ImGui_ImplVulkan_AddTexture(m_texSampler, spImage->getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);
		}
	}

	GuiImageRef::~GuiImageRef(){
		ImGui_ImplVulkan_RemoveTexture(m_guiTexture);
		m_texSampler.destroy();
	}

	GuiImageRef::operator bool() { return !m_imageRef.expired(); }

	GuiImageRef::operator GuiTexture() {
		if (*this)
			return m_guiTexture;
		else
			return VK_NULL_HANDLE;
	}

	Gui::Gui(Renderer* pRenderer, RenderTargetHandle<> target, Window* pWindow)
		: RenderTask(pRenderer), m_target(target), m_pWindow(pWindow)
	{}

	Gui::~Gui(){
		ImGui::EndFrame();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		for (auto image : m_textures)
			image.destroy();
		m_textureSampler.destroy();

		m_pRenderer->destroyFramebuffer(m_framebuffer);

		m_renderPass.destroy();
		vkDestroyDescriptorPool(vk::getDevice(), m_descriptorPool, nullptr);

		m_pWindow = nullptr;
	}

	void Gui::init(const LayoutTransitionHelper& layoutTransitionHelper) {
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

		vkCreateDescriptorPool(vk::getDevice(), &poolInfo, nullptr, &m_descriptorPool);

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
			colorAttachment.initialLayout = layoutTransitionHelper.getTransition(m_target).oldLayout;
			colorAttachment.finalLayout = layoutTransitionHelper.getTransition(m_target).newLayout;

			m_renderPass.addAttachmentDescription(colorAttachment);

			VkAttachmentReference* pColorAttachmentReference;
			{
				VkAttachmentReference tmp;
				tmp.attachment = 0;
				tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				pColorAttachmentReference = &tmp;

				m_renderPass.addAttachmentReference(&pColorAttachmentReference);
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

			m_renderPass.addSubpassDescription(subpassDescription);

			VkSubpassDependency subpassDependency;
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = 0;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dependencyFlags = 0;

			m_renderPass.addSubpassDependency(subpassDependency);

			m_renderPass.init();
		}

		/*Framebuffer*/
		m_framebuffer = m_pRenderer->createFramebuffer(m_renderPass, { m_target });

		m_textureSampler = vk::Sampler();
		m_textureSampler.init();

		auto* context = ImGui::CreateContext();
		ImGui::SetCurrentContext(context);

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
		imguiInitInfo.DescriptorPool = m_descriptorPool;
		imguiInitInfo.RenderPass = m_renderPass;
		imguiInitInfo.MinImageCount = VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES;
		imguiInitInfo.ImageCount = VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES;
		imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&imguiInitInfo);

		ImGui_ImplGlfw_InitForVulkan(*m_pWindow, true);

		ImGui_ImplVulkan_NewFrame();

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
	}

	void Gui::destroy() {}

	void Gui::beforeRender() {
		ImGui::Render();
	}

	void Gui::afterRender() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Gui::addDescriptorPoolSizes(DescriptorPoolSizeList& poolSizes) {}

	TaskLayoutTransitions Gui::getLayoutTransitions() {
		TaskLayoutTransitions helper;
		helper.addLayout(m_target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		return helper;
	}

	void Gui::recordCommands(const vk::CommandBuffer* cmd) {
		VkExtent3D targetExtent = m_target->getExtent();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = *m_framebuffer.get();
		renderPassBeginInfo.renderArea = { 0, 0, targetExtent.width, targetExtent.height };
		VkClearValue clearColor = { 0, 0, 0, 1 };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(*cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);

		vkCmdEndRenderPass(*cmd);
	}

	//GuiTexture Gui::loadTexture(Zap::Image* pImage) {
	//	return ImGui_ImplVulkan_AddTexture(m_textureSampler, pImage->getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);
	//}

	//GuiTexture Gui::loadTexture(const char* texturePath) {
	//	int width, height, channels;
	//	stbi_set_flip_vertically_on_load(false);
	//	auto data = stbi_load(texturePath, &width, &height, &channels, 4);
	//	ZP_ASSERT(data, "Image not loaded correctly");
	//	m_textures.push_back(vk::Image());
	//	vk::Image* image = &m_textures.back();
	//	image->setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
	//	image->setExtent(VkExtent3D{ (uint32_t)width, (uint32_t)height, 1 });
	//
	//	image->setFormat(VK_FORMAT_R8G8B8A8_UNORM); // TODO look for 1cmp formats
	//	image->setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	//	image->setType(VK_IMAGE_TYPE_2D);
	//
	//	image->init();
	//	image->allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//	image->initView();
	//
	//	image->changeLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);
	//
	//	image->uploadData(width * height * 4, data);
	//
	//	image->changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
	//
	//	return loadTexture(image);
	//}
	//
	//void Gui::unloadTexture(GuiTexture texture) {
	//	ImGui_ImplVulkan_RemoveTexture(texture);
	//}

	void Gui::pushContext(RenderTaskHandle<Gui> handle) {
		ImGui::SetCurrentContext(handle->m_context);
		m_contextStack.push_back(handle);
	}

	void Gui::popContext() {
		m_contextStack.pop_back();
		if(!m_contextStack.empty())
			ImGui::SetCurrentContext(m_contextStack.back()->m_context);
	}
}