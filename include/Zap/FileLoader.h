#pragma once

#include "Zap/UUID.h"
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

namespace vk {
	class Image;
}
namespace Zap {
	typedef vk::Image Image;
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

	class ImageLoader : public virtual Loader
	{
	public:
		Image load(void* data, uint32_t width, uint32_t height);

		friend class AssetHandler;
	};
	
	class TextureLoader : public ImageLoader, public FileLoader
	{
	public:
		Texture load(std::filesystem::path filepath);
		Texture load(std::string filepath);

	protected:
		Texture load(void* data, uint32_t width, uint32_t height, UUID handle = UUID());

		Texture load(std::filesystem::path filepath, UUID handle);
		Texture load(std::string filepath, UUID handle);

		Texture load(const aiTexture* texture, UUID handle = UUID());

		Texture load(std::filesystem::path modelpath, std::string textureID, UUID handle = UUID());
		Texture load(std::string modelpath, std::string textureID, UUID handle = UUID());

		friend class AssetHandler;
	};

	class MaterialLoader : protected TextureLoader
	{
	protected:
		Material load(const aiScene* aScene, const aiMaterial* aMaterial, std::string path, std::string filename, UUID handle = UUID());

		friend class AssetHandler;
	};

	class MeshLoader : public virtual Loader
	{
	protected:
		Mesh load(aiMesh* aMesh, glm::mat4& transform, glm::vec3& modelBoundMin, glm::vec3& modelBoundMax, UUID handle = UUID());

		Mesh loadFromFile(std::filesystem::path filepath, uint32_t index, glm::mat4& transform, UUID handle = UUID());
		Mesh loadFromFile(std::string filepath, uint32_t index, glm::mat4& transform, UUID handle = UUID());

		friend class AssetHandler;
	};

	class HitMeshLoader : public virtual Loader
	{
	public:
		HitMesh load(std::filesystem::path filepath, uint32_t index = 0);

	protected:
		HitMesh load(std::filesystem::path filepath, uint32_t index, UUID handle);
		HitMesh load(aiMesh* aMesh, UUID handle = UUID());

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
		Actor load(std::string filepath, Scene* pScene);

		void store(std::filesystem::path filepath, Actor actor);
		void store(std::string filepath, Actor actor);
	};

	class SceneLoader {
		//Scene load();
	};
}

