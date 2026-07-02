#include "Zap/AssetHandling/FileLinker.h"

#include "Zap/Zap.h"

namespace Zap {
	FileLinker::FileLinker(){}
	FileLinker::~FileLinker(){}

	bool FileLinker::isRegistered(std::filesystem::path path) {
		return m_registeredPaths.count(path);
	}

	bool FileLinker::hasSource(UUID handle) {
		return m_idToPath.count(handle);
	}

	std::filesystem::path FileLinker::getSourcePath(UUID handle) {
		return m_idToPath.at(handle);
	}
}