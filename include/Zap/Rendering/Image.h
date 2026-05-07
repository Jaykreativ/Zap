#pragma once

#include "VulkanFramework.h"

namespace Zap {
	class ImageMipRange {
		ImageMipRange(uint32_t baseMipLevel, uint32_t levelCount);
	private:
		uint32_t m_baseMipLevel;
		uint32_t m_levelCount;

		friend class ImageSubresourceRange;
	};

	class ImageArrayRange {
		ImageArrayRange(uint32_t baseArrayLayer, uint32_t layerCount);
	private:
		uint32_t m_baseArrayLayer;
		uint32_t m_layerCount;

		friend class ImageSubresourceRange;
	};

	class ImageSubresourceRange {
	public:
		ImageSubresourceRange(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, ImageMipRange mipRange = { 0, 1 }, ImageArrayRange arrayRange = { 0, 1 });

		operator VkImageSubresourceRange() const;

		VkImageSubresourceLayers getLayers(uint32_t mipLevel, uint32_t baseLayer, uint32_t layerCount) const;

	private:
		VkImageAspectFlags m_aspectMask;
		ImageMipRange      m_mipRange;
		ImageArrayRange    m_arrayRange;
	};

	// A vulkan image
	// Always defaults to VK_IMAGE_LAYOUT_GENERAL outside of renderTasks
	class Image {
	public:
		VkExtent3D getExtent();

		operator VkImage();
		operator VkImageView();

		VkImage getVkImage();
		VkImageView getVkImageView();

		VkImageSubresourceRange getSubresourceRange();

	protected:
		Image(
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
			VkImageLayout           layout = VK_IMAGE_LAYOUT_GENERAL
		);
		virtual ~Image();
		Image(const Image& other);
		// resize constructor
		Image(const Image& other, VkExtent3D extent);
		Image& operator= (const Image& other);
		Image(Image&& other) noexcept;
		Image& operator= (Image&& other) noexcept;

		Image();
		friend void swap(Image& first, Image& second);

		void copy(const Image& src, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t regionCount, const VkImageCopy2* pRegions);

		void cmdChangeLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAcccessMask);

		// meta data
		VkImageType             m_imageType;
		VkFormat                m_format;
		VkExtent3D              m_extent;
		uint32_t                m_mipLevels;
		uint32_t                m_arrayLayers;
		VkSampleCountFlagBits   m_samples;
		VkImageTiling           m_tiling;
		VkImageUsageFlags       m_usage;
		VkSharingMode           m_sharingMode;
		uint32_t                m_queueFamilyIndexCount;
		const uint32_t*         m_pQueueFamilyIndices;
		VkMemoryPropertyFlags   m_memoryProperties;
		VkImageViewType         m_viewType;
		VkComponentMapping      m_components;
		VkImageLayout           m_layout; // accurate only for static layout images
	private:
		// core resources
		VkImage m_image;
		VkDeviceMemory m_deviceMemory;
		VkImageView m_imageView;
	};

	class Image2DBase : public Image {
	protected:
		Image2DBase(
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
			VkImageLayout           layout = VK_IMAGE_LAYOUT_GENERAL
		);
		virtual ~Image2DBase();
		Image2DBase(const Image2DBase& other);
		Image2DBase(const Image2DBase& other, VkExtent2D extent);
		Image2DBase& operator= (const Image2DBase& other);
		Image2DBase(Image2DBase&& other) noexcept;
		Image2DBase& operator= (Image2DBase&& other) noexcept;

		Image2DBase();
		friend void swap(Image2DBase& first, Image2DBase& second);
	};

	class Image2D : public Image2DBase {
	public:
		Image2D(
			VkFormat                format,
			VkExtent2D              extent,
			VkImageUsageFlags       usage,
			VkMemoryPropertyFlags   memoryProperties,
			VkImageLayout           layout = VK_IMAGE_LAYOUT_GENERAL // allow RenderTasks to change initial layout
		);
		virtual ~Image2D();
		Image2D(const Image2D& other);
		Image2D(const Image2D& other, VkExtent2D extent);
		Image2D& operator= (const Image2D& other);
		Image2D(Image2D&& other) noexcept;
		Image2D& operator= (Image2D&& other) noexcept;

		Image2D();
		friend void swap(Image2D& first, Image2D& second);

	protected:
		void uploadData(size_t size, void* data);

		void cmdCopyFromBuffer(VkCommandBuffer cmd, vk::Buffer& src, VkImageLayout layout);

		friend class Image2DLoader;
	};

	// RenderTargetImage : Image2D (in RenderTargets.h)
}