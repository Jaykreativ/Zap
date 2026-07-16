#pragma once

#include "Zap/AssetHandling/Asset.h"

#include <iostream>
#include <vector>

namespace Zap {
	class Serializer
	{
	public:
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
			stream << "{ ";
			size_t i = 0;
			for (const T& element : val) {
				writeReadable(element, stream);
				i++;
				if (i < val.size())
					stream << ", ";
			}
			stream << " }";
		}
		template<class T>
		static void readReadable(std::vector<T>& val, std::istream& stream) {
			val.clear();
			stream.ignore(0xffff, '{');
			bool inBrackets = true;
			int c = ',';
			while (inBrackets) {
				switch(c){
				case '}':
					inBrackets = false;
					break;
				case ',':
					T element;
					readReadable(element, stream);
					val.push_back(std::move(element));
					break;
				}
				c = stream.get();
			}
		}

		// AssetHandler
		template<class T>
		static void write(AssetHandle<T> val, std::ostream& stream) {
			write(val.m_handle, stream);
		}
		template<class T>
		static void read(AssetHandle<T>& val, std::istream& stream) {
			UUID handle;
			read(handle, stream);
			val = AssetHandle<T>(handle, Base::getBase()->getAssetHandler());
		}
		template<class T>
		static void writeReadable(AssetHandle<T> val, std::ostream& stream) {
			stream << "AssetHandle: ";
			writeReadable(val.m_handle, stream);
		}
		template<class T>
		static void readReadable(AssetHandle<T>& val, std::istream& stream) {
			stream.ignore(0xffff, ':');
			UUID handle;
			readReadable(handle, stream);
			val = AssetHandle<T>(handle, Base::getBase()->getAssetHandler());
		}
	};
}

