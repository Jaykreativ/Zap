#include "Zap/Serializer.h"

#include "Zap/ModelLoader.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Transform.h"

#include <fstream>
#include <string>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

char getIgnore(std::ifstream& stream) {
	char c;
	do {
		c = stream.get();
	} while (c == '\n');
	return c;
}

char peekIgnore(std::ifstream& stream) {
	char c;
	do {
		c = stream.peek();
		if (c == '\n')
			stream.get();
	} while (c == '\n');
	return c;
}

void goToNextSymbol(std::ifstream& stream, char symbol) {
	while (getIgnore(stream) != symbol && stream.good()) {}
}

std::string getNextType(std::ifstream& stream) {
	goToNextSymbol(stream, '[');
	std::string strType = "";
	while (true) {
		auto c = getIgnore(stream);
		if (c == ']')
			break;
		strType += c;
	}
	return strType;
}

std::string getData(std::ifstream& stream) {
	std::string strData = "";
	while (true) {
		auto c = getIgnore(stream);
		if (c == '}')
			break;
		strData += c;
	}
	return strData;
}

void serializeCamera(std::ofstream& stream, Zap::Camera& camera) {
	stream << "[lookAtCenter]{";
	stream << camera.lookAtCenter;
	stream << "}\n";

	stream << "[offset]{\n";
	stream <<
		camera.offset[0].x << "," << camera.offset[1].x << "," << camera.offset[2].x << "," << camera.offset[3].x << "\n" <<
		camera.offset[0].y << "," << camera.offset[1].y << "," << camera.offset[2].y << "," << camera.offset[3].y << "\n" <<
		camera.offset[0].z << "," << camera.offset[1].z << "," << camera.offset[2].z << "," << camera.offset[3].z << "\n" <<
		camera.offset[0].w << "," << camera.offset[1].w << "," << camera.offset[2].w << "," << camera.offset[3].w;
	stream << "\n}\n";
}

Zap::Camera deserializeCamera(std::ifstream& stream) {
	Zap::Camera camera;
	goToNextSymbol(stream, '{');
	stream >> camera.lookAtCenter;
	goToNextSymbol(stream, '{');
	stream >> camera.offset[0].x; goToNextSymbol(stream, ',');
	stream >> camera.offset[1].x; goToNextSymbol(stream, ',');
	stream >> camera.offset[2].x; goToNextSymbol(stream, ',');
	stream >> camera.offset[3].x;
	stream >> camera.offset[0].y; goToNextSymbol(stream, ',');
	stream >> camera.offset[1].y; goToNextSymbol(stream, ',');
	stream >> camera.offset[2].y; goToNextSymbol(stream, ',');
	stream >> camera.offset[3].y;
	stream >> camera.offset[0].z; goToNextSymbol(stream, ',');
	stream >> camera.offset[1].z; goToNextSymbol(stream, ',');
	stream >> camera.offset[2].z; goToNextSymbol(stream, ',');
	stream >> camera.offset[3].z;
	stream >> camera.offset[0].w; goToNextSymbol(stream, ',');
	stream >> camera.offset[1].w; goToNextSymbol(stream, ',');
	stream >> camera.offset[2].w; goToNextSymbol(stream, ',');
	stream >> camera.offset[3].w;
	goToNextSymbol(stream, '}');
	return camera;
}

void serializeLight(std::ofstream& stream, Zap::Light& light) {
	stream << "[color]{\n";
	stream << light.color.r << "," << light.color.g << "," << light.color.b;
	stream << "\n}\n";

	stream << "[radius]{\n";
	stream << light.radius;
	stream << "\n}\n";

	stream << "[strength]{\n";
	stream << light.strength;
	stream << "\n}\n";
}

Zap::Light deserializeLight(std::ifstream& stream) {
	Zap::Light light;
	goToNextSymbol(stream, '{');
	stream >> light.color.r; goToNextSymbol(stream, ',');
	stream >> light.color.g; goToNextSymbol(stream, ',');
	stream >> light.color.b;
	goToNextSymbol(stream, '{');
	stream >> light.radius;
	goToNextSymbol(stream, '{');
	stream >> light.strength;
	goToNextSymbol(stream, '}');
	return light;
}

void serializeModel(std::ofstream& stream, Zap::Model& model) {
	//for (auto& mat : model.materials) { // TODO add asset handles for textures for serialization
	//	stream << "[Material]{\n";
	//	mat.albedoColor;
	//	mat.albedoMap;
	//	mat.emissive;
	//	mat.emissiveMap;
	//	mat.metallic;
	//	mat.metallicMap;
	//	mat.roughness;
	//	mat.roughnessMap;
	//	stream << "\n}\n";
	//}
	stream << "[ModelPath]{\n";
	stream << model.filepath;
	stream << "\n}\n";
}

