#include "Zap/Scene/Texture.h"

#include "Zap/Zap.h"

namespace Zap {
	Texture::Texture()
		: m_handle()
	{
		auto* base = Base::getBase();
		create();
	}

	Texture::Texture(UUID handle)
		: m_handle(handle)
	{}

	Texture::~Texture() {}

	bool Texture::isValid() {
		if(m_handle == 0) return false;
		return true;
	}

	void Texture::load(void* data, uint32_t width, uint32_t height) {
		auto base = Base::getBase();
		Image* image = &base->m_assetHandler.getTextureDataPtr(m_handle)->image;
		image->setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		image->setExtent({ width, height, 1 });
		image->setFormat(VK_FORMAT_R8G8B8A8_UNORM);
		image->setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		image->setType(VK_IMAGE_TYPE_2D);

		image->init();
		image->allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image->initView();

		image->changeLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);

		image->uploadData(width * height * 4, data);

		image->changeLayout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
	}

	void Texture::destroy() {
		auto& assetHandler = Base::getBase()->m_assetHandler;
		auto* pData = assetHandler.getTextureDataPtr(m_handle);
		destroy(pData);
	}
	void Texture::destroy(TextureData* data) {
		data->image.destroy();
	}

	bool Texture::exists() const {
		return Base::getBase()->m_assetHandler.m_textures.count(m_handle);
	}

	UUID Texture::getHandle() {
		return m_handle;
	}

	Image* Texture::getImage() {
		return &Base::getBase()->m_assetHandler.getTextureDataPtr(m_handle)->image;
	}
	const Image* Texture::getImage() const {
		return &Base::getBase()->m_assetHandler.getTextureData(m_handle).image;
	}

	void Texture::create() {
		auto base = Base::getBase();
		base->m_textureIndices[m_handle] = base->m_assetHandler.m_textures.size();
		base->m_assetHandler.addTexture(*this);
	}
}
