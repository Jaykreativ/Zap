#include "Zap/AssetHandling/FileLinker.h"

#include "Zap/Zap.h"

namespace Zap {
	FileLinker::FileLinker(){}
	FileLinker::~FileLinker(){}

	void FileLinker::registerAsset(UUID handle) {
		ZP_WARN(m_registryData.isActive, "Registering asset outside of Registry process | FileLinker::registerAsset");
		m_registryData.assetHandles.push_back(handle);
	}

	void FileLinker::endPathRegistry() {
		ZP_WARN(m_registryData.isActive, "Trying to end the registry process while inactive | FileLinker::beginPathRegistry");
		ZP_ASSERT(!isRegistered(m_registryData.path), "path is already registered | FileLinker::endPathRegistry");
		m_registeredPaths[m_registryData.path] = m_registryData.reconstructionData;
		for (UUID handle : m_registryData.assetHandles) {
			ZP_ASSERT(!hasSource(handle), "asset already present in another file | FileLinker::endPathRegistry");
			m_idToPath[handle] = m_registryData.path;
		}
		m_registryData = {};
	}

	void FileLinker::abortPathRegistry() {
		m_registryData = {};
	}

	bool FileLinker::isRegistered(std::filesystem::path path) {
		return m_registeredPaths.count(path);
	}

	std::weak_ptr<const ReconstructionData> FileLinker::getReconstructionData(std::filesystem::path path) {
		return m_registeredPaths.at(path);
	}

	bool FileLinker::hasSource(UUID handle) {
		return m_idToPath.count(handle);
	}

	std::filesystem::path FileLinker::getSourcePath(UUID handle) {
		return m_idToPath.at(handle);
	}
}