#pragma once

#include "Zap/Zap.h"
#include "Zap/Serializer.h"
#include "Zap/Scene/Model.h"
#include "Zap/AssetHandling/Asset.h"
#include "Zap/AssetHandling/AssetHandler.h"

#include <string>
#include <filesystem>

class aiString;
class aiNode;
class aiScene;
class aiMesh;
class aiMaterial;
class aiTexture;

namespace Zap {
	class Loader {
	public:
		Loader();
		~Loader();

		void load(std::filesystem::path path);

		bool isFileSupported(std::filesystem::path path);

	protected:
		virtual std::vector<std::string> supportedFileExtensions();

		// assembly methods
		virtual void submitModel(Model model){}

		virtual void submitTexture(AssetHandle<Texture> texture){}

	private:
		AssetHandler& m_assetHandler;

		// generic helpers
		AssetHandle<Texture> generateTextureFromData(UUID handle, int width, int height, int channels, void* data);

		// subloads
		class AssimpReconstructionData : public ReconstructionData {
		public:
			Model model;
			// used for unloaded assets only
			std::vector<UUID> meshes;
			std::vector<UUID> materials;
			std::vector<UUID> embeddedTextures;
		};
		AssetHandle<Texture> extractEmbeddedTexture(UUID handle, const aiTexture* aTexture);
		AssetHandle<Texture> extractTexture(const aiString* aTexPath, const aiScene* aScene, std::vector<AssetHandle<Texture>>& embeddedTextures, std::filesystem::path path);
		AssetHandle<Mesh> extractMesh(UUID handle, const aiMesh* aMesh);
		AssetHandle<Material> extractMaterial(UUID handle, const aiMaterial* aMaterial, const aiScene* aScene, std::vector<AssetHandle<Texture>>& embeddedTextures, std::filesystem::path path);
		void processNode(const aiNode* node, const aiScene* aScene, std::vector<AssetHandle<Mesh>>& meshes, std::vector<AssetHandle<Material>>& materials, const glm::mat4& transform, Model& model);
		void assimpLoad(std::filesystem::path path);
		class StbImageReconstructionData : public ReconstructionData {
		public:
			AssetHandle<Texture> texture;
		};
		void stbImageLoad(std::filesystem::path path);

		// serialization access to ReconstructionData
		friend class Serializer;
	};

	class ModelLoader : public Loader {
	public:
		Model result() { return m_model; } // TODO move result to make room for next load

		virtual void submitModel(Model model) override { m_model = model; }
	protected:
		virtual std::vector<std::string> supportedFileExtensions() override;
	private:
		Model m_model;
	};

	class TextureLoader : public Loader {
	public:
		AssetHandle<Texture> result() { return m_texture; }

		virtual void submitTexture(AssetHandle<Texture> texture) override { m_texture = texture; }
	protected:
		virtual std::vector<std::string> supportedFileExtensions() override;
	private:
		AssetHandle<Texture> m_texture;
	};
}

