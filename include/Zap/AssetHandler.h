#pragma once

#include "Zap/UUID.h"
#include "Zap/EventHandler.h"
#include "Zap/Scene/Mesh.h"
#include "Zap/Scene/Material.h"
#include "Zap/Scene/Texture.h"
#include "Zap/Physics/HitMesh.h"

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

	class AssetHandler
	{
	public:
		AssetHandler();
		// associates the asset handler with a zal asset library file
		AssetHandler(std::filesystem::path path);
		~AssetHandler();

		/* Mesh */

		std::unordered_map<UUID, MeshData>::const_iterator beginMeshes() const;
		std::unordered_map<UUID, MeshData>::iterator beginMeshes();

		std::unordered_map<UUID, MeshData>::const_iterator endMeshes() const;
		std::unordered_map<UUID, MeshData>::iterator endMeshes();

		bool existsMeshData(UUID handle) const;

		const MeshData& getMeshData(UUID handle) const;

		MeshData* getMeshDataPtr(UUID handle);

		/* Material */

		std::unordered_map<UUID, MaterialData>::const_iterator beginMaterials() const;
		std::unordered_map<UUID, MaterialData>::iterator beginMaterials();

		std::unordered_map<UUID, MaterialData>::const_iterator endMaterials() const;
		std::unordered_map<UUID, MaterialData>::iterator endMaterials();

		bool existsMaterialData(UUID handle) const;

		const MaterialData& getMaterialData(UUID handle) const;
		
		MaterialData* getMaterialDataPtr(UUID handle);

		/* Texture */

		std::unordered_map<UUID, TextureData>::const_iterator beginTextures() const;
		std::unordered_map<UUID, TextureData>::iterator beginTextures();

		std::unordered_map<UUID, TextureData>::const_iterator endTextures() const;
		std::unordered_map<UUID, TextureData>::iterator endTextures();

		bool existsTextureData(UUID handle) const;

		const TextureData& getTextureData(UUID handle) const;
		
		TextureData* getTextureDataPtr(UUID handle);

		/* HitMesh */

		std::unordered_map<UUID, HitMeshData>::const_iterator beginHitMeshes() const;
		std::unordered_map<UUID, HitMeshData>::iterator beginHitMeshes();

		std::unordered_map<UUID, HitMeshData>::const_iterator endHitMeshes() const;
		std::unordered_map<UUID, HitMeshData>::iterator endHitMeshes();

		bool existsHitMeshData(UUID handle) const;

		const HitMeshData& getHitMeshData(UUID handle) const;
		
		HitMeshData* getHitMeshDataPtr(UUID handle);

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
		EventHandler<TextureLoadEvent>* getTextureLoadEventHandler();

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

		std::unordered_map<UUID, MeshData> m_meshes = {};
		std::vector<Mesh> m_loadedMeshes = {};
		std::unordered_map<UUID, std::pair<std::filesystem::path, uint32_t>> m_meshPaths = {};
		std::unordered_map<std::pair<std::filesystem::path, uint32_t>, UUID, pairhash> m_pathMeshMap = {};

		std::unordered_map<UUID, MaterialData> m_materials = {};
		std::vector<Material> m_loadedMaterials = {};
		std::unordered_map<UUID, std::pair<std::filesystem::path, uint32_t>> m_materialPaths = {};
		std::unordered_map<std::pair<std::filesystem::path, uint32_t>, UUID, pairhash> m_pathMaterialMap = {};

		std::unordered_map<UUID, TextureData> m_textures = {};
		std::vector<Texture> m_loadedTextures = {};
		std::unordered_map<UUID, std::pair<std::filesystem::path, std::filesystem::path>> m_texturePaths = {}; // path and modelpath if texture is embedded

		std::unordered_map<UUID, HitMeshData> m_hitMeshes = {};
		std::vector<HitMesh> m_loadedHitMeshes = {};
		std::unordered_map<UUID, std::pair<std::filesystem::path, uint32_t>> m_hitMeshPaths = {};
		std::unordered_map<std::pair<std::filesystem::path, uint32_t>, UUID, pairhash> m_pathHitMeshMap = {};

		// Events
		EventHandler<TextureLoadEvent> m_textureLoadEventHandler;

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
		friend class RenderTaskTemplate;
	};
}

