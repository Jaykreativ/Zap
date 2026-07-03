#pragma once

#include "Zap/UUID.h"
#include "Zap/Events.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/FileLinker.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"
#include "Zap/AssetHandling/AssetTypes/Texture.h"
#include "Zap/AssetHandling/AssetTypes/HitMesh.h"

#include <set>
#include <filesystem>
/*
* FileLoader parent
* Actor loader
* Scene loader
* Model loader -> loads a model, if the model is not present in the assetlibrary it gets added to it. has unload function for optimization
* etc.
* 
* Assets
*	-assetLib.zal
*	Models
*	Textures
*	etc.
* Actors
*	-actor1.zac
*	-actor2.zac
*	etc.
* 
* assetLib has all assets: meshes, materials, textures
* keeps track of the load status and dynamically allocates and destroys assets
* assets can be created and stored during runtime, those assets are not saved to file
* contents of the assetlibrary are controlled by the asset handler, not the user
* 
* Generated/Added assets are all assets that are handled by the AssetHandler
* Loaded assets have some sort of file associated with it
*/

namespace Zap {

	class TextureLoadEvent {
	public:
		TextureLoadEvent(AssetHandle<Texture> texture)
			: texture(texture)
		{}
		~TextureLoadEvent() = default;

		AssetHandle<Texture> texture;
	};

	template<class T>
	class AssetIterator {
		friend class AssetHandler;
	public:
		~AssetIterator(){}

		void operator++(int) {
			m_iterator++;
		}

		AssetHandle<T> operator*() {
			return AssetHandle<T>((*m_iterator).first, m_pAssetHandler);
		}

		bool operator==(AssetIterator<T> other) {
			return m_iterator == other.m_iterator;
		}
		bool operator!=(AssetIterator<T> other) {
			return m_iterator != other.m_iterator;
		}

	private:
		AssetHandler* m_pAssetHandler = nullptr;
		typename std::unordered_map<UUID, std::unique_ptr<T>>::iterator m_iterator;

		AssetIterator(AssetHandler* pAssetHandler, typename std::unordered_map<UUID, std::unique_ptr<T>>::iterator iterator)
			: m_pAssetHandler(pAssetHandler), m_iterator(iterator)
		{}
	};

	class AssetHandler
	{
	public:
		AssetHandler();
		~AssetHandler();
		AssetHandler& operator=(const AssetHandler&) = delete;
		AssetHandler& operator=(AssetHandler&&) = default;

		bool isAsset(UUID handle) const;

		bool isMesh(UUID handle) const;
		bool isMaterial(UUID handle) const;
		bool isTexture(UUID handle) const;
		bool isHitmesh(UUID handle) const;

	private:
		template<class T, class... Types>
		AssetHandle<T> generateAssetUsingID(UUID handle, Types&&... args) {
			static_assert(std::is_base_of_v<Asset, T>, "Type has to be an Asset | AssetHandler::generateAssetUsingID");
			getMap<T>()[handle] = std::unique_ptr<T>(new T(std::forward<Types>(args)...));
			if constexpr (std::is_same_v<T, Texture>) { Base::getBase()->registerTextureIndex(handle); }// TODO find new solution for gpu texture indexing which allows dynamic registering (adding/removeing textures)
			return AssetHandle<T>(handle, this);
		}
	public:
		template<class T, class... Types>
		AssetHandle<T> generateAsset(Types&&... args) {
			static_assert(std::is_base_of_v<Asset, T>, "Type has to be an Asset | AssetHandler::generateAsset");
			return generateAssetUsingID<T, Types...>(UUID(), std::forward<Types>(args)...);
		}

		template<class T>
		AssetIterator<T> begin() {
			static_assert(std::is_base_of_v<Asset, T>, "Type has to be an Asset | AssetHandler::begin");
			return AssetIterator<T>(this, getMap<T>().begin());
		}

		template<class T>
		AssetIterator<T> end() {
			static_assert(std::is_base_of_v<Asset, T>, "Type has to be an Asset | AssetHandler::end");
			return AssetIterator<T>(this, getMap<T>().end());
		}

		std::filesystem::path getAssetLibrary();

		// events
		AssetHandlerEventHandler& getEventHandler();

	private:
		std::filesystem::path m_alpath;
		std::filesystem::path m_aldir;

		FileLinker m_fileLinker;

		std::unordered_map<UUID, std::unique_ptr<Mesh>> m_meshMap;
		std::unordered_map<UUID, std::unique_ptr<Material>> m_materialMap;
		std::unordered_map<UUID, std::unique_ptr<Texture>> m_textureMap;
		std::unordered_map<UUID, std::unique_ptr<HitMesh>> m_hitmeshMap;

		// Events
		AssetHandlerEventHandler m_eventHandler;

		template<class T>
		T* getAsset(UUID handle) {
			static_assert(std::is_base_of_v<Asset, T>, "Type has to be an Asset | AssetHandler::getAsset");
			if (getMap<T>().count(handle))
				return getMap<T>().at(handle).get();
			return nullptr;
		}

		FileLinker& getFileLinker();

		template<class T>
		std::unordered_map<UUID, std::unique_ptr<T>>& getMap();

		template<class T>
		friend class AssetHandle;
		friend class Loader;
		friend class Base;
		friend class RenderTask;
	};
}

