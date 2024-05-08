#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Actor.h"

namespace Zap {
	class Serializer
	{
	public:
		Serializer();
		~Serializer();

		void serialize(const char* filePath);

		void deserialize(const char* filePath, std::vector<Actor>* actors, std::vector<Scene>* scenes);

		void addActor(Actor actor);

		std::ofstream beginCustomSerialization(const char* filePath);

		void endCustomSerialization();

		std::ifstream beginCustomDeserialization(const char* filePath);

		void endCustomDeserialization();

	private:
		std::vector<Actor> m_actorQueue = {};

		void serializeScene(std::ofstream& stream, Zap::Scene* pScene);

		void deserializeSceneFile(std::ifstream& stream, std::vector<Scene>& scenes, std::unordered_map<UUID, uint32_t>& scenesHandleMap);
		
		void deserializeActorFile(std::ifstream& stream, std::vector<Actor>& actors, std::vector<Scene>& scenes, std::unordered_map<UUID, uint32_t>& scenesHandleMap);
	};
}

