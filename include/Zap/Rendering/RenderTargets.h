#pragma once

#include "Zap/Zap.h"

namespace Zap {
	class RenderTarget {
	public:
		RenderTarget();
		virtual ~RenderTarget();

		bool isValid();

	private:
		bool m_isValid = false;

		friend class Renderer;
	};

	// wrapper for a standart Zap image used by rendering
	class RenderTargetImage : public RenderTarget, public Image {
	public:
		RenderTargetImage();
		~RenderTargetImage();
	};

	// references a Zap window and allows rendering to it
	class RenderTargetWindow : public RenderTarget {
	public:
		RenderTargetWindow(Window& window);
		~RenderTargetWindow();

	private:
		Window& m_window;
	};
}