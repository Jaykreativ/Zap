#pragma once

#define ZP_ASSET_METADATA_FILE_EXTENSION ".zamd"

#include "Zap/UUID.h"
#include "Zap/Serializer.h"
#include "Zap/AssetHandling/Asset.h"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace Zap {
	class ReconstructionData {
		friend class FileLinker;
	};

	class FileLinker {
		FileLinker();
	public:
		~FileLinker();

		bool isLoaded(std::filesystem::path path);

		template<class ReconstructionDataType>
		std::shared_ptr<ReconstructionDataType> getReconstructionData(std::filesystem::path path) {
			return std::reinterpret_pointer_cast<ReconstructionDataType>(m_registeredPaths.at(path));
		}

		template<class ReconstructionDataType>
		class FileLink {
			FileLink(std::filesystem::path path)
				: m_path(path), m_isLoaded(false)
			{
				static_assert(std::is_base_of_v<ReconstructionData, ReconstructionDataType>, "Type has to be subclass to ReconstructionData | FileLinker::FileLink::FileLink");
				m_reconstructionData = std::make_shared<ReconstructionDataType>();

				std::ifstream file(m_path.string() + ZP_ASSET_METADATA_FILE_EXTENSION);
				if (file.is_open()) { // check for existing metadata file (ReconstructionData file)
					Serializer::readReadable(*m_reconstructionData, file);
					file.close();
					m_isRegistered = true;
				}
				else {
					m_isRegistered = false;
				}
			}
			FileLink(std::filesystem::path path, std::shared_ptr<ReconstructionDataType> reconstructionData)
				: m_path(path), m_reconstructionData(reconstructionData), m_isLoaded(true)
			{}
		public:
			~FileLink(){}

			ReconstructionDataType* operator->() {
				return m_reconstructionData.get();
			}

			std::shared_ptr<ReconstructionDataType> getReconstructionData() {
				return m_reconstructionData;
			}

			template<class T>
			void registerAsset(AssetHandle<T> handle){
				handle->makeLoaded();
			}

			bool isLoaded() { return m_isLoaded; }
			bool isRegistered() { return m_isRegistered; }

			bool hasSourceFileChanged(){
				return true;
			} // placeholder for future file sync systems

			void finishLinking() {
				if (isLoaded())
					return;
				if (isRegistered() && !hasSourceFileChanged())
					return;
				// write to metadata file (ReconstructionData file)
				std::ofstream file(m_path.string() + ZP_ASSET_METADATA_FILE_EXTENSION);
				if(file.good())
					Serializer::writeReadable(*m_reconstructionData, file);
				file.close();
			}

		private:
			bool m_isValid = true;
			bool m_isLoaded = false;
			bool m_isRegistered = false;
			std::shared_ptr<ReconstructionDataType> m_reconstructionData;
			std::filesystem::path m_path;

			friend class FileLinker;
		};

		template<class ReconstructionDataType>
		FileLink<ReconstructionDataType> beginFileLinking(std::filesystem::path path) {
			static_assert(std::is_base_of_v<ReconstructionData, ReconstructionDataType>, "Type has to be child class of ReconstructionData | FileLinker::beginFileLinking");
			if(isLoaded(path))
				return FileLink<ReconstructionDataType>(path, getReconstructionData<ReconstructionDataType>(path));
			else
				return FileLink<ReconstructionDataType>(path);
		}

		template<class ReconstructionDataType>
		void endFileLinking(FileLink<ReconstructionDataType>& fileLink) {
			ZP_WARN(fileLink.m_isValid, "Trying to end the registry using an invalid FileLink | FileLinker::beginFileLinking");
			fileLink.finishLinking();
			if (!fileLink.isLoaded()) {
				m_registeredPaths[fileLink.m_path] = fileLink.m_reconstructionData;
			}
			abortFileLinking(fileLink);
		}

		template<class ReconstructionDataType>
		void abortFileLinking(FileLink<ReconstructionDataType>& fileLink) {
			fileLink.m_isValid = false;
			fileLink.m_path.clear();
			fileLink.m_reconstructionData.reset();
		}

	private:
		std::unordered_map<std::filesystem::path, std::shared_ptr<ReconstructionData>> m_registeredPaths;

		friend class AssetHandler;
	};
}