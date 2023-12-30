#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/MeshComponent.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Transform.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

physx::PxTransform convertGlmMat(glm::mat4 glmt) {
	glmt[0] = glm::normalize(glmt[0]);
	glmt[1] = glm::normalize(glmt[1]);
	glmt[2] = glm::normalize(glmt[2]);

	auto pos = *((physx::PxVec3*)&glm::vec3(glmt[3]));
	auto quat = *((physx::PxQuat*)&glm::quat_cast(glm::mat3(glmt)));

	return physx::PxTransform(pos, quat);
}

namespace Zap {
	Actor::Actor(){}

	Actor::~Actor(){}
	
	/* Transform */
	void Actor::addTransform(glm::mat4 transform) {
		ZP_ASSERT(!m_pScene->m_transformComponents.count(m_handle), "Actor can't have multiple transforms");
		m_pScene->m_transformComponents[m_handle] = Transform{ transform };
	}

	void Actor::cmpTransform_translate(glm::vec3 vec) {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = glm::translate(cmp->transform, vec);
	}

	void Actor::cmpTransform_translate(float x, float y, float z) {
		cmpTransform_translate({ x, y, z });
	}

	void Actor::cmpTransform_setPos(float x, float y, float z) {
		cmpTransform_setPos(glm::vec3(x, y, z));
	}

	void Actor::cmpTransform_setPos(glm::vec3 pos) {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform[3] = glm::vec4(pos, 1);
	}

	void Actor::cmpTransform_rotateX(float angle) {
		cmpTransform_rotate(angle, glm::vec3(1, 0, 0));
	}

	void Actor::cmpTransform_rotateY(float angle) {
		cmpTransform_rotate(angle, glm::vec3(0, 1, 0));
	}

	void Actor::cmpTransform_rotateZ(float angle) {
		cmpTransform_rotate(angle, glm::vec3(0, 0, 1));
	}

	void Actor::cmpTransform_rotate(float angle, glm::vec3 axis) {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = glm::rotate(cmp->transform, glm::radians<float>(angle), axis);
	}

	void Actor::cmpTransform_setScale(glm::vec3 scale) {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = glm::scale(cmp->transform, scale);
	}

	void Actor::cmpTransform_setScale(float x, float y, float z) {
		cmpTransform_setScale({ x, y, z });
	}

	void Actor::cmpTransform_setTransform(glm::mat4& transform) {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = transform;
	}

	glm::vec3 Actor::cmpTransform_getPos() {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		return glm::vec3(cmp->transform[3]);
	}

	glm::mat4 Actor::cmpTransform_getTransform() {
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		return cmp->transform;
	}

	bool Actor::addMesh(uint32_t mesh) {
		ZP_ASSERT(!m_pScene->m_meshComponents.count(m_handle), "Actor can't have multiple meshes");
		m_pScene->m_meshComponents[m_handle] = Mesh{};
	}

	bool Actor::addMeshes(std::vector<uint32_t> meshes) {
		bool returnVal = true;
		for (uint32_t mesh : meshes) {
			returnVal &= addMesh(mesh);
		}
		return returnVal;
	}
	
	bool Actor::addRigidDynamic(Shape shape, Scene scene) {
		ZP_ASSERT(!m_pScene->m_rigidDynamicComponents.count(m_handle), "Actor can't have multiple RigidDynamics");
		auto base = Base::getBase();
		RigidDynamicComponent* cmp = &(m_pScene->m_rigidDynamicComponents[m_handle] = RigidDynamicComponent{});
		cmp->pxActor = base->m_pxPhysics->createRigidDynamic(convertGlmMat(m_pScene->m_transformComponents.at(m_handle).transform));
		cmp->pxActor->userData = (void*)(uint64_t)m_handle;
		cmp->pxActor->attachShape(*shape.m_pxShape);
		m_pScene->m_pxScene->addActor(*cmp->pxActor);
	}

	void Actor::cmpRigidDynamic_addForce(const glm::vec3& force) {
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->addForce(*((physx::PxVec3*)&force), physx::PxForceMode::eIMPULSE, true);
	}

	void Actor::cmpRigidDynamic_clearForce() {
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->clearForce();
	}

