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

		template<class ReconstructionDataType>
		class FileLink {
			FileLink(std::filesystem::path path)
				: m_path(path)
			{
				m_reconstructionData = std::make_shared<ReconstructionDataType>();
			}
		public:
			~FileLink(){}

			ReconstructionDataType* operator->() {
				return m_reconstructionData.get();
			}

			std::shared_ptr<ReconstructionDataType> getReconstructionData() {
				return m_reconstructionData;
			}

			void registerAsset(UUID handle) {
				m_assetHandles.push_back(handle);
			}
		private:
			bool m_isValid = true;
			std::vector<UUID> m_assetHandles;
			std::shared_ptr<ReconstructionDataType> m_reconstructionData;
			std::filesystem::path m_path;

			friend class FileLinker;
		};

		template<class ReconstructionDataType>
		FileLink<ReconstructionDataType> beginFileLinking(std::filesystem::path path) {
			static_assert(std::is_base_of_v<ReconstructionData, ReconstructionDataType>, "Type has to be child class of ReconstructionData | FileLinker::beginFileLinking");
			return FileLink<ReconstructionDataType>(path);
		}

		template<class ReconstructionDataType>
		void endFileLinking(FileLink<ReconstructionDataType>& fileLink) {
			ZP_WARN(fileLink.m_isValid, "Trying to end the registry using an invalid FileLink | FileLinker::beginFileLinking");
			ZP_ASSERT(!isRegistered(fileLink.m_path), "path is already registered | FileLinker::endFileLinking");
			m_registeredPaths[fileLink.m_path] = fileLink.m_reconstructionData;
			for (UUID handle : fileLink.m_assetHandles) {
				ZP_ASSERT(!hasSource(handle), "asset already present in another file | FileLinker::endFileLinking");
				m_idToPath[handle] = fileLink.m_path;
			}
			abortFileLinking(fileLink);
		}

		template<class ReconstructionDataType>
		void abortFileLinking(FileLink<ReconstructionDataType>& fileLink) {
			fileLink.m_isValid = false;
			fileLink.m_assetHandles.clear();
			fileLink.m_path.clear();
			fileLink.m_reconstructionData.reset();
		}

		bool isRegistered(std::filesystem::path path);

		template<class ReconstructionDataType>
		std::shared_ptr<const ReconstructionDataType> getReconstructionData(std::filesystem::path path) {
			return std::reinterpret_pointer_cast<ReconstructionDataType>(m_registeredPaths.at(path));
		}

		bool hasSource(UUID handle);

		std::filesystem::path getSourcePath(UUID handle);

	private:
		std::unordered_map<UUID, std::filesystem::path> m_idToPath;
		std::unordered_map<std::filesystem::path, std::shared_ptr<ReconstructionData>> m_registeredPaths;

		friend class AssetHandler;
	};
}