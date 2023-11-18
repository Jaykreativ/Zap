#include "Zap/Gui.h"
#include "VulkanUtils.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h";
#include "backends/imgui_impl_glfw.h";

namespace Zap {
	Gui::Gui(Window& window) 
		: Renderer(window)
	{}

	Gui::~Gui(){
		ImGui::EndFrame();

		vkDestroyDescriptorPool(vk::getDevice(), m_imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}

    void Gui::init() {
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

		vkCreateDescriptorPool(vk::getDevice(), &poolInfo, nullptr, &m_imguiPool);

		ImGui::CreateContext();

		ImGui_ImplVulkan_InitInfo imguiInitInfo{};
		imguiInitInfo.Instance = vk::getInstance();
		imguiInitInfo.PhysicalDevice = vk::getPhysicalDevice();
		imguiInitInfo.Device = vk::getDevice();
		imguiInitInfo.QueueFamily = vk::getQueueFamily();
		imguiInitInfo.Queue = vkUtils::queueHandler::getQueue();
		imguiInitInfo.DescriptorPool = m_imguiPool;
		imguiInitInfo.MinImageCount = VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES;
		imguiInitInfo.ImageCount = VK_MIN_AMOUNT_OF_SWAPCHAIN_IMAGES;
		imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&imguiInitInfo, m_window.m_renderPass);

		ImGui_ImplGlfw_InitForVulkan(m_window.m_window, true);

		ImGui_ImplVulkan_NewFrame();

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
    }

	void Gui::render(uint32_t cam) {
		ImGui::Render();

		auto cmd = vk::CommandBuffer();
		cmd.allocate();
		cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = *m_window.getRenderPass();
		renderPassBeginInfo.framebuffer = *m_window.getFramebuffer(m_window.getCurrentImageIndex());
		VkRect2D renderArea{};
		int32_t restX = m_window.getWidth() - (m_scissor.extent.width + m_scissor.offset.x);
		renderArea.offset.x = std::max<int32_t>(0, m_window.getWidth() - (m_scissor.extent.width + std::max<int32_t>(0, restX)));
		renderArea.extent.width = std::min<int32_t>(m_window.getWidth() - (m_scissor.offset.x + restX), m_window.getWidth());
		int32_t restY = m_window.getHeight() - (m_scissor.extent.height + m_scissor.offset.y);
		renderArea.offset.y = std::max<int32_t>(0, m_window.getHeight() - (m_scissor.extent.height + std::max<int32_t>(0, restY)));
		renderArea.extent.height = std::min<int32_t>(m_window.getHeight() - (m_scissor.offset.y + restY), m_window.getHeight());
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = 0;
		renderPassBeginInfo.pClearValues = nullptr;

		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

		vkCmdEndRenderPass(cmd);

		cmd.end();
		cmd.submit();

		ImGui_ImplVulkan_NewFrame();

		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
	}
}