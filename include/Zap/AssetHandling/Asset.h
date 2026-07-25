#pragma once

#include "Zap/UUID.h"

#include <filesystem>

namespace Zap {
	class AssetHandler;

	// Assets are stored in external files like .obj/.glb
	class Asset {
	public:
		Asset() {}
		virtual ~Asset() {}

		bool isGenerated();

		bool hasSourcePath();

		std::filesystem::path getSourcePath();
	private:
		void makeLoaded(std::filesystem::path path);

		std::unique_ptr<std::filesystem::path> m_sourcePath = nullptr;

		friend class FileLinker;
	};

	template<class T = Asset>
	class AssetHandle {
		template<class U>
		friend class AssetIterator;
		friend class AssetHandler;
	public:
		AssetHandle() {}
		AssetHandle(const AssetHandle<T>& other)
			: m_handle(other.m_handle), m_pAssetHandler(other.m_pAssetHandler)
		{}
		~AssetHandle() {}

		operator AssetHandle<Asset>() {
			return AssetHandle<Asset>(m_handle, m_pAssetHandler);
		}

		operator UUID() { return m_handle; }

		T* get() {
			return m_pAssetHandler->getAsset<T>(m_handle);
		}
		const T* get() const {
			return m_pAssetHandler->getAsset<T>(m_handle);
		}

		T* operator->() {
			return get();
		}

		operator bool() const {
			return m_pAssetHandler != nullptr && get() != nullptr;
		}

		void reset() {
			m_handle = 0;
			m_pAssetHandler = nullptr;
		}

	private:
		UUID m_handle = 0;
		AssetHandler* m_pAssetHandler = nullptr;

		AssetHandle(UUID handle, AssetHandler* pAssetHandler)
			: m_handle(handle), m_pAssetHandler(pAssetHandler)
		{}

		// serialization access
		friend class Serializer;
	};
}