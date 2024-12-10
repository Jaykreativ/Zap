#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"

#include <fstream>
#include <iostream>

namespace Zap {
	class Serializer
	{
	public:
		Serializer();
		~Serializer();

		bool beginSerialization(const char* filePath);

		void endSerialization();

		bool beginDeserialization(const char* filePath);

		void endDeserialization();

		char getIgnore(std::istream& stream);

		char peekIgnore(std::istream& stream);

		void goToNextSymbol(std::istream& stream, char symbol);

		bool beginElement(std::string name);

		void endElement();

		std::string readAttribute(std::string attribute, bool* success = nullptr);
		int readAttributei(std::string attribute, bool* success = nullptr);
		float readAttributef(std::string attribute, bool* success = nullptr);
		glm::vec3 readAttributeVec3(std::string attribute, bool* success = nullptr);
		glm::vec4 readAttributeVec4(std::string attribute, bool* success = nullptr);
		glm::mat4 readAttributeMat4(std::string attribute, bool* success = nullptr);

		void writeAttribute(std::string attribute, std::string data);
		void writeAttribute(std::string attribute, int         data);
		void writeAttribute(std::string attribute, float       data);
		void writeAttribute(std::string attribute, glm::vec3   data);
		void writeAttribute(std::string attribute, glm::vec4   data);
		void writeAttribute(std::string attribute, glm::mat4   data);
		void writeAttribute(std::string attribute, UUID        data);

		// Can be called while deserializing
		// Looks for a matching element in current scope
		bool existsElement(std::string element);

		// Can be called while deserializing
		bool existsAttribute(std::string attribute);

	private:
		bool m_isActive = false;
		bool m_isInput = false;

		struct Element {
			std::unordered_map<std::string, Element> m_elements = {};
			std::unordered_map<std::string, std::string> m_attributes = {};
		};

		Element m_rootElement = {};
		std::vector<Element*> m_elementTree = {};
		Element* m_focusedElement = &m_rootElement;

		std::ofstream m_ofstream;
	};
}

