#pragma once

#include "Zap/UUID.h"
#include "Zap/Events.h"
#include "Zap/AssetHandling/Asset.h"
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
		// associates the asset handler with a zal asset library file
		AssetHandler(std::filesystem::path path);
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
		AssetHandle<T> generateAssetUsingID(UUID handle, Types&&... args);
		template<class... Types>
		AssetHandle<Mesh> generateAssetUsingID(UUID handle, Types&&... args) {
			m_meshMap[handle] = std::make_unique<Mesh>(std::forward<Types>(args)...);
			return AssetHandle<Mesh>(handle, this);
		}
		template<class... Types>
		AssetHandle<Material> generateAssetUsingID(UUID handle, Types&&... args) {
			m_materialMap[handle] = std::make_unique<Material>(std::forward<Types>(args)...);
			return AssetHandle<Material>(handle, this);
		}
		template<class... Types>
		AssetHandle<Texture> generateAssetUsingID(UUID handle, Types&&... args) {
			m_textureMap[handle] = std::make_unique<Texture>(std::forward<Types>(args)...);
			return AssetHandle<Texture>(handle, this);
		}
		template<class... Types>
		AssetHandle<HitMesh> generateAssetUsingID(UUID handle, Types&&... args) {
			m_hitmeshMap[handle] = std::make_unique<HitMesh>(std::forward<Types>(args)...);
			return AssetHandle<HitMesh>(handle, this);
		}
	public:
		template<class T, class... Types>
		AssetHandle<T> generateAsset(Types&&... args) {
			generateAssetUsingID<T, Types>(UUID(), args);
		}

		template<class T>
		AssetIterator<T> begin();

		template<class T>
		AssetIterator<T> end();

		std::filesystem::path getAssetLibrary();

		// events
		AssetHandlerEventHandler& getEventHandler();

	private:
		std::filesystem::path m_alpath;
		std::filesystem::path m_aldir;

		std::unordered_map<UUID, std::unique_ptr<Mesh>> m_meshMap;
		std::unordered_map<UUID, std::unique_ptr<Material>> m_materialMap;
		std::unordered_map<UUID, std::unique_ptr<Texture>> m_textureMap;
		std::unordered_map<UUID, std::unique_ptr<HitMesh>> m_hitmeshMap;

		// Events
		AssetHandlerEventHandler m_eventHandler;

		template<class T>
		AssetHandle<T> createAsset(UUID handle = UUID());

		template<class T>
		T* getAsset(UUID handle);

		// register assets for Asset Library

		void registerTexture(Texture texture, std::filesystem::path filepath);
		void registerTexture(Texture texture, std::filesystem::path modelpath, std::filesystem::path textureID);
		void registerMaterial(Material material, std::filesystem::path modelpath, uint32_t index);
		void registerMesh(Mesh mesh, std::filesystem::path modelpath, uint32_t index);
		void registerHitMesh(HitMesh hitMesh, std::filesystem::path modelpath, uint32_t index);

		std::filesystem::path processPath(std::filesystem::path path);

		void addTexture(Texture texture);

		void addLoadedTexture(Texture texture);

		template<class T>
		friend class AssetHandle;
		friend class Base;
		friend class RenderTask;
	};
}

