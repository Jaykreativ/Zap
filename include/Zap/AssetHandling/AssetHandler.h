#pragma once

#include "Zap/UUID.h"
#include "Zap/Events.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetTypes/Mesh.h"
#include "Zap/AssetHandling/AssetTypes/Material.h"
#include "Zap/AssetHandling/AssetTypes/Texture.h"

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
		TextureLoadEvent(Texture texture)
			: texture(texture)
		{}
		~TextureLoadEvent() = default;

		Texture texture;
	};

	template<class T>
	class AssetIterator {
		friend class AssetHandler;
	public:
		~AssetIterator(){}

		void operator++() {
			m_iterator++;
		}

		AssetHandle<T> operator*() {
			return AssetHandle<T>(m_iterator*, m_pAssetHandler);
		}

		void operator==(AssetIterator<T> other) {
			return m_iterator == other.m_iterator;
		}

	private:
		AssetHandler* m_pAssetHandler = nullptr;
		std::unordered_map<UUID, T>::iterator m_iterator;

		AssetIterator(AssetHandler* pAssetHandler, std::unordered_map<UUID, T>::iterator iterator)
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

		bool isAsset(UUID handle) const;

		bool isMesh(UUID handle) const;
		bool isMaterial(UUID handle) const;
		bool isTexture(UUID handle) const;
		bool isHitmesh(UUID handle) const;

		template<class T>
		AssetIterator<T> begin();

		template<class T>
		AssetIterator<T> end();

		void setAssetLibrary(std::filesystem::path filepath);

		std::filesystem::path getAssetLibrary();

		// loads/reloads all assets from the given .zal file
		// will invalidate all actors using any assets
		void loadFromFile();

		// stores all assets to a .zal file
		void saveToFile();

		// destroys all assets
		// will invalidate all actors using any assets
		void destroyAssets();

		// events
		AssetHandlerEventHandler& getEventHandler();

	private:
		class pairhash {
		public:
			template <typename T, typename U>
			std::size_t operator()(const std::pair<T, U>& x) const
			{
				return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
			}
		};

		std::filesystem::path m_alpath;
		std::filesystem::path m_aldir;

		std::unordered_map<UUID, Mesh> m_meshMap = {};
		std::unordered_map<UUID, Material> m_materialMap = {};
		std::unordered_map<UUID, Texture> m_textureMap = {};
		std::unordered_map<UUID, HitMesh> m_hitmeshMap = {};

		// Events
		AssetHandlerEventHandler m_eventHandler;

		Asset* getAsset(UUID handle);

		// register assets for Asset Library

		void registerTexture(Texture texture, std::filesystem::path filepath);
		void registerTexture(Texture texture, std::filesystem::path modelpath, std::filesystem::path textureID);
		void registerMaterial(Material material, std::filesystem::path modelpath, uint32_t index);
		void registerMesh(Mesh mesh, std::filesystem::path modelpath, uint32_t index);
		void registerHitMesh(HitMesh hitMesh, std::filesystem::path modelpath, uint32_t index);

		std::filesystem::path processPath(std::filesystem::path path);

		void addTexture(Texture texture);

		void addLoadedTexture(Texture texture);

		friend class Base;
		friend class Mesh;
		friend class Material;
		friend class Texture;
		friend class HitMesh;
		friend class TextureLoader;
		friend class MaterialLoader;
		friend class MeshLoader;
		friend class HitMeshLoader;
		friend class ModelLoader;
		friend class RenderTask;
	};
}

