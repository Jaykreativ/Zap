#pragma once

#include "Zap/UUID.h"
#include "VulkanFramework.h"

namespace Zap {
	typedef vk::Image Image;

	struct TextureData {
		Image image;
	};

	class Texture
	{
	public:
		Texture();
		Texture(UUID handle);
		~Texture();

		/*
		* sets the texture image
		* takes data in the format R8G8B8A8
		*/
		void load(void* data, uint32_t width, uint32_t height);

		void destroy();

		bool exists() const;

		UUID getHandle();

		Image* getImage();
		const Image* getImage() const;

	private:
		UUID m_handle;
	};
}