Zap::Model deserializeModel(std::ifstream& stream) {
	Zap::Model model{};
	goToNextSymbol(stream, '{');
	stream >> model.filepath;
	goToNextSymbol(stream, '}');
	Zap::ModelLoader loader;
	model = loader.load(model.filepath.c_str()); // TODO fix loading same model multiple times with asset handles
	return model;
}

void serializeRigidDynamic(std::ofstream& stream, Zap::RigidDynamic& rigidDynamic) {

}

Zap::RigidDynamic deserializeRigidDynamic(std::ifstream& stream) {
	return Zap::RigidDynamic{};
}

void serializeRigidStatic(std::ofstream& stream, Zap::RigidStatic& rigidStatic) {

}

Zap::RigidStatic deserializeRigidStatic(std::ifstream& stream) {
	return Zap::RigidStatic{};
}

void serializeTransform(std::ostream& stream, Zap::Transform& transform) {
	stream <<
		transform.transform[0].x << "," << transform.transform[1].x << "," << transform.transform[2].x << "," << transform.transform[3].x << "\n" <<
		transform.transform[0].y << "," << transform.transform[1].y << "," << transform.transform[2].y << "," << transform.transform[3].y << "\n" <<
		transform.transform[0].z << "," << transform.transform[1].z << "," << transform.transform[2].z << "," << transform.transform[3].z << "\n" <<
		transform.transform[0].w << "," << transform.transform[1].w << "," << transform.transform[2].w << "," << transform.transform[3].w << "\n";
}

Zap::Transform deserializeTransform(std::ifstream& stream) {
	Zap::Transform transform;
	stream >> transform.transform[0].x; goToNextSymbol(stream, ',');
	stream >> transform.transform[1].x; goToNextSymbol(stream, ',');
	stream >> transform.transform[2].x; goToNextSymbol(stream, ',');
	stream >> transform.transform[3].x;
	stream >> transform.transform[0].y; goToNextSymbol(stream, ',');
	stream >> transform.transform[1].y; goToNextSymbol(stream, ',');
	stream >> transform.transform[2].y; goToNextSymbol(stream, ',');
	stream >> transform.transform[3].y;
	stream >> transform.transform[0].z; goToNextSymbol(stream, ',');
	stream >> transform.transform[1].z; goToNextSymbol(stream, ',');
	stream >> transform.transform[2].z; goToNextSymbol(stream, ',');
	stream >> transform.transform[3].z;
	stream >> transform.transform[0].w; goToNextSymbol(stream, ',');
	stream >> transform.transform[1].w; goToNextSymbol(stream, ',');
	stream >> transform.transform[2].w; goToNextSymbol(stream, ',');
	stream >> transform.transform[3].w;
	return transform;
}

namespace Zap {
	Serializer::Serializer(){}
	Serializer::~Serializer(){}

	void Serializer::serializeScene(std::ofstream& stream, Zap::Scene* pScene) {// TODO add serialization of physX scene
		stream << "[Scene]{\n";
		stream << pScene->m_handle;
		stream << "\n}\n";
	}

	void Serializer::serialize(const char* path){
		//{
		//	std::string filePath = std::string(path) + "/" + Zap::Base::getBase()->getApplicationName() + ".zprj";
		//	std::ofstream stream(filePath.c_str(), std::ios::out | std::ios::trunc);
		//
		//	stream << "[Texture]{\n";
		//	stream << "[Handle]{\n";
		//	stream << 
		//
		//	stream.close();
		//}

		std::vector<Scene*> scenes = {};
		for (auto actor : m_actorQueue) {
			bool sceneExists = false;
			for (auto pScene : scenes)
				sceneExists |= actor.m_pScene == pScene;
			if (!sceneExists) {
				std::string filePath = std::string(path) + "/scene(" + std::to_string(scenes.size()) + ").zscn";
				std::ofstream stream(filePath.c_str(), std::ios::out | std::ios::trunc);
				serializeScene(stream, actor.m_pScene);
				stream.close();
				scenes.push_back(actor.m_pScene);
			}

			std::string filePath = std::string(path) + "/" + std::to_string(actor.m_handle) + ".zact";
			std::ofstream stream(filePath.c_str(), std::ios::out | std::ios::trunc);
			stream << "[Actor]{\n";
			
			stream << "[SceneHandle]{\n";
			stream << actor.m_pScene->m_handle;
			stream << "\n}\n";
			
			stream << "[Handle]{\n";
			stream << actor.m_handle;// actor handle
			stream << "\n}\n";
			
			if (actor.hasCamera()) {
				stream << "[Camera]{\n";
				serializeCamera(stream, actor.getCameraCmp());
				stream << "}\n";
			}
			if (actor.hasLight()) {
				stream << "[Light]{\n";
				serializeLight(stream, actor.getLightCmp());
				stream << "}\n";
			}
			if (actor.hasModel()) {
				stream << "[Model]{\n";
				serializeModel(stream, actor.getModelCmp());
				stream << "}\n";
			}
			if (actor.hasRigidDynamic()) {
				stream << "[RigidDynamic]{\n";
				serializeRigidDynamic(stream, actor.getRigidDynamicCmp());
				stream << "}\n";
			}
			if (actor.hasRigidStatic()) {
				stream << "[RigidStatic]{\n";
				serializeRigidStatic(stream, actor.getRigidStaticCmp());
				stream << "}\n";
			}
			if (actor.hasTransform()) {
				stream << "[Transform]{\n";
				serializeTransform(stream, actor.getTransformCmp());
				stream << "}\n";
			}
			stream << "}\n";
			stream.close();
		}

		return;
	}

