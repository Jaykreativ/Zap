#include "Zap/Serializer.h"

#include "Zap/AssetHandling/Loaders.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Components/PhysicsComponents.h"
#include "Zap/Physics/Geometry.h"
#include "Zap/Physics/Shape.h"

#include "glm.hpp"

#include <string>

namespace Zap {
	std::unique_ptr<Serializer::Linker> Serializer::m_linker = nullptr;
	void Serializer::Linker::link(void* ptr, UUID id){
		m_links[ptr] = id;
		m_inverseLinks[id] = ptr;
	}
	UUID Serializer::Linker::generateLink(void* ptr){
		UUID id;
		link(ptr, id);
		return id;
	}
	bool Serializer::Linker::hasLink(void* ptr){
		return m_links.count(ptr);
	}
	bool Serializer::Linker::hasLink(UUID id) {
		return m_inverseLinks.count(id);
	}
	UUID Serializer::Linker::getLink(void* ptr){
		return m_links.at(ptr);
	}
	void* Serializer::Linker::getLink(UUID id) {
		return m_inverseLinks.at(id);
	}

	void Serializer::beginLinking() {
		m_linker = std::make_unique<Linker>();
	}
	void Serializer::endLinking() {
		m_linker.reset();
	}

	// bool
	template<>
	void Serializer::write(const bool& val, std::ostream& stream) { stream.write(reinterpret_cast<const char*>(&val), sizeof(bool)); }
	template<>
	void Serializer::read(bool& val, std::istream& stream) { stream.read(reinterpret_cast<char*>(&val), sizeof(bool)); }
	template<>
	void Serializer::writeReadable(const bool& val, std::ostream& stream) {
		if(val)
			stream << "true";
		else
			stream << "false";
	}
	template<>
	void Serializer::readReadable(bool& val, std::istream& stream) {
		char str[5];
		stream.get(str, 5);
		val = !strcmp(str, "true");
		if (!val)
			stream.get(); // str = fals remove the remaining e, fals e
	}

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

