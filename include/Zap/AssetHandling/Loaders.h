#pragma once

#include <string>
#include <filesystem>

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

	protected:
		virtual bool isFileCompatible(std::string filetype) = 0;

		// assembly methods

	private:
		// subloads
		void assimpLoad(std::filesystem::path path);
		void stbImageLoad(std::filesystem::path path);
	};

	class ModelLoader : public Loader {

	};

	class TextureLoader : public Loader {

	};
}

