#include "Zap/AssetHandling/AssetTypes/Texture.h"

#include "Zap/Zap.h"

namespace Zap {
	Texture::Texture(Image2D image)
		: m_image(std::move(image))
	{}
	Texture::~Texture() {}

	//void Texture::load(void* data, uint32_t width, uint32_t height) {
	//	auto base = Base::getBase();
	//	Image* image = &base->m_assetHandler.getTextureDataPtr(m_handle)->image;
	//	image->setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
	//	image->setExtent({ width, height, 1 });
	//	image->setFormat(VK_FORMAT_R8G8B8A8_UNORM);
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
	//}

	Image2D& Texture::getImage() {
		return m_image;
	}
}