	void Actor::cmpRigidDynamic_addTorque(const glm::vec3& torque) {
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->addTorque(*((physx::PxVec3*)&torque), physx::PxForceMode::eIMPULSE, true);
	}

	void Actor::cmpRigidDynamic_clearTorque() {
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->clearTorque();
	}

	void Actor::cmpRigidDynamic_wakeUp() {
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->wakeUp();
	}

	void Actor::cmpRigidDynamic_setFlag(physx::PxActorFlag::Enum flag, bool value) { // TODO Make own enum
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->setActorFlag(flag, value);
	}

	bool Actor::cmpRigidDynamic_getFlag(physx::PxActorFlag::Enum flag) {
		RigidDynamicComponent* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		return (flag & (uint8_t)cmp->pxActor->getActorFlags()) == flag;
	}

	bool Actor::addRigidStatic(Shape shape, Scene scene) {
		ZP_ASSERT(!m_pScene->m_rigidStaticComponents.count(m_handle), "Actor can't have multiple RigidStatics");
		auto base = Base::getBase();
		RigidStaticComponent* cmp = &(m_pScene->m_rigidStaticComponents[m_handle] = RigidStaticComponent{});
		cmp->pxActor = base->m_pxPhysics->createRigidStatic(convertGlmMat(m_pScene->m_transformComponents.at(m_handle).transform));
		cmp->pxActor->userData = (void*)(uint64_t)m_handle;
		cmp->pxActor->attachShape(*shape.m_pxShape);
		m_pScene->m_pxScene->addActor(*cmp->pxActor);
	}

	bool Actor::addLight(glm::vec3 color) {
		ZP_ASSERT(!m_pScene->m_lightComponents.count(m_handle), "Actor can't have multiple lights");
		m_pScene->m_lightComponents[m_handle] = Light{ color };
	}

	void Actor::cmpLight_setColor(glm::vec3 color) {
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		cmp->color = color;
	}

	glm::vec3 Actor::cmpLight_getColor() {
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		return cmp->color;
	}

	bool Actor::addCamera(glm::mat4 offset = glm::mat4(1)) {
		ZP_ASSERT(!m_pScene->m_cameraComponents.count(m_handle), "Actor can't have multiple cameras");
		m_pScene->m_cameraComponents[m_handle] = Camera{ false, offset };
	}

	void Actor::cmpCamera_lookAtCenter() {
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		cmp->lookAtCenter = true;
	}

	void Actor::cmpCamera_lookAtFront() {
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		cmp->lookAtCenter = false;
	}

	void Actor::cmpCamera_setOffset(glm::mat4 offset) {
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		cmp->offset = offset;
	}

	glm::mat4 Actor::cmpCamera_getOffset() {
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		return cmp->offset;
	}

	glm::mat4 Actor::cmpCamera_getView() {
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		glm::mat4 transform = m_pScene->m_transformComponents.at(m_handle).transform;
		if (cmp->lookAtCenter) {
			return glm::lookAt(glm::vec3(transform[3]) + glm::vec3(cmp->offset[3]), glm::vec3(transform[3]), glm::vec3(cmp->offset[1]));
		}
		else {
			return glm::lookAt(glm::vec3(transform[3]) + glm::vec3(cmp->offset[3]), glm::vec3(transform[3]) + glm::vec3(cmp->offset[3]) + glm::vec3(transform[2]), glm::vec3(cmp->offset * transform[1]));
		}
	}

	glm::mat4 Actor::cmpCamera_getPerspective(float aspect) {
		return glm::perspective<float>(glm::radians<float>(60), aspect, 0.01, 1000);
	}

	Transform* Actor::getTransform() {
		return &m_pScene->m_transformComponents.at(m_handle);
	}

	MeshComponent* Actor::getMesh() {
		return &m_pScene->m_meshComponents.at(m_handle);
	}

	RigidDynamicComponent* Actor::getRigidDynamic() {
		return &m_pScene->m_rigidDynamicComponents.at(m_handle);
	}

	RigidStaticComponent* Actor::getRigidStatic() {
		return &m_pScene->m_rigidStaticComponents.at(m_handle);
	}

	Light* Actor::getLight() {
		return &m_pScene->m_lightComponents.at(m_handle);
	}

	Camera* Actor::getCamera() {
		return &m_pScene->m_cameraComponents.at(m_handle);
	}
}