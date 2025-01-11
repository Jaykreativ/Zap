#include "Zap/Serializer.h"

#include "glm/gtx/string_cast.hpp"

#include <fstream>
#include <string>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>


namespace Zap {
	Serializer::Serializer(){}
	Serializer::~Serializer(){}

	bool Serializer::beginSerialization(const char* path){
		m_isInput = false;
		m_isActive = true;

		m_ofstream = std::ofstream(path, std::ios::out | std::ios::trunc);

		return m_ofstream.fail();
	}

	void Serializer::endSerialization(){
		m_isActive = false;
	}

	bool Serializer::beginDeserialization(const char* path){
		m_isInput = true;
		m_isActive = true;

		std::ifstream stream(path);

		if (stream.fail())
			return false;

		stream.seekg(0, stream.end);
		size_t filesize = stream.tellg();// get file size
		stream.seekg(0, stream.beg);

		uint32_t elementDepth = 0;
		std::vector<Element*> elementTree = {};
		Element* focusedElement = &m_rootElement;

		while (stream.tellg()<filesize) {
			while (stream.good()) {
				if (peekIgnore(stream) == '[') {
					getIgnore(stream);
					elementDepth++;

					std::string strType = "";
					while (true) {
						auto c = getIgnore(stream);
						if (c == ']')
							break;
						strType += c;
					}
					goToNextSymbol(stream, '{');

					elementTree.push_back(focusedElement);
					focusedElement = &(focusedElement->m_elements[strType] = Element{});
				}
				else if (peekIgnore(stream) == '}') {
					getIgnore(stream);
					if (elementDepth > 0) {
						elementDepth--;
						focusedElement = elementTree.back();
						elementTree.pop_back();
					}
				}
				else {
					std::string attrName;
					while (true) {
						auto c = getIgnore(stream);
						if (c == ':' || c == EOF)
							break;
						attrName += c;
					}
					std::string data;
					while (true) {
						auto c = getIgnore(stream);
						if (c == ';' || c == EOF)
							break;
						if (c != ' ')
							data += c;
					}
					if (attrName != "" || data != "") {
						focusedElement->m_attributes[attrName] = data;
					}
				}
			}
		}

		return true;
	}

	void Serializer::endDeserialization(){
		m_isActive = false;
	}

	char Serializer::getIgnore(std::istream& stream) {
		char c;
		do {
			c = stream.get();
		} while (c == '\n');
		return c;
	}

	char Serializer::peekIgnore(std::istream& stream) {
		char c;
		do {
			c = stream.peek();
			if (c == '\n')
				stream.get();
		} while (c == '\n');
		return c;
	}

	void Serializer::goToNextSymbol(std::istream& stream, char symbol) {
		while (getIgnore(stream) != symbol && stream.good()) {}
	}

	bool Serializer::beginElement(std::string name) {
		if (m_isInput) {
			if (!existsElement(name))
				return false;
			m_elementTree.push_back(m_focusedElement);
			m_focusedElement = &m_focusedElement->m_elements.at(name);
		}
		else {
			m_ofstream << "[" << name << "] {\n";
		}
		return true;
	}

	void Serializer::endElement() {
		if (m_isInput) {
			m_focusedElement = m_elementTree.back();
			m_elementTree.pop_back();
		}
		else {
			m_ofstream << "}\n";
		}
	}


	std::string Serializer::readAttribute(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return "";
		}

