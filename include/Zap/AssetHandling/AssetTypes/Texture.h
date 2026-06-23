#pragma once

#include "Zap/UUID.h"
#include "Zap/Rendering/Image.h"

#include "VulkanFramework.h"

namespace Zap {
	class Texture {
		friend class TextureLoader;
	public:
		Texture();
		~Texture();

		Image2D& getImage();

		void setIndex();

	private:
		Image2D m_image;
	};
}

