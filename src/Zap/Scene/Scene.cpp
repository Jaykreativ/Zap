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
        std::cout << base->m_pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) << ", " << numActors << "\n";
        for (uint32_t i = 0; i < numActors; i++) {
            auto pxActor = actors[i];
            PhysicsComponent* pComponent = (PhysicsComponent*)pxActor->userData;
            switch (pComponent->m_type) {
            case PHYSICS_TYPE_RIGID_DYNAMIC: {
                physx::PxRigidDynamic* pxRigidDynamic = (physx::PxRigidDynamic*)pxActor;
                auto mat = physx::PxMat44(pxRigidDynamic->getGlobalPose());
                pComponent->m_pActor->setTransform(*((glm::mat4*)&mat));
                break;
            }
            }
        }
    }
}