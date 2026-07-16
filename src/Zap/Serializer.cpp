#include "Zap/Serializer.h"

#include "Zap/AssetHandling/Loaders.h"

#include <string>

namespace Zap {
		// arithmetic
		template<>
		void Serializer::write(const float& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(float)); }
		template<>
		void Serializer::read(float& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(float)); }
		template<>
		void Serializer::writeReadable(const float& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(float& val, std::istream& stream) { stream >> val; }
		template<>
		void Serializer::write(const double& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(double)); }
		template<>
		void Serializer::read(double& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(double)); }
		template<>
		void Serializer::writeReadable(const double& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(double& val, std::istream& stream) { stream >> val; }
		template<>
		void Serializer::write(const char& val, std::ostream& stream) { stream.write(&val, sizeof(char)); }
		template<>
		void Serializer::read(char& val, std::istream& stream) { stream.read(&val, sizeof(char)); }
		template<>
		void Serializer::writeReadable(const char& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(char& val, std::istream& stream) { stream >> val; }
		template<>
		void Serializer::write(const int32_t& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(int32_t)); }
		template<>
		void Serializer::read(int32_t& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(int32_t)); }
		template<>
		void Serializer::writeReadable(const int32_t& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(int32_t& val, std::istream& stream) { stream >> val; }
		template<>
		void Serializer::write(const int64_t& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(int64_t)); }
		template<>
		void Serializer::read(int64_t& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(int64_t)); }
		template<>
		void Serializer::writeReadable(const int64_t& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(int64_t& val, std::istream& stream) { stream >> val; }
		template<>
		void Serializer::write(const uint32_t& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(uint32_t)); }
		template<>
		void Serializer::read(uint32_t& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(uint32_t)); }
		template<>
		void Serializer::writeReadable(const uint32_t& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(uint32_t& val, std::istream& stream) { stream >> val; }
		template<>
		void Serializer::write(const uint64_t& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(uint64_t)); }
		template<>
		void Serializer::read(uint64_t& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(uint64_t)); }
		template<>
		void Serializer::writeReadable(const uint64_t& val, std::ostream& stream) { stream << val; }
		template<>
		void Serializer::readReadable(uint64_t& val, std::istream& stream) { stream >> val; }

		template<>
		void Serializer::write(const UUID& val, std::ostream& stream) {
			write((uint64_t)val, stream);
		}
		template<>
		void Serializer::read(UUID& val, std::istream& stream) {
			uint64_t id;
			read(id, stream);
			val = id;
		}
		template<>
		void Serializer::writeReadable(const UUID& val, std::ostream& stream) {
			stream << "id: ";
			writeReadable((uint64_t)val, stream);
		}
		template<>
		void Serializer::readReadable(UUID& val, std::istream& stream) {
			stream.ignore(0xffff, ':');
			uint64_t id;
			readReadable(id, stream);
			val = id;
		}

		// string
		template<>
		void Serializer::write(const std::string& val, std::ostream& stream) {
			stream.write(val.c_str(), val.size() + 1);
		}
		template<>
		void Serializer::read(std::string& val, std::istream& stream) {
			while (stream.peek() != '\0') {
				char c;
				read(c, stream);
				val.push_back(c);
			}
		}
		template<>
		void Serializer::writeReadable(const std::string& val, std::ostream& stream) {
			stream << "\"" << val << "\"";
		}
		template<>
		void Serializer::readReadable(std::string& val, std::istream& stream) {
			stream.ignore(1);
			while (stream.peek() != '\"') {
				char c;
				read(c, stream);
				val.push_back(c);
			}
			stream.ignore(1);
		}

		// ReconstructionData
		template<>
		void Serializer::write(const Loader::AssimpReconstructionData& val, std::ostream& stream) {
			write(val.model.materials, stream);
			write(val.model.meshes, stream);
		}
		template<>
		void Serializer::read(Loader::AssimpReconstructionData& val, std::istream& stream) {
			read(val.model.materials, stream);
			read(val.model.meshes, stream);
		}
		template<>
		void Serializer::writeReadable(const Loader::AssimpReconstructionData& val, std::ostream& stream) {
			stream << "Materials: ";
			writeReadable(val.model.materials, stream);
			stream << "\nMeshes: ";
			writeReadable(val.model.meshes, stream);
		}
		template<>
		void Serializer::readReadable(Loader::AssimpReconstructionData& val, std::istream& stream) {
			stream.ignore(0xffff, ':');
			readReadable(val.model.materials, stream);
			stream.ignore(0xffff, ':');
			readReadable(val.model.meshes, stream);
		}

		template<>
		void Serializer::write(const Loader::StbImageReconstructionData& val, std::ostream& stream) {
			write(val.texture, stream);
		}
		template<>
		void Serializer::read(Loader::StbImageReconstructionData& val, std::istream& stream) {
			read(val.texture, stream);
		}
		template<>
		void Serializer::writeReadable(const Loader::StbImageReconstructionData& val, std::ostream& stream) {
			stream << "Texture: ";
			writeReadable(val.texture, stream);
		}
		template<>
		void Serializer::readReadable(Loader::StbImageReconstructionData& val, std::istream& stream) {
			stream.ignore(0xffff, ':');
			readReadable(val.texture, stream);
		}
}