		return m_focusedElement->m_attributes.at(attribute);
	}
	
	int Serializer::readAttributei(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stoi(m_focusedElement->m_attributes.at(attribute));
	}
	
	long Serializer::readAttributel(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stol(m_focusedElement->m_attributes.at(attribute));
	}
	unsigned long Serializer::readAttributeul(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stoul(m_focusedElement->m_attributes.at(attribute));
	}
	
	long long Serializer::readAttributell(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stoll(m_focusedElement->m_attributes.at(attribute));
	}
	unsigned long long Serializer::readAttributeull(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stoull(m_focusedElement->m_attributes.at(attribute));
	}
	
	float Serializer::readAttributef(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stof(m_focusedElement->m_attributes.at(attribute));
	}
	double Serializer::readAttributed(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return 0;
		}

		return std::stod(m_focusedElement->m_attributes.at(attribute));
	}
	
	glm::vec3 Serializer::readAttributeVec3(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return {0, 0, 0};
		}

		std::string str = m_focusedElement->m_attributes.at(attribute);
		auto vecStr = str.substr(str.find_first_of('(') + 1, str.find_last_of(')') - 1);
		glm::vec3 vec3;

		size_t vecOff = 0;
		for (size_t i = 0; i < 3; i++) {
			auto comma = vecStr.find_first_of(',', vecOff);
			auto value = vecStr.substr(vecOff, (vecOff - comma) - 1);
			vecOff = comma + 1;

			vec3[i] = std::stof(value);
		}
		return vec3;
	}
	glm::vec4 Serializer::readAttributeVec4(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return { 0, 0, 0, 0 };
		}

		std::string str = m_focusedElement->m_attributes.at(attribute);
		auto vecStr = str.substr(str.find_first_of('(') + 1, str.find_last_of(')') - 1);
		glm::vec4 vec4;

		size_t vecOff = 0;
		for (size_t i = 0; i < 4; i++) {
			auto comma = vecStr.find_first_of(',', vecOff);
			auto value = vecStr.substr(vecOff, (vecOff - comma) - 1);
			vecOff = comma + 1;

			vec4[i] = std::stof(value);
		}
		return vec4;
	}
	
	glm::mat4 Serializer::readAttributeMat4(std::string attribute, bool* success) {
		if (success && !existsAttribute(attribute)) {
			*success = false;
			return glm::mat4(0);
		}

		std::string str = m_focusedElement->m_attributes.at(attribute);
		auto matStr = str.substr(str.find_first_of('(')+1, str.find_last_of(')')-1);

		glm::mat4 mat4 = glm::mat4(1);

		size_t offset = 0;
		for (size_t i = 0; i < 4; i++) {
			auto open = matStr.find_first_of('(', offset);
			auto close = matStr.find_first_of(')', offset);
			auto vecStr = matStr.substr(open+1, (close - open)-1);
			offset = close+1;

			size_t vecOff = 0;
			for (size_t j = 0; j < 4; j++) {
				auto comma = vecStr.find_first_of(',', vecOff);
				auto value = vecStr.substr(vecOff, (vecOff - comma)-1);
				vecOff = comma + 1;

				mat4[i][j] = std::stof(value);
			}
		}

		return mat4;
	}
	
	UUID Serializer::readAttributeUUID(std::string attribute, bool* success) {
		return std::stoull(readAttribute(attribute, success));
	}


	void Serializer::writeAttribute(std::string attribute, std::string data) {
		m_ofstream << attribute << ": " << data << ";\n";
	}
	
	void Serializer::writeAttribute(std::string attribute, int data) {
		writeAttribute(attribute, std::to_string(data));
	}
	void Serializer::writeAttribute(std::string attribute, unsigned int data) {
		writeAttribute(attribute, std::to_string(data));
	}
	
	void Serializer::writeAttribute(std::string attribute, long data) {
		writeAttribute(attribute, std::to_string(data));
	}
	void Serializer::writeAttribute(std::string attribute, unsigned long data) {
		writeAttribute(attribute, std::to_string(data));
	}
	
	void Serializer::writeAttribute(std::string attribute, long long data) {
		writeAttribute(attribute, std::to_string(data));
	}
	void Serializer::writeAttribute(std::string attribute, unsigned long long data) {
		writeAttribute(attribute, std::to_string(data));
	}
		
	void Serializer::writeAttribute(std::string attribute, float data) {
		writeAttribute(attribute, std::to_string(data));
	}
	void Serializer::writeAttribute(std::string attribute, double data) {
		writeAttribute(attribute, std::to_string(data));
	}
	
	void Serializer::writeAttribute(std::string attribute, glm::vec3 data) {
		writeAttribute(attribute, glm::to_string(data));
	}
	
	void Serializer::writeAttribute(std::string attribute, glm::vec4 data) {
		writeAttribute(attribute, glm::to_string(data));
	}
	
	void Serializer::writeAttribute(std::string attribute, glm::mat4 data) {
		writeAttribute(attribute, glm::to_string(data));
	}
	
	void Serializer::writeAttribute(std::string attribute, UUID data) {
		writeAttribute(attribute, std::to_string(data));
	}


	bool Serializer::existsElement(std::string element) {
		return m_focusedElement->m_elements.count(element);
	}

	bool Serializer::existsAttribute(std::string attribute) {
		return m_focusedElement->m_attributes.count(attribute);
	}
}
