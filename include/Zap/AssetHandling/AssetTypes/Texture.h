#pragma once

#include "Zap/UUID.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/Rendering/Image.h"

#include "VulkanFramework.h"

namespace Zap {
	class Texture : public Asset {
	public:
		Texture(
			Image2D image
		);
		~Texture();

		Image2D& getImage();

	private:
		Image2D m_image;
	};
}

