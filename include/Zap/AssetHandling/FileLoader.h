#pragma once

#include "Zap/UUID.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/Rendering/Image.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm.hpp"

#include <vector>
#include <string>
#include <filesystem>

class aiNode;
class aiScene;
class aiMesh;
class aiMaterial;
class aiTexture;

namespace Zap {
	class Texture;
	class Material;
	class Mesh;
	class HitMesh;
	class Model;
	class Actor;
	class Scene;

	class Loader
	{
	public:
		enum Flags {
			eNone = 0x0,
			eTintTextures = 0x1,
			eReuseActor = 0x2
		};
		int flags = eNone;

		friend class AssetHandler;
	};

	class FileLoader : public virtual Loader
	{
		friend class AssetHandler;
	};

	class Image2DLoader : public virtual Loader
	{
	public:
		Image2D load(void* data, uint32_t width, uint32_t height);

		friend class AssetHandler;
	};
	
	class TextureLoader : public Image2DLoader, public FileLoader
	{
	public:
		AssetHandle<Texture> load(std::filesystem::path filepath);

	protected:
		AssetHandle<Texture> load(void* data, uint32_t width, uint32_t height, UUID handle = UUID());

		AssetHandle<Texture> load(std::filesystem::path filepath, UUID handle);

		AssetHandle<Texture> load(const aiTexture* texture, UUID handle = UUID());

		AssetHandle<Texture> load(std::filesystem::path modelpath, std::filesystem::path textureID, UUID handle = UUID());

		friend class AssetHandler;
		friend class Base;
	};

	class MaterialLoader : protected TextureLoader
	{
	protected:
		AssetHandle<Material> load(const aiScene* aScene, const aiMaterial* aMaterial, std::filesystem::path modelpath, UUID handle = UUID());

		friend class AssetHandler;
	};

	class MeshLoader : public virtual Loader
	{
	protected:
		AssetHandle<Mesh> load(aiMesh* aMesh, glm::mat4& transform, glm::vec3& modelBoundMin, glm::vec3& modelBoundMax, UUID handle = UUID());

		AssetHandle<Mesh> loadFromFile(std::filesystem::path filepath, uint32_t index, glm::mat4& transform, UUID handle = UUID());

		friend class AssetHandler;
	};

	class HitMeshLoader : public virtual Loader
	{
	public:
		AssetHandle<HitMesh> load(std::filesystem::path filepath, uint32_t index = 0);

	protected:
		AssetHandle<HitMesh> load(std::filesystem::path filepath, uint32_t index, UUID handle);
		AssetHandle<HitMesh> load(aiMesh* aMesh, UUID handle = UUID());

		friend class AssetHandler;
	};

	class ModelLoader : protected MaterialLoader, protected MeshLoader
	{
	public:
		Model load(std::filesystem::path filepath);

	private:
		void processNode(const aiNode* node, const aiScene* aScene, std::filesystem::path path, glm::mat4& transform, Model& model);

		friend class AssetHandler;
	};

	class ActorLoader : public FileLoader {
	public:
		Actor load(std::filesystem::path filepath, Scene* pScene);

		void store(std::filesystem::path filepath, Actor actor);
	};

	class SceneLoader {
		//Scene load();
	};
}

