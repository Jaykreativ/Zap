#include "Zap/Rendering/Image.h"

#include "VulkanUtils.h"

#include "Zap/Zap.h"

VkImageAspectFlags getAspectFromUsage(VkImageUsageFlags usage) {
	if (ZP_IS_FLAG_ENABLED(usage, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		return VK_IMAGE_ASPECT_COLOR_BIT;
}

namespace Zap {
	ImageMipRange::ImageMipRange(uint32_t baseMipLevel, uint32_t levelCount)
		: m_baseMipLevel(baseMipLevel), m_levelCount(levelCount)
	{}

	ImageArrayRange::ImageArrayRange(uint32_t baseArrayLayer, uint32_t layerCount)
		: m_baseArrayLayer(baseArrayLayer), m_layerCount(layerCount)
	{}

	ImageSubresourceRange::ImageSubresourceRange(VkImageAspectFlags aspectMask, ImageMipRange mipRange, ImageArrayRange arrayRange)
		: m_aspectMask(aspectMask), m_mipRange(mipRange), m_arrayRange(arrayRange)
	{}

	ImageSubresourceRange::operator VkImageSubresourceRange() const {
		VkImageSubresourceRange range;
		range.aspectMask     = m_aspectMask;
		range.baseMipLevel   = m_mipRange.m_baseMipLevel;
		range.levelCount     = m_mipRange.m_levelCount;
		range.baseArrayLayer = m_arrayRange.m_baseArrayLayer;
		range.layerCount     = m_arrayRange.m_layerCount;
		return range;
	}

	VkImageSubresourceLayers ImageSubresourceRange::getLayers(uint32_t mipLevel, uint32_t baseLayer, uint32_t layerCount) const {
		VkImageSubresourceLayers layers;
		layers.aspectMask = m_aspectMask;
		layers.mipLevel = mipLevel;
		layers.baseArrayLayer = baseLayer;
		layers.layerCount = layerCount;
		return layers;
	}

	// layout transitions:
	// Inital layout                               |: Default to Layout_General (RenderTasks are allowed to change this, copying will revert back to Layout_General)
	// Renderer(Outside -> RenderTask -> Outside)  |: Use Layout_General for Outside
	// DescriptorSet( Texture, GuiImage )          |: Default to Layout_General
	// TODO find more optimal way to organize layout transitions, outside and inside RenderTasks

	Image::Image(
		VkImageType             imageType,
		VkFormat                format,
		VkExtent3D              extent,
		uint32_t                mipLevels,
		uint32_t                arrayLayers,
		VkSampleCountFlagBits   samples,
		VkImageTiling           tiling,
		VkImageUsageFlags       usage,
		VkSharingMode           sharingMode,
		uint32_t                queueFamilyIndexCount,
		const uint32_t*         pQueueFamilyIndices,
		VkMemoryPropertyFlags   memoryProperties,
		VkImageViewType         viewType,
		VkComponentMapping      components,
		VkImageLayout           layout
	) :
		m_imageType(imageType),
		m_format(format),
		m_extent(extent),
		m_mipLevels(mipLevels),
		m_arrayLayers(arrayLayers),
		m_samples(samples),
		m_tiling(tiling),
		m_usage(usage),
		m_sharingMode(sharingMode),
		m_queueFamilyIndexCount(queueFamilyIndexCount),
		m_pQueueFamilyIndices(pQueueFamilyIndices),
		m_memoryProperties(memoryProperties),
		m_viewType(viewType),
		m_components(components),
		m_layout(layout)
	{
		VkImageCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0};
		createInfo.imageType = imageType;
		createInfo.format = format;
		createInfo.extent = extent;
		createInfo.mipLevels = mipLevels;
		createInfo.arrayLayers = arrayLayers;
		createInfo.samples = samples;
		createInfo.tiling = tiling;
		createInfo.usage = usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.sharingMode = sharingMode;
		createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
		createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		vkCreateImage(vk::getDevice(), &createInfo, nullptr, &m_image);
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(vk::getDevice(), m_image, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr };
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = vkUtils::findMemoryTypeIndex(vk::getPhysicalDevice(), memoryRequirements.memoryTypeBits, memoryProperties);

		vkAllocateMemory(vk::getDevice(), &allocateInfo, nullptr, &m_deviceMemory);

		vkBindImageMemory(vk::getDevice(), m_image, m_deviceMemory, 0);

		VkImageViewCreateInfo viewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0 };
		viewCreateInfo.image = m_image;
		viewCreateInfo.viewType = viewType;
		viewCreateInfo.format = format;
		viewCreateInfo.components = components;
		viewCreateInfo.subresourceRange = getSubresourceRange();

		vkCreateImageView(vk::getDevice(), &viewCreateInfo, nullptr, &m_imageView);
		
		vk::CommandBuffer cmd(true);
		cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		cmdChangeLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, layout, 0, 0);
		cmd.end();
		cmd.submit();
		cmd.free();
	}

	Image::~Image() {
		if(m_imageView)
			vkDestroyImageView(vk::getDevice(), m_imageView, nullptr);
		if(m_deviceMemory)
			vkFreeMemory(vk::getDevice(), m_deviceMemory, nullptr);
		if(m_image)
			vkDestroyImage(vk::getDevice(), m_image, nullptr);
	}

	// swap logic for move
	Image::Image()
		: m_image(VK_NULL_HANDLE), m_deviceMemory(VK_NULL_HANDLE), m_imageView(VK_NULL_HANDLE)
	{}

	Image::Image(const Image& other)
		: Image(
			other.m_imageType,
			other.m_format,
			other.m_extent,
			other.m_mipLevels,
			other.m_arrayLayers,
			other.m_samples,
			other.m_tiling,
			other.m_usage,
			other.m_sharingMode,
			other.m_queueFamilyIndexCount,
			other.m_pQueueFamilyIndices,
			other.m_memoryProperties,
			other.m_viewType,
			other.m_components
		)
	{
		std::vector<VkImageCopy2> copyInfos(m_mipLevels);
		size_t i = 0;
		for (VkImageCopy2& copyInfo : copyInfos) { // copy the entire image with all mip levels and array layers
			copyInfo.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
			copyInfo.pNext = nullptr;
			VkImageSubresourceLayers subresourceLayers;
			subresourceLayers.aspectMask = getAspectFromUsage(m_usage);
			subresourceLayers.mipLevel = i;
			subresourceLayers.baseArrayLayer = 0;
			subresourceLayers.layerCount = m_arrayLayers;
			copyInfo.srcSubresource = subresourceLayers;
			copyInfo.srcOffset = {0, 0, 0};
			copyInfo.dstSubresource = subresourceLayers;
			copyInfo.dstOffset = {0, 0, 0};
			copyInfo.extent = m_extent;
			i++;
		}
		vk::CommandBuffer cmd(true);
		cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		cmdCopy(cmd, other, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, copyInfos.size(), copyInfos.data());
		cmd.end();
		cmd.submit(); cmd.free();
	}

	Image::Image(const Image& other, VkExtent3D extent)
		: Image(
			other.m_imageType,
			other.m_format,
			extent,
			other.m_mipLevels,
			other.m_arrayLayers,
			other.m_samples,
			other.m_tiling,
			other.m_usage,
			other.m_sharingMode,
			other.m_queueFamilyIndexCount,
			other.m_pQueueFamilyIndices,
			other.m_memoryProperties,
			other.m_viewType,
			other.m_components,
			other.m_layout
		)
	{}

	Image& Image::operator= (const Image& other) {
		auto copy = other;
		swap(*this, copy);
		return *this;
	}

	Image::Image(Image&& other) noexcept {
		swap(*this, other);
	}

	Image& Image::operator= (Image&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	void swap(Image& first, Image& second) {
		using std::swap;
		swap(first.m_imageType,             second.m_imageType);
		swap(first.m_format,                second.m_format);
		swap(first.m_extent,                second.m_extent);
		swap(first.m_mipLevels,             second.m_mipLevels);
		swap(first.m_arrayLayers,           second.m_arrayLayers);
		swap(first.m_samples,               second.m_samples);
		swap(first.m_tiling,                second.m_tiling);
		swap(first.m_usage,                 second.m_usage);
		swap(first.m_sharingMode,           second.m_sharingMode);
		swap(first.m_queueFamilyIndexCount, second.m_queueFamilyIndexCount);
		swap(first.m_pQueueFamilyIndices,   second.m_pQueueFamilyIndices);
		swap(first.m_memoryProperties,      second.m_memoryProperties);
		swap(first.m_viewType,              second.m_viewType);
		swap(first.m_components,            second.m_components);

		swap(first.m_image,                 second.m_image);
		swap(first.m_deviceMemory,          second.m_deviceMemory);
		swap(first.m_imageView,             second.m_imageView);
	}

	void Image::cmdCopy(VkCommandBuffer cmd, const Image& src, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t regionCount, const VkImageCopy2* pRegions) {
		VkCopyImageInfo2 copyInfo{ VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2, nullptr };
		copyInfo.srcImage = src.m_image;
		copyInfo.srcImageLayout = srcLayout;
		copyInfo.dstImage = m_image;
		copyInfo.dstImageLayout = dstLayout;
		copyInfo.regionCount = regionCount;
		copyInfo.pRegions = pRegions;
		vkCmdCopyImage2(cmd, &copyInfo);
	}

	void Image::cmdChangeLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr };
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_image;
		imageMemoryBarrier.subresourceRange = getSubresourceRange();
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	VkExtent3D Image::getExtent() {
		return m_extent;
	}

	Image::operator VkImage() {
		return getVkImage();
	}

	Image::operator VkImageView() {
		return getVkImageView();
	}

	VkImage Image::getVkImage() {
		return m_image;
	}

	VkImageView Image::getVkImageView() {
		return m_imageView;
	}

	VkImageSubresourceRange Image::getSubresourceRange() {
		VkImageSubresourceRange subresourceRange; // subresource covers the entire image as the image and its view are 1:1
		subresourceRange.aspectMask = getAspectFromUsage(m_usage);
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_mipLevels;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = m_arrayLayers;
		return subresourceRange;
	}

	Image2DBase::Image2DBase(
		VkFormat                format,
		VkExtent2D              extent,
		uint32_t                mipLevels,
		uint32_t                arrayLayers,
		VkSampleCountFlagBits   samples,
		VkImageTiling           tiling,
		VkImageUsageFlags       usage,
		VkSharingMode           sharingMode,
		uint32_t                queueFamilyIndexCount,
		const uint32_t*         pQueueFamilyIndices,
		VkMemoryPropertyFlags   memoryProperties,
		VkComponentMapping      components,
		VkImageLayout           layout
	)
		: Image(
			VK_IMAGE_TYPE_2D,
			format,
			{extent.width, extent.height, 1},
			mipLevels,
			arrayLayers,
			samples,
			tiling,
			usage,
			sharingMode,
			queueFamilyIndexCount,
			pQueueFamilyIndices,
			memoryProperties,
			VK_IMAGE_VIEW_TYPE_2D,
			components,
			layout
		)
	{}

	Image2DBase::~Image2DBase() {}

	Image2DBase::Image2DBase(const Image2DBase& other)
		: Image(other)
	{}

	Image2DBase::Image2DBase(const Image2DBase& other, VkExtent2D extent)
		: Image(other, { extent.width, extent.height, 1 })
	{}

	Image2DBase& Image2DBase::operator= (const Image2DBase& other) {
		auto copy = other;
		swap(*this, copy);
		return *this;
	}

	Image2DBase::Image2DBase(Image2DBase&& other) noexcept {
		swap(*this, other);
	}

	Image2DBase& Image2DBase::operator= (Image2DBase&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	Image2DBase::Image2DBase(){}

	void swap(Image2DBase& first, Image2DBase& second){
		swap((Image&)first, (Image&)second);
	}

	Image2D::Image2D(
		VkFormat                format,
		VkExtent2D              extent,
		VkImageUsageFlags       usage,
		VkMemoryPropertyFlags   memoryProperties,
		VkImageLayout layout
	) : Image2DBase(
		format,
		extent,
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		memoryProperties,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY, // r
			VK_COMPONENT_SWIZZLE_IDENTITY, // g
			VK_COMPONENT_SWIZZLE_IDENTITY, // b
			VK_COMPONENT_SWIZZLE_IDENTITY, // a
		},
		layout
	) {}

	Image2D::~Image2D() {}

	Image2D::Image2D(const Image2D& other)
		: Image2DBase(other)
	{}

	Image2D::Image2D(const Image2D& other, VkExtent2D extent)
		: Image2DBase(other, extent)
	{}

	Image2D& Image2D::operator= (const Image2D& other) {
		auto copy = other;
		swap(*this, copy);
		return *this;
	}

	Image2D::Image2D(Image2D&& other) noexcept {
		swap(*this, other);
	}

	Image2D& Image2D::operator= (Image2D&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	Image2D::Image2D() {}

	void swap(Image2D& first, Image2D& second) {
		swap((Image2DBase&)first, (Image2DBase&)second);
	}

	void Image2D::uploadData(size_t size, void* data) {
		vk::CommandBuffer cmd = vk::CommandBuffer(true);
		cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		cmdChangeLayout(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);

		if (!VK_IS_FLAG_ENABLED(m_usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			std::cerr << "Image cant be destination of upload transfer: enable VK_IMAGE_USAGE_TRANSFER_DST_BIT\n";
			throw std::runtime_error("Image cant be destination of upload transfer");
		}
		vk::Buffer stagingBuffer = vk::Buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		stagingBuffer.init();
		stagingBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* rawData;
		stagingBuffer.map(&rawData);
		memcpy(rawData, data, size);
		stagingBuffer.unmap();

		cmdCopyFromBuffer(cmd, stagingBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		cmdChangeLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT, 0);

		cmd.end();
		cmd.submit();
		cmd.free();
		stagingBuffer.destroy(); // can only be destroyed after commands using the buffer have been submitted and finished
	}

	void Image2D::cmdCopyFromBuffer(VkCommandBuffer cmd, vk::Buffer& src, VkImageLayout layout) {
		VkBufferImageCopy bufferImageCopy;
		bufferImageCopy.bufferOffset = 0;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		VkImageSubresourceLayers subresourceLayers;
		subresourceLayers.aspectMask = getAspectFromUsage(m_usage);
		subresourceLayers.mipLevel = 0;
		subresourceLayers.baseArrayLayer = 0;
		subresourceLayers.layerCount = 1;
		bufferImageCopy.imageSubresource = subresourceLayers;
		bufferImageCopy.imageOffset = { 0, 0, 0 };
		bufferImageCopy.imageExtent = m_extent;

		vkCmdCopyBufferToImage(cmd, src, *this, layout, 1, &bufferImageCopy);
	}
}