	void Serializer::deserializeSceneFile(std::ifstream& stream, std::vector<Scene>& scenes, std::unordered_map<UUID, uint32_t>& scenesHandleMap) {
		goToNextSymbol(stream, '{');
		auto strData = getData(stream);
		UUID uuid = std::stoull(strData);
		scenesHandleMap[uuid] = scenes.size();
		scenes.push_back(Scene(uuid));
		scenes.back().init();
		goToNextSymbol(stream, '}');
	}

	void Serializer::deserializeActorFile(std::ifstream& stream, std::vector<Actor>& actors, std::vector<Scene>& scenes, std::unordered_map<UUID, uint32_t>& scenesHandleMap) {
		auto strType = getNextType(stream);
		if (strType.compare("Actor") == 0) {
			goToNextSymbol(stream, '{');
			// get scene

			goToNextSymbol(stream, '{');
			auto strData = getData(stream);
			UUID sceneHandle = std::stoull(strData);
			uint32_t sceneIndex = scenesHandleMap.at(sceneHandle);
			Zap::Scene* pScene = &scenes.at(sceneIndex);

			goToNextSymbol(stream, '{');
			strData = getData(stream);
			UUID uuid = std::stoull(strData);
			actors.push_back(Zap::Actor(uuid, pScene));
			Zap::Actor& actor = actors.back();

			while (peekIgnore(stream) != '}') {
				strType = getNextType(stream);
				goToNextSymbol(stream, '{');

				if (strType.compare("Camera") == 0) {
					Zap::Camera camera = deserializeCamera(stream);
					actor.addCamera(camera);
				}
				else if (strType.compare("Light") == 0) {
					Zap::Light light = deserializeLight(stream);
					actor.addLight(light);
				}
				else if (strType.compare("Model") == 0) {
					Zap::Model model = deserializeModel(stream);
					actor.addModel(model);
				}
				else if (strType.compare("RigidDynamic") == 0) {
					Zap::RigidDynamic rigidDynamic = deserializeRigidDynamic(stream);
					actor.addRigidDynamic(rigidDynamic);
				}
				else if (strType.compare("RigidStatic") == 0) {
					Zap::RigidStatic rigidStatic = deserializeRigidStatic(stream);
					actor.addRigidStatic(rigidStatic);
				}
				else if (strType.compare("Transform") == 0) {
					Zap::Transform transform = deserializeTransform(stream);
					actor.addTransform(transform);
				}

				goToNextSymbol(stream, '}');
			}
		}

	}

	void Serializer::deserialize(const char* path, std::vector<Actor>* actors, std::vector<Scene>* scenes){
		std::unordered_map<UUID, uint32_t> scenesHandleMap;
		for (const auto& entry : std::experimental::filesystem::directory_iterator(path)) {
			auto filePath = entry.path().u8string();
			if (filePath.compare(filePath.size() - 5, 5, ".zscn") == 0) {
				std::ifstream stream(filePath);
				deserializeSceneFile(stream, *scenes, scenesHandleMap);
				stream.close();
			}
		}

		for (const auto& entry : std::experimental::filesystem::directory_iterator(path)) {
			auto filePath = entry.path().u8string();
			if (filePath.compare(filePath.size() - 5, 5, ".zact") == 0) {
				std::ifstream stream(entry.path());
				deserializeActorFile(stream, *actors, *scenes, scenesHandleMap);
				stream.close();
			}
		}
	}

	void Serializer::addActor(Actor actor){
		m_actorQueue.push_back(actor);
	}

	std::ofstream Serializer::beginCustomSerialization(const char* path){
		std::ofstream stream(path, std::ios::out | std::ios::trunc);
		return stream;
	}

	void Serializer::endCustomSerialization(){}

	std::ifstream Serializer::beginCustomDeserialization(const char* path){
		std::ifstream stream(path);
		return stream;
	}

	void Serializer::endCustomDeserialization(){}
}
