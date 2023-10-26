#include "Zap/Scene/Scene.h"

namespace Zap {
    Scene::Scene() {

    }

    Scene::~Scene() {

    }

    void Scene::addActor(Actor& actor) {
        m_physicsActors.push_back(&actor);
    }
}