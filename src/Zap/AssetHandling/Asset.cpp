#include "Zap/AssetHandling/Asset.h"

namespace Zap {
	bool Asset::isGenerated() {
		return m_isGenerated;
	}

	void Asset::makeLoaded() {
		m_isGenerated = false;
	}
}