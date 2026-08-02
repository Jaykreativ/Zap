#pragma once

#include "Zap/AssetHandling/Asset.h"

#define PX_PHYSX_STATIC_LIB
#include "PxPhysicsAPI.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

namespace Zap {
	class Scene;
	class Serializer
	{
		class Linker {
		public:
			void link(void* ptr, UUID id);
			UUID generateLink(void* ptr);
			bool hasLink(void* ptr);
			bool hasLink(UUID id);
			UUID getLink(void* ptr);
			void* getLink(UUID id);

		private:
			std::unordered_map<void*, UUID> m_links;
			std::unordered_map<UUID, void*> m_inverseLinks;
		};
		static std::unique_ptr<Linker> m_linker;
	public:
		static void beginLinking();
		static void endLinking();

		static void writeLink(void* ptr, std::ostream& stream);
		template<class T>
		static void readLink(T*& ptr, std::istream& stream) {
			UUID id;
			read(id, stream);
			if (m_linker->hasLink(id))
				ptr = reinterpret_cast<T*>(m_linker->getLink(id));
			else
				m_linker->link(ptr, id);
		}
		static void writeLinkReadable(void* ptr, std::ostream& stream);
		template<class T>
		static void readLinkReadable(T*& ptr, std::istream& stream) {
			UUID id;
			stream.ignore(0xffff, ':');
			readReadable(id, stream);
			if (m_linker->hasLink(id))
				ptr = reinterpret_cast<T*>(m_linker->getLink(id));
			else
				m_linker->link(ptr, id);
		}

		template<class T>
		static void write(const T& val, std::ostream& stream);
		template<class T>
		static void read(T& val, std::istream& stream);
		template<class T>
		static void writeReadable(const T& val, std::ostream& stream);
		template<class T>
		static void readReadable(T& val, std::istream& stream);

		// vector
		template<class T>
		static void write(const std::vector<T>& val, std::ostream& stream) {
			write(val.size(), stream);
			for (const T& element : val)
				write(element, stream);
		}
		template<class T>
		static void read(std::vector<T>& val, std::istream& stream) {
			size_t size;
			read(size, stream);
			val.resize(size);
			for (T& element : val)
				read(element, stream);
		}
		template<class T>
		static void writeReadable(const std::vector<T>& val, std::ostream& stream) {
			stream << "{";
			size_t i = 0;
			for (const T& element : val) {
				writeReadable(element, stream);
				i++;
				if (i < val.size())
					stream << ",";
			}
			stream << "}";
		}
		template<class T>
		static void readReadable(std::vector<T>& val, std::istream& stream) {
			val.clear();
			stream.ignore(0xffff, '{');
			bool inBrackets = true;
			int c = ',';
			while (inBrackets) {
				switch(c){
				case ',': {
					if (stream.peek() == '}')
						break;
					T element;
					readReadable(element, stream);
					val.push_back(std::move(element));
					break;
				}
				case '}':
				case std::char_traits<char>::eof():
					return;
				}
				c = stream.get();
			}
		}

		// pair
		template<typename T1, class T2>
		static void write(const std::pair<T1, T2>& val, std::ostream& stream) {
			write(val.first, stream);
			write(val.second, stream);
		}
		template<typename T1, class T2>
		static void read(std::pair<T1, T2>& val, std::istream& stream) {
			read(val.first, stream);
			read(val.second, stream);
		}
		template<typename T1, class T2>
		static void writeReadable(const std::pair<T1, T2>& val, std::ostream& stream) {
			stream << "{";
			writeReadable(val.first, stream);
			stream << ",";
			writeReadable(val.second, stream);
			stream << "}";
		}
		template<typename T1, class T2>
		static void readReadable(std::pair<T1, T2>& val, std::istream& stream) {
			stream.ignore(0xffff, '{');
			readReadable(val.first, stream);
			stream.ignore(0xffff, ',');
			readReadable(val.second, stream);
			stream.ignore(0xffff, '}');
		}

		// unordered_map
		template<typename keyType, class T>
		static void write(const std::unordered_map<keyType, T>& val, std::ostream& stream) {
			write(val.size(), stream);
			for (auto& pair : val) {
				write(pair, stream);
			}
		}
		template<typename keyType, class T>
		static void read(std::unordered_map<keyType, T>& val, std::istream& stream) {
			size_t size;
			read(size, stream);
			for (size_t i = 0; i < size; i++) {
				std::pair<keyType, T> pair;
				read(pair, stream);
				val.insert(pair);
			}
		}
		template<typename keyType, class T>
		static void writeReadable(const std::unordered_map<keyType, T>& val, std::ostream& stream) {
			std::vector<std::pair<keyType, T>> vec;
			vec.resize(val.size());
			size_t i = 0;
			for (auto& pair : val) {
				vec[i] = pair;
				i++;
			}
			writeReadable(vec, stream);
		}
		template<typename keyType, class T>
		static void readReadable(std::unordered_map<keyType, T>& val, std::istream& stream) {
			std::vector<std::pair<keyType, T>> vec;
			readReadable(vec, stream);
			for (auto& pair : vec) {
				val.insert(pair);
			}
		}

		// AssetHandle
		template<class T>
		static void write(AssetHandle<T> val, std::ostream& stream) {
			bool gen = val->isGenerated();
			write(gen, stream);
			if(!gen)
				write(val.m_handle, stream);
		}
		template<class T>
		static void read(AssetHandle<T>& val, std::istream& stream) {
			bool gen;
			read(gen, stream);
			if (!gen) {
				UUID handle;
				read(handle, stream);
				val = AssetHandle<T>(handle, Base::getBase()->getAssetHandler());
			}
			else
				val = AssetHandle<T>(); // TODO implement default assets accessible through the assetHandler
		}
		template<class T>
		static void writeReadable(AssetHandle<T> val, std::ostream& stream) {
			stream << "AssetHandle: ";
			bool gen = val->isGenerated();
			if (gen)
				stream << "{generated}";
			else
				writeReadable(val.m_handle, stream);
		}
		template<class T>
		static void readReadable(AssetHandle<T>& val, std::istream& stream) {
			stream.ignore(0xffff, ':');
			stream.get();
			bool gen = stream.peek() == '{';
			if (!gen) {
				UUID handle;
				readReadable(handle, stream);
				val = AssetHandle<T>(handle, Base::getBase()->getAssetHandler());
			}
			else
				val = AssetHandle<T>();
		}

		// physx
		static void writePxScene(physx::PxScene* pxScene, std::ostream& stream);
		static void readPxScene(physx::PxScene* pxScene, std::istream& stream);
		static void writePxSceneReadable(physx::PxScene* pxScene, std::ostream& stream);
		static void readPxSceneReadable(physx::PxScene* pxScene, std::istream& stream);

		// Scene
		static void writeScene(const Scene& scene, std::filesystem::path sceneFilePath, std::ostream& stream);
		static void readScene(Scene& scene, std::filesystem::path sceneFilePath, std::istream& stream);
		static void writeSceneReadable(const Scene& scene, std::filesystem::path sceneFilePath, std::ostream& stream);
		static void readSceneReadable(Scene& scene, std::filesystem::path sceneFilePath, std::istream& stream);
	};
}

