#include "Zap/AssetHandling/FileLinker.h"

#include "Zap/Zap.h"

namespace Zap {
	FileLinker::FileLinker(){}
	FileLinker::~FileLinker(){}

	bool FileLinker::isLoaded(std::filesystem::path path) {
		return m_registeredPaths.count(path);
	}
}