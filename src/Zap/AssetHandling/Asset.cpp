#include "Zap/AssetHandling/Asset.h"

namespace Zap {
	bool Asset::isLoaded() {
		return !m_filepath.empty();
	}

	void Asset::storePath(std::filesystem::path filepath) {
		m_filepath = filepath;
	}
}