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

		bool isValid();

		/*
		* sets the texture image
		* takes data in the format R8G8B8A8
		*/
		void load(void* data, uint32_t width, uint32_t height);

		void destroy();
		static void destroy(TextureData* data);

		// removes asset from assetLibrary and destroys it
		// only works correctly with runtime generated assets
		void remove();

		bool exists() const;

		UUID getHandle();

		Image* getImage();
		const Image* getImage() const;

	private:
		UUID m_handle;

		void create();

		friend class TextureLoader;
	};
}

