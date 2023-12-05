#include "Zap/Scene/Scene.h"
#include "Zap/Scene/PhysicsComponent.h"

namespace Zap {
    Scene::Scene() {

    }

    Scene::~Scene() {

    }

    void Scene::simulate(float elapsedTime) {
        auto base = Base::getBase();
        base->m_pxScene->simulate(elapsedTime);
        base->m_pxScene->fetchResults(true);
        uint32_t numActors = 0;
        auto actors = base->m_pxScene->getActiveActors(numActors);
        for (uint32_t i = 0; i < numActors; i++) {
            auto pxActor = actors[i];
            switch (pxActor->getType()) {
            case physx::PxActorType::eRIGID_DYNAMIC: {
                auto cmp = &RigidDynamicComponent::all[(uint32_t)pxActor->userData];
                auto mat = physx::PxMat44(((physx::PxRigidDynamic*)pxActor)->getGlobalPose());
                cmp->m_pActor->setTransform(*((glm::mat4*)&mat));
            }
            }
        }
    }
}