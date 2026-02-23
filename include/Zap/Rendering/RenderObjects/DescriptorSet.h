#pragma once

#include "Zap/Zap.h"
#include "Zap/Rendering/RenderObject.h"
#include "Zap/Rendering/RenderObjects/RenderTargets.h"

namespace Zap {
// --- Vulkan DescriptorSet Logic ---
// 
// Layout: describe bindings | (( stage, type, count ) for each binding ) -> VkDescriptorSetLayout
// Allocate: allocate memory from the pool | ( descriptorPool, layout ) -> VkDescriptorSet | can be done in bulk
// Update/Write: writes the description of the image/buffer | ((descriptorSet, binding, count, type, image/buffer info ) for each binding ) -> void
// Destroy: frees the descriptorSet and destroys the layout | (descritptorSet, layout) -> void
//
// ----------------------------------

	// Stores the descriptors the descriptorPool has to allocate
	class DescriptorPoolSizeList {
	public:
		void addPoolSize(VkDescriptorType type, uint32_t size) {
			m_poolSizes.push_back({ type, size });
		}

		void addSets(uint32_t count) {
			m_maxSets += count;
		}

	private:
		std::vector<VkDescriptorPoolSize> m_poolSizes = {};
		uint32_t m_maxSets = 0;

		friend class Renderer;
	};

	class DescriptorSetBinding {
	public:
		DescriptorSetBinding() = default;
		DescriptorSetBinding(VkDescriptorType type, VkShaderStageFlags stages, uint32_t count = 1);

	private:
		uint32_t           m_binding = 0;
		VkDescriptorType   m_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		VkShaderStageFlags m_stages = 0;
		uint32_t           m_count = 0;

		friend class DescriptorSet;
	};

	class DescriptorPool {
	public:
		DescriptorPool();
		DescriptorPool(const DescriptorPool& other);
		~DescriptorPool();

		operator VkDescriptorPool() const { return m_descriptorPool; }

		void init();

		// sets the max amount of sets for this pool
		void setMaxSets(uint32_t maxSets);

		// gets the max amount of sets for this pool
		size_t getMaxSets();

		// a poolSize describes the type and the amount of descriptors to allocate in the pool
		void addPoolSize(VkDescriptorType type, uint32_t count);
		void addPoolSize(VkDescriptorPoolSize poolSize);
		void addPoolSizes(VkDescriptorPoolSize* poolSize, uint32_t poolSizeCount);

	private:
		bool m_isInit = false;

		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

		uint32_t m_maxSets = 0;
		std::vector<VkDescriptorPoolSize> m_poolSizes = {};
	};

	class DescriptorSet : public RenderObject {
	public:
		DescriptorSet(Renderer* pRenderer);
		virtual ~DescriptorSet();

		virtual operator VkDescriptorSet() { return m_descriptorSet; }

		void addBinding(const DescriptorSetBinding& binding);
		
		// uses just the bindings to create a layout
		// all bindings have to be added before calling this
		void createLayout();

		// allocates the layout in the given descriptor pool
		void allocate();

		// generates a write which contains multiple images for one binding, imageCount must match the descriptorCount of the given binding
		VkWriteDescriptorSet writeImage(const VkDescriptorImageInfo* pImageInfos, uint32_t imageCount, uint32_t binding);
		VkWriteDescriptorSet writeImage(const VkDescriptorImageInfo& imageInfo, uint32_t binding = 0);

		// generates a write which contains multiple buffers for one binding, bufferCount must match the descriptorCount of the given binding
		VkWriteDescriptorSet writeBuffer(const VkDescriptorBufferInfo* pBufferInfos, uint32_t bufferCount, uint32_t binding);
		VkWriteDescriptorSet writeBuffer(const VkDescriptorBufferInfo& bufferInfo, uint32_t binding = 0);

		VkWriteDescriptorSet writeGeneric(void* pNext, uint32_t count, uint32_t binding);

		static void write(uint32_t writeCount, VkWriteDescriptorSet* pWrites) {
			vkUpdateDescriptorSets(vk::getDevice(), writeCount, pWrites, 0, nullptr); // TODO check result for errors
		}

		VkDescriptorSetLayout getLayout();

	private:
		std::vector<DescriptorSetBinding> m_bindings = {};

		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;

		void destroy();

		friend class Renderer;
	};

	template<class T = DescriptorSet>
	class DescriptorSetHandle {
		friend class Renderer;
		template<class U>
		friend class DescriptorSetHandle;
	public:
		DescriptorSetHandle() = default;
		DescriptorSetHandle(const DescriptorSetHandle<T>& other)
			: m_handle(other.m_handle), m_renderer(other.m_renderer)
		{}
		~DescriptorSetHandle() = default;

		operator VkDescriptorSet() { return *get(); }

		operator DescriptorSetHandle<DescriptorSet>() {
			return DescriptorSetHandle<DescriptorSet>(m_handle, m_renderer);
		}

		T* get() {
			return reinterpret_cast<T*>(m_renderer->getDescriptorSet(m_handle));
		}
		const T* get() const {
			return reinterpret_cast<T*>(m_renderer->getDescriptorSet(m_handle));
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_renderer != nullptr && get() != nullptr;
		}

		void reset() {
			m_handle = 0;
			m_renderer = nullptr;
		}

	private:
		UUID m_handle = 0;
		Renderer* m_renderer = nullptr;

		DescriptorSetHandle(UUID handle, Renderer* renderer)
			: m_handle(handle), m_renderer(renderer)
		{}
	};

	class GenericDescriptorSet : public DescriptorSet {
	public:
		GenericDescriptorSet(Renderer* pRenderer);

		friend class Renderer;
	};

	class RenderTargetDescriptorSet : public DescriptorSet,
		public EventListener<RenderEvent::Resize>
	{
	public:
		RenderTargetDescriptorSet(Renderer* pRenderer, RenderTargetHandle<> target, VkShaderStageFlags stages);
		virtual ~RenderTargetDescriptorSet();

		operator VkDescriptorSet() override;

		void write();

	private:
		RenderTargetHandle<> m_target;
		std::vector<DescriptorSetHandle<GenericDescriptorSet>> m_additionalSets = {};

		void callback(const RenderEvent::Resize& event) override;

		static void initImageDescriptorSet(DescriptorSet* pDescriptorSet, VkShaderStageFlags stages);

		static void writeImageDescriptorSet(DescriptorSet* pDescriptorSet, VkImageView view, VkImageLayout layout);

		friend class Renderer;
	};

}