	void Serializer::writeLink(void* ptr, std::ostream& stream) {
		if (m_linker->hasLink(ptr))
			write(m_linker->getLink(ptr), stream);
		else
			write(m_linker->generateLink(ptr), stream);
	}
	void Serializer::writeLinkReadable(void* ptr, std::ostream& stream) {
		stream << "Link: ";
		if (m_linker->hasLink(ptr))
			writeReadable(m_linker->getLink(ptr), stream);
		else
			writeReadable(m_linker->generateLink(ptr), stream);
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
		stream.get(); // remove null terminator from input sequence
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

	// glm vec
	template<>
	void Serializer::write(const glm::vec2& val, std::ostream& stream) {
			write(val.x, stream);
			write(val.y, stream);
		}
	template<>
	void Serializer::read(glm::vec2& val, std::istream& stream) {
			read(val.x, stream);
			read(val.y, stream);
		}
	template<>
	void Serializer::writeReadable(const glm::vec2& val, std::ostream& stream) {
			stream << "vec";
			writeReadable(std::vector({ val.x, val.y }), stream);
		}
	template<>
	void Serializer::readReadable(glm::vec2& val, std::istream& stream) {
			std::vector<float> vec;
			readReadable(vec, stream);
			val.x = vec[0];
			val.y = vec[1];
		}
	template<>
	void Serializer::write(const glm::vec3& val, std::ostream& stream) {
			write(val.x, stream);
			write(val.y, stream);
			write(val.z, stream);
		}
	template<>
	void Serializer::read(glm::vec3& val, std::istream& stream) {
			read(val.x, stream);
			read(val.y, stream);
			read(val.z, stream);
		}
	template<>
	void Serializer::writeReadable(const glm::vec3& val, std::ostream& stream) {
			stream << "vec";
			writeReadable(std::vector({ val.x, val.y, val.z }), stream);
		}
	template<>
	void Serializer::readReadable(glm::vec3& val, std::istream& stream) {
			std::vector<float> vec;
			readReadable(vec, stream);
			val.x = vec[0];
			val.y = vec[1];
			val.z = vec[2];
		}
	template<>
	void Serializer::write(const glm::vec4& val, std::ostream& stream) {
			write(val.x, stream);
			write(val.y, stream);
			write(val.z, stream);
			write(val.w, stream);
		}
	template<>
	void Serializer::read(glm::vec4& val, std::istream& stream) {
			read(val.x, stream);
			read(val.y, stream);
			read(val.z, stream);
			read(val.w, stream);
		}
	template<>
	void Serializer::writeReadable(const glm::vec4& val, std::ostream& stream) {
			stream << "vec";
			writeReadable(std::vector({ val.x, val.y, val.z, val.w }), stream);
		}
	template<>
	void Serializer::readReadable(glm::vec4& val, std::istream& stream) {
			std::vector<float> vec;
			readReadable(vec, stream);
			val.x = vec[0];
			val.y = vec[1];
			val.z = vec[2];
			val.w = vec[3];
		}

	// glm mat
	template<>
	void Serializer::write(const glm::mat2& val, std::ostream& stream) {
			write(val[0], stream);
			write(val[1], stream);
		}
	template<>
	void Serializer::read(glm::mat2& val, std::istream& stream) {
			read(val[0], stream);
			read(val[1], stream);
		}
	template<>
	void Serializer::writeReadable(const glm::mat2& val, std::ostream& stream) {
			stream << "mat";
			writeReadable(std::vector({ val[0], val[1] }), stream);
		}
	template<>
	void Serializer::readReadable(glm::mat2& val, std::istream& stream) {
			std::vector<glm::vec2> vec;
			readReadable(vec, stream);
			val[0] = vec[0];
			val[1] = vec[1];
		}
	template<>
	void Serializer::write(const glm::mat3& val, std::ostream& stream) {
			write(val[0], stream);
			write(val[1], stream);
			write(val[2], stream);
		}
	template<>
	void Serializer::read(glm::mat3& val, std::istream& stream) {
			read(val[0], stream);
			read(val[1], stream);
			read(val[2], stream);
		}
	template<>
	void Serializer::writeReadable(const glm::mat3& val, std::ostream& stream) {
			stream << "mat";
			writeReadable(std::vector({ val[0], val[1], val[2] }), stream);
		}
	template<>
	void Serializer::readReadable(glm::mat3& val, std::istream& stream) {
			std::vector<glm::vec3> vec;
			readReadable(vec, stream);
			val[0] = vec[0];
			val[1] = vec[1];
			val[2] = vec[2];
		}
	template<>
	void Serializer::write(const glm::mat4& val, std::ostream& stream) {
			write(val[0], stream);
			write(val[1], stream);
			write(val[2], stream);
			write(val[3], stream);
		}
	template<>
	void Serializer::read(glm::mat4& val, std::istream& stream) {
			read(val[0], stream);
			read(val[1], stream);
			read(val[2], stream);
			read(val[3], stream);
		}
	template<>
	void Serializer::writeReadable(const glm::mat4& val, std::ostream& stream) {
			stream << "mat";
			writeReadable(std::vector({ val[0], val[1], val[2], val[3] }), stream);
		}
	template<>
	void Serializer::readReadable(glm::mat4& val, std::istream& stream) {
			std::vector<glm::vec4> vec;
			readReadable(vec, stream);
			val[0] = vec[0];
			val[1] = vec[1];
			val[2] = vec[2];
			val[3] = vec[3];
		}


	// ReconstructionData
	template<>
	void Serializer::write(const Loader::AssimpReconstructionData& val, std::ostream& stream) {
			write(val.model.materials, stream);
			write(val.model.meshes, stream);
			write(val.model.transforms, stream);
			write(val.meshes, stream);
			write(val.materials, stream);
			write(val.embeddedTextures, stream);
		}
	template<>
	void Serializer::read(Loader::AssimpReconstructionData& val, std::istream& stream) {
			read(val.model.materials, stream);
			read(val.model.meshes, stream);
			read(val.model.transforms, stream);
			read(val.meshes, stream);
			read(val.materials, stream);
			read(val.embeddedTextures, stream);
		}
	template<>
	void Serializer::writeReadable(const Loader::AssimpReconstructionData& val, std::ostream& stream) {
			stream << "Model{";
			stream << "\nMaterials: ";
			writeReadable(val.model.materials, stream);
			stream << "\nMeshes: ";
			writeReadable(val.model.meshes, stream);
			stream << "\nTransforms: ";
			writeReadable(val.model.transforms, stream);
			stream << "\n}";
			stream << "\nMeshes: ";
			writeReadable(val.meshes, stream);
			stream << "\nMaterials: ";
			writeReadable(val.materials, stream);
			stream << "\nEmbedded Textures: ";
			writeReadable(val.embeddedTextures, stream);
		}
	template<>
	void Serializer::readReadable(Loader::AssimpReconstructionData& val, std::istream& stream) {
			stream.ignore(0xffff, '{');
			stream.ignore(0xffff, ':');
			readReadable(val.model.materials, stream);
			stream.ignore(0xffff, ':');
			readReadable(val.model.meshes, stream);
			stream.ignore(0xffff, ':');
			readReadable(val.model.transforms, stream);
			stream.ignore(0xffff, '}');
			stream.ignore(0xffff, ':');
			readReadable(val.meshes, stream);
			stream.ignore(0xffff, ':');
			readReadable(val.materials, stream);
			stream.ignore(0xffff, ':');
			readReadable(val.embeddedTextures, stream);
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

	// Scene Components
	template<>
	void Serializer::write(const Camera& val, std::ostream& stream) {
		write(val.lookAtCenter, stream);
		write(val.offset, stream);
	}
	template<>
	void Serializer::read(Camera& val, std::istream& stream) {
		read(val.lookAtCenter, stream);
		read(val.offset, stream);
	}
	template<>
	void Serializer::writeReadable(const Camera& val, std::ostream& stream) {
		stream << "lookAtCenter: ";
		writeReadable(val.lookAtCenter, stream);
		stream << " | offset: ";
		writeReadable(val.offset, stream);
	}
	template<>
	void Serializer::readReadable(Camera& val, std::istream& stream) {
		stream.ignore(0xffff, ':');
		readReadable(val.lookAtCenter, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.offset, stream);
	}
	
	template<>
	void Serializer::write(const Light& val, std::ostream& stream) {
		write(val.color, stream);
		write(val.radius, stream);
		write(val.strength, stream);
	}
	template<>
	void Serializer::read(Light& val, std::istream& stream) {
		read(val.color, stream);
		read(val.radius, stream);
		read(val.strength, stream);
	}
	template<>
	void Serializer::writeReadable(const Light& val, std::ostream& stream) {
		stream << "color: ";
		writeReadable(val.color, stream);
		stream << " | radius: ";
		writeReadable(val.radius, stream);
		stream << " | strength: ";
		writeReadable(val.strength, stream);
	}
	template<>
	void Serializer::readReadable(Light& val, std::istream& stream) {
		stream.ignore(0xffff, ':');
		readReadable(val.color, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.radius, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.strength, stream);
	}
	
	template<>
	void Serializer::write(const Model& val, std::ostream& stream) {
		write(val.boundMin, stream);
		write(val.boundMax, stream);
		write(val.materials, stream);
		write(val.meshes, stream);
		write(val.transforms, stream);
	}
	template<>
	void Serializer::read(Model& val, std::istream& stream) {
		read(val.boundMin, stream);
		read(val.boundMax, stream);
		read(val.materials, stream);
		read(val.meshes, stream);
		read(val.transforms, stream);
	}
	template<>
	void Serializer::writeReadable(const Model& val, std::ostream& stream) {
		stream << "\nbounds{min: ";
		writeReadable(val.boundMin, stream);
		stream << " | max: ";
		writeReadable(val.boundMax, stream);
		stream << "}\nmaterials: ";
		writeReadable(val.materials, stream);
		stream << "\nmeshes: ";
		writeReadable(val.meshes, stream);
		stream << "\ntransforms: ";
		writeReadable(val.transforms, stream);
		stream << "\n";
	}
	template<>
	void Serializer::readReadable(Model& val, std::istream& stream) {
		stream.ignore(0xffff, ':');
		readReadable(val.boundMin, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.boundMax, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.materials, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.meshes, stream);
		stream.ignore(0xffff, ':');
		readReadable(val.transforms, stream);
	}
	
	// Geometries
	template<>
	void Serializer::write(const SphereGeometry& val, std::ostream& stream) {
		write(val.getRadius(), stream);
	}
	template<>
	void Serializer::read(SphereGeometry& val, std::istream& stream) {
		float radius;
		read(radius, stream);
		val = SphereGeometry(radius);
	}
	template<>
	void Serializer::writeReadable(const SphereGeometry& val, std::ostream& stream) {
		stream << "{radius: ";
		writeReadable(val.getRadius(), stream);
		stream << "}";
	}
	template<>
	void Serializer::readReadable(SphereGeometry& val, std::istream& stream) {
		float radius;
		stream.ignore(0xffff, ':');
		readReadable(radius, stream);
		val = SphereGeometry(radius);
		stream.ignore(0xffff, '}');
	}

	template<>
	void Serializer::write(const CapsuleGeometry& val, std::ostream& stream) {
		write(val.getRadius(), stream);
		write(val.getHalfHeight(), stream);
	}
	template<>
	void Serializer::read(CapsuleGeometry& val, std::istream& stream) {
		float radius, halfHeight;
		read(radius, stream);
		read(halfHeight, stream);
		val = CapsuleGeometry(radius, halfHeight);
	}
	template<>
	void Serializer::writeReadable(const CapsuleGeometry& val, std::ostream& stream) {
		stream << "{radius: ";
		writeReadable(val.getRadius(), stream);
		stream << ", halfHeight: ";
		writeReadable(val.getHalfHeight(), stream);
		stream << "}";
	}
	template<>
	void Serializer::readReadable(CapsuleGeometry& val, std::istream& stream) {
		float radius, halfHeight;
		stream.ignore(0xffff, ':');
		readReadable(radius, stream);
		stream.ignore(0xffff, ':');
		readReadable(halfHeight, stream);
		val = CapsuleGeometry(radius, halfHeight);
		stream.ignore(0xffff, '}');
	}

	template<>
	void Serializer::write(const BoxGeometry& val, std::ostream& stream) {
		write(val.getHalfExtents(), stream);
	}
	template<>
	void Serializer::read(BoxGeometry& val, std::istream& stream) {
		glm::vec3 halfExtents;
		read(halfExtents, stream);
		val = BoxGeometry(halfExtents);
	}
	template<>
	void Serializer::writeReadable(const BoxGeometry& val, std::ostream& stream) {
		stream << "{halfExtents: ";
		writeReadable(val.getHalfExtents(), stream);
		stream << "}";
	}
	template<>
	void Serializer::readReadable(BoxGeometry& val, std::istream& stream) {
		glm::vec3 halfExtents;
		stream.ignore(0xffff, ':');
		readReadable(halfExtents, stream);
		val = BoxGeometry(halfExtents);
		stream.ignore(0xffff, '}');
	}

	template<>
	void Serializer::write(const PlaneGeometry& val, std::ostream& stream) {}
	template<>
	void Serializer::read(PlaneGeometry& val, std::istream& stream) {
		val = PlaneGeometry();
	}
	template<>
	void Serializer::writeReadable(const PlaneGeometry& val, std::ostream& stream) {
		stream << "{}";
	}
	template<>
	void Serializer::readReadable(PlaneGeometry& val, std::istream& stream) {
		stream.ignore(0xffff, '}');
	}

	template<>
	void Serializer::write(const ConvexMeshGeometry& val, std::ostream& stream) {
		write(val.getHitMesh(), stream);
	}
	template<>
	void Serializer::read(ConvexMeshGeometry& val, std::istream& stream) {
		AssetHandle<Mesh> mesh;
		read(mesh, stream);
		ConvexMesh convexMesh(mesh);
		val = ConvexMeshGeometry(convexMesh);
	}
	template<>
	void Serializer::writeReadable(const ConvexMeshGeometry& val, std::ostream& stream) {
		stream << "{hitMesh: ";
		writeReadable(val.getHitMesh(), stream);
		stream << "}";
	}
	template<>
	void Serializer::readReadable(ConvexMeshGeometry& val, std::istream& stream) {
		AssetHandle<Mesh> mesh;
		stream.ignore(0xffff, ':');
		readReadable(mesh, stream);
		ConvexMesh convexMesh(mesh);
		val = ConvexMeshGeometry(convexMesh);
		stream.ignore(0xffff, '}');
	}

	// Physics Material
	template<>
	void Serializer::write(const PhysicsMaterial& val, std::ostream& stream) {
		write(val.getDynamicFriction(), stream);
		write(val.getStaticFriction(), stream);
		write(val.getRestitution(), stream);
	}
	template<>
	void Serializer::read(PhysicsMaterial& val, std::istream& stream) {
		float dynamicFriction, staticFriction, restitution;
		read(dynamicFriction, stream);
		read(staticFriction, stream);
		read(restitution, stream);
		val = PhysicsMaterial(staticFriction, dynamicFriction, restitution);
	}
	template<>
	void Serializer::writeReadable(const PhysicsMaterial& val, std::ostream& stream) {
		stream << "{dynamic friction: ";
		writeReadable(val.getDynamicFriction(), stream);
		stream << ", static friction: ";
		writeReadable(val.getStaticFriction(), stream);
		stream << ", restitution: ";
		writeReadable(val.getRestitution(), stream);
		stream << "}";
	}
	template<>
	void Serializer::readReadable(PhysicsMaterial& val, std::istream& stream) {
		float dynamicFriction, staticFriction, restitution;
		stream.ignore(0xffff, ':');
		readReadable(dynamicFriction, stream);
		stream.ignore(0xffff, ':');
		readReadable(staticFriction, stream);
		stream.ignore(0xffff, ':');
		readReadable(restitution, stream);
		val = PhysicsMaterial(staticFriction, dynamicFriction, restitution);
		stream.ignore(0xffff, '}');
	}

	// Shape
	template<>
	void Serializer::write(const Shape& val, std::ostream& stream) {
		write(val.getLocalPose(), stream);
		write(val.getMaterial(), stream);
		auto upGeometry = val.getGeometry();
		int type = upGeometry->getType();
		write(type, stream);
		switch (type) {
		case eGEOMETRY_TYPE_SPHERE:
			write(*dynamic_cast<SphereGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_CAPSULE:
			write(*dynamic_cast<CapsuleGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_BOX:
			write(*dynamic_cast<BoxGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_PLANE:
			write(*dynamic_cast<PlaneGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_CONVEX_MESH:
			write(*dynamic_cast<ConvexMeshGeometry*>(upGeometry.get()), stream);
			break;
		default:
			break;
		}
	}
	template<class T>
	void makeShape(Shape& val, PhysicsMaterial material, glm::mat4 localPose, std::istream& stream) {
		T geometry;
		Serializer::read(geometry, stream);
		val = Shape(geometry, material, true, localPose);
	}
	template<>
	void Serializer::read(Shape& val, std::istream& stream) {
		glm::mat4 localPose;
		PhysicsMaterial material(0, 0, 0);
		int type;
		read(localPose, stream);
		read(material, stream);
		read(type, stream);
		switch (type) {
		case eGEOMETRY_TYPE_SPHERE:
			makeShape<SphereGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_CAPSULE:
			makeShape<CapsuleGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_BOX:
			makeShape<BoxGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_PLANE:
			makeShape<PlaneGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_CONVEX_MESH:
			makeShape<ConvexMeshGeometry>(val, material, localPose, stream);
			break;
		default:
			break;
		}
	}
	template<>
	void Serializer::writeReadable(const Shape& val, std::ostream& stream) {
		stream << "local pose: ";
		writeReadable(val.getLocalPose(), stream);
		stream << "\nmaterial: ";
		writeReadable(val.getMaterial(), stream);
		auto upGeometry = val.getGeometry();
		int type = upGeometry->getType();
		stream << "\ngeometry type index: ";
		writeReadable(type, stream);
		stream << "\n";
		switch (type) {
		case eGEOMETRY_TYPE_SPHERE:
			stream << "sphere geometry: ";
			writeReadable(*dynamic_cast<SphereGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_CAPSULE:
			stream << "capsule geometry: ";
			writeReadable(*dynamic_cast<CapsuleGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_BOX:
			stream << "box geometry: ";
			writeReadable(*dynamic_cast<BoxGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_PLANE:
			stream << "plane geometry: ";
			writeReadable(*dynamic_cast<PlaneGeometry*>(upGeometry.get()), stream);
			break;
		case eGEOMETRY_TYPE_CONVEX_MESH:
			stream << "convex mesh geometry: ";
			writeReadable(*dynamic_cast<ConvexMeshGeometry*>(upGeometry.get()), stream);
			break;
		default:
			break;
		}
	}

	template<class T>
	void makeShapeReadable(Shape& val, PhysicsMaterial material, glm::mat4 localPose, std::istream& stream) {
		T geometry;
		Serializer::readReadable(geometry, stream);
		val = Shape(geometry, material, true, localPose);
	}
	template<>
	void Serializer::readReadable(Shape& val, std::istream& stream) {
		glm::mat4 localPose;
		PhysicsMaterial material(0, 0, 0);
		int type;
		stream.ignore(0xffff, ':');
		readReadable(localPose, stream);
		stream.ignore(0xffff, ':');
		readReadable(material, stream);
		stream.ignore(0xffff, ':');
		readReadable(type, stream);
		stream.ignore(0xffff, ':');
		switch (type) {
		case eGEOMETRY_TYPE_SPHERE:
			makeShapeReadable<SphereGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_CAPSULE:
			makeShapeReadable<CapsuleGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_BOX:
			makeShapeReadable<BoxGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_PLANE:
			makeShapeReadable<PlaneGeometry>(val, material, localPose, stream);
			break;
		case eGEOMETRY_TYPE_CONVEX_MESH:
			makeShapeReadable<ConvexMeshGeometry>(val, material, localPose, stream);
			break;
		default:
			break;
		}
	}

	// Physics Components // TODO make a seperate section for pxScene and link the pxActors using generated ids
	template<>
	void Serializer::write(const RigidDynamic& val, std::ostream& stream) {
		writeLink(val.pxActor, stream);
	}
	template<>
	void Serializer::read(RigidDynamic& val, std::istream& stream) {
		readLink(val.pxActor, stream);
	}
	template<>
	void Serializer::writeReadable(const RigidDynamic& val, std::ostream& stream) {
		stream << "PxActor: ";
		writeLinkReadable(val.pxActor, stream);
	}
	template<>
	void Serializer::readReadable(RigidDynamic& val, std::istream& stream) {
		stream.ignore(0xffff, ':');
		readLinkReadable(val.pxActor, stream);
	}
	
	template<>
	void Serializer::write(const RigidStatic& val, std::ostream& stream) {
		writeLink(val.pxActor, stream);
	}
	template<>
	void Serializer::read(RigidStatic& val, std::istream& stream) {
		readLink(val.pxActor, stream);
	}
	template<>
	void Serializer::writeReadable(const RigidStatic& val, std::ostream& stream) {
		stream << "PxActor: ";
		writeLinkReadable(val.pxActor, stream);
	}
	template<>
	void Serializer::readReadable(RigidStatic& val, std::istream& stream) {
		stream.ignore(0xffff, ':');
		readLinkReadable(val.pxActor, stream);
	}
	
	template<>
	void Serializer::write(const Transform& val, std::ostream& stream) {
		write(val.transform, stream);
	}
	template<>
	void Serializer::read(Transform& val, std::istream& stream) {
		read(val.transform, stream);
	}
	template<>
	void Serializer::writeReadable(const Transform& val, std::ostream& stream) {
		stream << "transform: ";
		writeReadable(val.transform, stream);
	}
	template<>
	void Serializer::readReadable(Transform& val, std::istream& stream) {
		stream.ignore(0xffff, ':');
		readReadable(val.transform, stream);
	}
	
	// PxActor
	void writePxActor(physx::PxRigidActor* pxActor, std::ostream& stream) {
		Serializer::writeLink(pxActor, stream);
		Serializer::write((uint64_t)pxActor->userData, stream);
		auto shapes = Shape::getPxRigidActorShapes(pxActor);
		Serializer::write(shapes, stream);
	}
	void readPxRigidActor(physx::PxRigidActor* pxActor, physx::PxScene* pxScene, std::istream& stream) {
		Serializer::readLink(pxActor, stream);
		uint64_t handle;
		Serializer::read(handle, stream);
		std::vector<Shape> shapes;
		Serializer::read(shapes, stream);
		pxActor->userData = (void*)(uint64_t)handle;
		for (auto& shape : shapes) {
			pxActor->attachShape(shape);
		}
		pxScene->addActor(*pxActor);
	}
	void writePxActorReadable(physx::PxRigidActor* pxActor, std::ostream& stream) {
		Serializer::writeLinkReadable(pxActor, stream);
		stream << " | userData: ";
		Serializer::writeReadable((uint64_t)pxActor->userData, stream);
		auto shapes = Shape::getPxRigidActorShapes(pxActor);
		stream << " | shapes: ";
		Serializer::writeReadable(shapes, stream);
	}
	void readPxRigidActorReadable(physx::PxRigidActor* pxActor, physx::PxScene* pxScene, std::istream& stream) {
		Serializer::readLinkReadable(pxActor, stream);
		uint64_t handle;
		stream.ignore(0xffff, ':');
		Serializer::readReadable(handle, stream);
		std::vector<Shape> shapes;
		stream.ignore(0xffff, ':');
		Serializer::readReadable(shapes, stream);
		pxActor->userData = (void*)(uint64_t)handle;
		for (auto& shape : shapes) {
			if (!shape.getPxShape())
				continue;
			pxActor->attachShape(shape);
		}
		pxScene->addActor(*pxActor);
	}

	// PxScene
	std::vector<physx::PxRigidActor*> getPxActors(physx::PxActorTypeFlags types, physx::PxScene* pxScene) {
		size_t numActors = pxScene->getNbActors(types);
		std::vector<physx::PxRigidActor*> pxActors(numActors);
		pxScene->getActors(types, reinterpret_cast<physx::PxActor**>(pxActors.data()), pxActors.size());
		return pxActors;
	}

	void Serializer::writePxScene(physx::PxScene* pxScene, std::ostream& stream) {
		{
			auto pxActors = getPxActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, pxScene);
			write(pxActors.size(), stream);
			for (auto* pxActor : pxActors) {
				writePxActor(pxActor, stream);
			}
		}
		{
			auto pxActors = getPxActors(physx::PxActorTypeFlag::eRIGID_STATIC, pxScene);
			write(pxActors.size(), stream);
			for (auto* pxActor : pxActors) {
				writePxActor(pxActor, stream);
			}
		}
	}
	void Serializer::readPxScene(physx::PxScene* pxScene, std::istream& stream) {
		auto base = Base::getBase();
		{
			size_t numActors;
			read(numActors, stream);
			for (size_t i = 0; i < numActors; i++) {
				physx::PxRigidActor* pxActor = base->m_pxPhysics->createRigidDynamic(PxUtils::glmMat4ToTransform(glm::mat4(1)));
				readPxRigidActor(pxActor, pxScene, stream);
			}
		}
		{
			size_t numActors;
			read(numActors, stream);
			for (size_t i = 0; i < numActors; i++) {
				physx::PxRigidActor* pxActor = base->m_pxPhysics->createRigidStatic(PxUtils::glmMat4ToTransform(glm::mat4(1)));
				readPxRigidActor(pxActor, pxScene, stream);
			}
		}
	}
	void Serializer::writePxSceneReadable(physx::PxScene* pxScene, std::ostream& stream) {
		{
			auto pxActors = getPxActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, pxScene);
			stream << "size: ";
			writeReadable(pxActors.size(), stream);
			stream << " | {";
			size_t i = 0;
			for (auto* pxActor : pxActors) {
				writePxActorReadable(pxActor, stream);
				if(++i < pxActors.size())
					stream << ",";
			}
			stream << "}";
		}
		{
			stream << "\nsize: ";
			auto pxActors = getPxActors(physx::PxActorTypeFlag::eRIGID_STATIC, pxScene);
			writeReadable(pxActors.size(), stream);
			stream << " | {";
			size_t i = 0;
			for (auto* pxActor : pxActors) {
				writePxActorReadable(pxActor, stream);
				if (++i < pxActors.size())
					stream << ",";
			}
			stream << "}";
		}
	}
	void Serializer::readPxSceneReadable(physx::PxScene* pxScene, std::istream& stream) {
		auto base = Base::getBase();
		{
			size_t numActors;
			stream.ignore(0xffff, ':');
			readReadable(numActors, stream);
			stream.ignore(0xffff, '{');
			for (size_t i = 0; i < numActors; i++) {
				physx::PxRigidActor* pxActor = base->m_pxPhysics->createRigidDynamic(PxUtils::glmMat4ToTransform(glm::mat4(1)));
				readPxRigidActorReadable(pxActor, pxScene, stream);
				if (i+1 < numActors)
					stream.ignore(0xffff, ',');
			}
			stream.ignore(0xffff, '}');
		}
		{
			size_t numActors;
			stream.ignore(0xffff, ':');
			readReadable(numActors, stream);
			stream.ignore(0xffff, '{');
			for (size_t i = 0; i < numActors; i++) {
				physx::PxRigidActor* pxActor = base->m_pxPhysics->createRigidStatic(PxUtils::glmMat4ToTransform(glm::mat4(1)));
				readPxRigidActorReadable(pxActor, pxScene, stream);
				if (i + 1 < numActors)
					stream.ignore(0xffff, ',');
			}
			stream.ignore(0xffff, '}');
		}
	}

	// Scene
	void Serializer::writeScene(const Scene& scene, std::filesystem::path sceneFilePath, std::ostream& stream) {
		auto pathSet = scene.getAssetPaths();
		write(pathSet.size(), stream);
		for (auto it = pathSet.begin(); it != pathSet.end(); it++) // write all asset paths for loading before the scene
			write(it->lexically_relative(sceneFilePath.remove_filename()).string(), stream);

		beginLinking();
		writePxScene(scene.m_pxScene, stream);
		write(scene.m_cameraComponents, stream);
		write(scene.m_lightComponents, stream);
		write(scene.m_meshInstanceCount, stream);
		write(scene.m_modelComponents, stream);
		write(scene.m_rigidDynamicComponents, stream);
		write(scene.m_rigidStaticComponents, stream);
		write(scene.m_transformComponents, stream);
		endLinking();
	}
	void Serializer::readScene(Scene& scene, std::filesystem::path sceneFilePath, std::istream& stream) {
		Loader loader;
		size_t numPaths;
		read(numPaths, stream);
		for (size_t i = 0; i < numPaths; i++) { // make sure all required assets are loaded before loading the scene
			std::string path;
			read(path, stream);
			loader.load(sceneFilePath.remove_filename() / path);
		}

		beginLinking();
		readPxScene(scene.m_pxScene, stream);
		read(scene.m_cameraComponents, stream);
		read(scene.m_lightComponents, stream);
		read(scene.m_meshInstanceCount, stream);
		read(scene.m_modelComponents, stream);
		read(scene.m_rigidDynamicComponents, stream);
		read(scene.m_rigidStaticComponents, stream);
		read(scene.m_transformComponents, stream);
		endLinking();
	}
	void Serializer::writeSceneReadable(const Scene& scene, std::filesystem::path sceneFilePath, std::ostream& stream) {
		auto pathSet = scene.getAssetPaths();
		stream << "-- Asset Paths --\nsize: ";
		writeReadable(pathSet.size(), stream);
		stream << "\n";
		for (auto it = pathSet.begin(); it != pathSet.end(); it++) { // write all asset paths for loading before the scene
			writeReadable(it->lexically_relative(sceneFilePath.remove_filename()).string(), stream);
			stream << "\n";
		}

		beginLinking();
		stream << "\n-- PxScene --\n";
		writePxSceneReadable(scene.m_pxScene, stream);
		stream << "\n-- Camera Components --\n";
		writeReadable(scene.m_cameraComponents, stream);
		stream << "\n-- Light Components --\n";
		writeReadable(scene.m_lightComponents, stream);
		stream << "\nmeshInstanceCount: ";
		writeReadable(scene.m_meshInstanceCount, stream);
		stream << "\n-- Model Components --\n";
		writeReadable(scene.m_modelComponents, stream);
		stream << "\n-- RigidDynamic Components --\n";
		writeReadable(scene.m_rigidDynamicComponents, stream);
		stream << "\n-- RigidStatic Components --\n";
		writeReadable(scene.m_rigidStaticComponents, stream);
		stream << "\n-- Transform Components --\n";
		writeReadable(scene.m_transformComponents, stream);
		endLinking();
	}
	void Serializer::readSceneReadable(Scene& scene, std::filesystem::path sceneFilePath, std::istream& stream) {
		Loader loader;
		size_t numPaths;
		stream.ignore(0xffff, ':');
		readReadable(numPaths, stream);
		stream.ignore(0xffff, '\n');
		for (size_t i = 0; i < numPaths; i++) { // make sure all required assets are loaded before loading the scene
			std::string path;
			readReadable(path, stream);
			stream.ignore(0xffff, '\n');
			loader.load(sceneFilePath.remove_filename() / path);
		}

		beginLinking();
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readPxSceneReadable(scene.m_pxScene, stream);
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readReadable(scene.m_cameraComponents, stream);
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readReadable(scene.m_lightComponents, stream);
		stream.ignore(0xffff, ':');
		readReadable(scene.m_meshInstanceCount, stream);
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readReadable(scene.m_modelComponents, stream);
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readReadable(scene.m_rigidDynamicComponents, stream);
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readReadable(scene.m_rigidStaticComponents, stream);
		stream.ignore(0xffff, '-');
		stream.ignore(0xffff, '\n');
		readReadable(scene.m_transformComponents, stream);
		endLinking();
	}
}
