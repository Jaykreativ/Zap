#pragma once

#include "Zap/UUID.h"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace Zap {
	class ReconstructionData {};

	class FileLinker {
		FileLinker();
	public:
		~FileLinker();

		void registerAsset(UUID handle);

		template<class ReconstructionDataType>
		std::weak_ptr<ReconstructionDataType> beginPathRegistry(std::filesystem::path path) {
			ZP_WARN(!m_registryData.isActive, "Trying to begin the registry process while still active | FileLinker::beginPathRegistry");
			static_assert(std::is_base_of_v<RenderTarget, T>, "Type has to be child class of ReconstructionData | FileLinker::beginPathRegistry");

			m_registryData = {};
			m_registryData.isActive = true;
			m_registryData.path = path;
			m_registryData.reconstructionData = std::make_shared<ReconstructionDataType>();
		}

		void endPathRegistry();

		void abortPathRegistry();

		bool isRegistered(std::filesystem::path path);

		std::weak_ptr<const ReconstructionData> getReconstructionData(std::filesystem::path path);

		bool hasSource(UUID handle);

		std::filesystem::path getSourcePath(UUID handle);

	private:
		std::unordered_map<UUID, std::filesystem::path> m_idToPath;
		std::unordered_map<std::filesystem::path, std::shared_ptr<ReconstructionData>> m_registeredPaths;
	
		struct RegistryData {
			bool isActive = false;
			std::vector<UUID> assetHandles;
			std::shared_ptr<ReconstructionData> reconstructionData;
			std::filesystem::path path;
		} m_registryData;
	};
}