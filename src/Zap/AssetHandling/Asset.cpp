#include "Zap/AssetHandling/Asset.h"

namespace Zap {
	bool Asset::isGenerated() {
		return !hasSourcePath();
	}

	bool Asset::hasSourcePath() {
		return m_sourcePath.operator bool();
	}

	std::filesystem::path Asset::getSourcePath() {
		return *m_sourcePath;
	}

	void Asset::makeLoaded(std::filesystem::path path) {
		m_sourcePath = std::make_unique<std::filesystem::path>(path);
	}
}