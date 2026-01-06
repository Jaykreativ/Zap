#include "Zap/Rendering/RenderObjects/DescriptorSet.h"

#include "Zap/Rendering/Renderer.h"

namespace Zap {
	DescriptorSetBinding::DescriptorSetBinding(VkDescriptorType type, VkShaderStageFlags stages, uint32_t count)
		: m_type(type), m_stages(stages), m_count(count)
	{}

	DescriptorSet::DescriptorSet(uint32_t size) {
		m_bindings.resize(size);
	}

	void DescriptorSet::addBinding(const DescriptorSetBinding& binding) {
		bool hasSpace = m_nextBinding < m_bindings.size();
		ZP_WARN(hasSpace, "DescriptorSet cannot hold more bindings, adjust DescriptorSet size");
		if (!hasSpace) return;
		
		m_bindings[m_nextBinding] = binding;
		m_nextBinding++;
	}

	void DescriptorSet::createLayout() {
		auto* bindings = new VkDescriptorSetLayoutBinding[m_bindings.size()];
		for (uint32_t i = 0; i < m_nextBinding; i++) {
			bindings[i].binding = i;
			bindings[i].descriptorType = m_bindings[i].m_type;
			bindings[i].descriptorCount = m_bindings[i].m_count;
			bindings[i].stageFlags = m_bindings[i].m_stages;
		}

		VkDescriptorSetLayoutCreateInfo info {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0};
		info.bindingCount = m_nextBinding;
		info.pBindings = bindings;
		
		VkResult result = vkCreateDescriptorSetLayout(vk::getDevice(), &info, nullptr, &m_layout); // TODO check result for errors

		delete[] bindings;
	}

	void DescriptorSet::allocate() {
		VkDescriptorPool pool = m_pRenderer->getDescriptorPool();
		ZP_ASSERT(pool, "Invalid DescriptorPool : DescriptorSet requires a valid DescriptorPool to be allocated");

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr};
		descriptorSetAllocateInfo.descriptorPool = pool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &m_layout;

		VkResult result = vkAllocateDescriptorSets(vk::getDevice(), &descriptorSetAllocateInfo, &m_descriptorSet); // TODO check result for errors
	}

	VkWriteDescriptorSet DescriptorSet::writeImage(const VkDescriptorImageInfo* pImageInfos, uint32_t imageCount, uint32_t binding) {
		bool hasSpace = m_bindings[binding].m_count < imageCount;
		ZP_WARN(hasSpace, "Binding has not enough space for all images, confirm image count and the bindings descriptor count match");

		VkWriteDescriptorSet write {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr};
		write.dstSet = m_descriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = m_bindings[binding].m_count;
		write.descriptorType = m_bindings[binding].m_type;
		write.pImageInfo = pImageInfos;

		return write;
	}
	VkWriteDescriptorSet DescriptorSet::writeImage(const VkDescriptorImageInfo& imageInfo, uint32_t binding) {
		return writeImage(&imageInfo, 1, binding);
	}

	VkWriteDescriptorSet DescriptorSet::writeBuffer(const VkDescriptorBufferInfo* pBufferInfos, uint32_t bufferCount, uint32_t binding) {
		bool hasSpace = m_bindings[binding].m_count < bufferCount;
		ZP_WARN(hasSpace, "Binding has not enough space for all buffers, confirm buffer count and the bindings descriptor count match");

		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr };
		write.dstSet = m_descriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = m_bindings[binding].m_count;
		write.descriptorType = m_bindings[binding].m_type;
		write.pBufferInfo = pBufferInfos;

		return write;
	}
	VkWriteDescriptorSet DescriptorSet::writeBuffer(const VkDescriptorBufferInfo& bufferInfo, uint32_t binding) {
		return writeBuffer(&bufferInfo, 1, binding);
	}

	void DescriptorSet::destroy() {
		VkDescriptorPool pool = m_pRenderer->getDescriptorPool();
		vkDestroyDescriptorSetLayout(vk::getDevice(), m_layout, nullptr);
		m_layout = VK_NULL_HANDLE;
		vkFreeDescriptorSets(vk::getDevice(), pool, 1, &m_descriptorSet);
		m_descriptorSet = VK_NULL_HANDLE;
	}

	VkDescriptorSetLayout DescriptorSet::getLayout() {
		return m_layout;
	}

	GenericDescriptorSet::GenericDescriptorSet(uint32_t size)
		: DescriptorSet(size)
	{}
}