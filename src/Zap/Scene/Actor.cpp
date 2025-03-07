#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"
#include "Zap/Scene/PhysicsComponent.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Material.h"

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

	Actor::Actor(UUID uuid, Scene* pScene)
		: m_handle(uuid), m_pScene(pScene)
	{}

	Actor::~Actor(){}
	
	void Actor::destroy() {
		m_pScene->getRemoveActorEventHandler()->pushEvent(RemoveActorEvent(m_pScene, *this));
		if (hasCamera())
			destroyCamera();
		if (hasLight())
			destroyLight();
		if (hasModel())
			destroyModel();
		if (hasRigidDynamic())
			destroyRigidDynamic();
		if (hasRigidStatic())
			destroyRigidStatic();
		if (hasTransform())
			destroyTransform();
	}

	/* Transform */

	void Actor::addTransform(Transform transform) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_transformComponents.count(m_handle), "Actor can't have multiple transforms");
		m_pScene->m_transformComponents[m_handle] = transform;
	}

	void Actor::addTransform(glm::mat4 transform) {
		addTransform(Transform{ transform });
	}

	void Actor::destroyTransform() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		m_pScene->m_transformComponents.erase(m_handle);
	}

	bool Actor::hasTransform() const {
		return m_pScene->m_transformComponents.count(m_handle);
	}

	void Actor::cmpTransform_translate(glm::vec3 vec) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
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
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
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
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = glm::rotate(cmp->transform, glm::radians<float>(angle), axis);
	}

	void Actor::cmpTransform_setScale(glm::vec3 scale) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = glm::scale(cmp->transform, scale);
	}

	void Actor::cmpTransform_setScale(float x, float y, float z) {
		cmpTransform_setScale({ x, y, z });
	}

	void Actor::cmpTransform_setScale(float s) {
		cmpTransform_setScale({ s, s, s });
	}

	void Actor::cmpTransform_setTransform(glm::mat4& transform) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		cmp->transform = transform;
	}

	glm::vec3 Actor::cmpTransform_getPos() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		return glm::vec3(cmp->transform[3]);
	}

	glm::mat4 Actor::cmpTransform_getTransform() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Transform* cmp = &m_pScene->m_transformComponents.at(m_handle);
		return cmp->transform;
	}
	
	/* Model */

	bool Actor::addModel(Model model) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_modelComponents.count(m_handle), "Actor can't have multiple Models");
		Model* cmp = &(m_pScene->m_modelComponents[m_handle] = model);
		m_pScene->m_meshInstanceCount += cmp->meshes.size();

		m_pScene->getAddModelEventHandler()->pushEvent(AddModelEvent(m_pScene, *this, m_pScene->m_modelComponents.size()));
	}

	void Actor::destroyModel() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Model* cmp = &m_pScene->m_modelComponents[m_handle];
		m_pScene->m_meshInstanceCount -= cmp->meshes.size();
		m_pScene->m_modelComponents.erase(m_handle);

		m_pScene->getRemoveModelEventHandler()->pushEvent(RemoveModelEvent(m_pScene, *this, m_pScene->m_modelComponents.size()));
	}

	bool Actor::hasModel() {
		return m_pScene->m_modelComponents.count(m_handle);
	}

	void Actor::cmpModel_setMaterial(Material material) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Model* cmp = &m_pScene->m_modelComponents.at(m_handle);
		for (Material& mat : cmp->materials) {
			mat = material;
		}
	}

	void Actor::cmpModel_setMaterial(uint32_t meshIndex, Material material) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Model* cmp = &m_pScene->m_modelComponents.at(m_handle);
		cmp->materials[meshIndex] = material;
	}

	void Actor::cmpModel_addMesh(Mesh mesh, Material material) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Model* cmp = &m_pScene->m_modelComponents.at(m_handle);
		cmp->meshes.push_back(mesh);
		cmp->materials.push_back(material);
		m_pScene->m_meshInstanceCount++;
	}

	void Actor::cmpModel_removeMesh(uint32_t meshIndex) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Model* cmp = &m_pScene->m_modelComponents.at(m_handle);
		cmp->meshes.erase(cmp->meshes.begin()+meshIndex);
		cmp->materials.erase(cmp->materials.begin()+meshIndex);
		m_pScene->m_meshInstanceCount--;
	}

	/* RigidDynamic */

	void Actor::addRigidDynamic(RigidDynamic shape) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_rigidDynamicComponents.count(m_handle), "Actor can't have multiple RigidDynamics");
		RigidDynamic* cmp = &(m_pScene->m_rigidDynamicComponents[m_handle] = RigidDynamic{});
		m_pScene->m_pxScene->addActor(*cmp->pxActor);
	}

	void Actor::addRigidDynamic(Shape shape) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_rigidDynamicComponents.count(m_handle), "Actor can't have multiple RigidDynamics");
		auto base = Base::getBase();
		RigidDynamic* cmp = &(m_pScene->m_rigidDynamicComponents[m_handle] = RigidDynamic{});
		cmp->pxActor = base->m_pxPhysics->createRigidDynamic(convertGlmMat(m_pScene->m_transformComponents.at(m_handle).transform));
		cmp->pxActor->userData = (void*)(uint64_t)m_handle;
		cmp->pxActor->attachShape(*shape.m_pxShape);
		m_pScene->m_pxScene->addActor(*cmp->pxActor);
	}

	void Actor::destroyRigidDynamic() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		m_pScene->m_rigidDynamicComponents.erase(m_handle);
	}

	bool Actor::hasRigidDynamic() {
		return m_pScene->m_rigidDynamicComponents.count(m_handle);
	}

	void Actor::cmpRigidDynamic_addForce(const glm::vec3& force) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->addForce(*((physx::PxVec3*)&force), physx::PxForceMode::eIMPULSE, true);
	}

	void Actor::cmpRigidDynamic_clearForce() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->clearForce();
	}

	void Actor::cmpRigidDynamic_addTorque(const glm::vec3& torque) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->addTorque(*((physx::PxVec3*)&torque), physx::PxForceMode::eIMPULSE, true);
	}

	void Actor::cmpRigidDynamic_clearTorque() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->clearTorque();
	}

	void Actor::cmpRigidDynamic_updatePose() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->setGlobalPose(convertGlmMat(m_pScene->m_transformComponents.at(m_handle).transform));
	}

	void Actor::cmpRigidDynamic_wakeUp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->wakeUp();
	}

	void Actor::cmpRigidDynamic_setFlag(physx::PxActorFlag::Enum flag, bool value) { // TODO Make own enum
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		cmp->pxActor->setActorFlag(flag, value);
	}

	bool Actor::cmpRigidDynamic_getFlag(physx::PxActorFlag::Enum flag) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		RigidDynamic* cmp = &m_pScene->m_rigidDynamicComponents.at(m_handle);
		return (flag & (uint8_t)cmp->pxActor->getActorFlags()) == flag;
	}

	/* RigidStatic */

	void Actor::addRigidStatic(RigidStatic rigidStatic) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_rigidStaticComponents.count(m_handle), "Actor can't have multiple RigidStatics");
		RigidStatic* cmp = &(m_pScene->m_rigidStaticComponents[m_handle] = RigidStatic{});
		m_pScene->m_pxScene->addActor(*cmp->pxActor);
	}

	void Actor::addRigidStatic(Shape shape) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_rigidStaticComponents.count(m_handle), "Actor can't have multiple RigidStatics");
		auto base = Base::getBase();
		RigidStatic* cmp = &(m_pScene->m_rigidStaticComponents[m_handle] = RigidStatic{});
		cmp->pxActor = base->m_pxPhysics->createRigidStatic(convertGlmMat(m_pScene->m_transformComponents.at(m_handle).transform));
		cmp->pxActor->userData = (void*)(uint64_t)m_handle;
		cmp->pxActor->attachShape(*shape.m_pxShape);
		m_pScene->m_pxScene->addActor(*cmp->pxActor);
	}
	
	void Actor::destroyRigidStatic() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		m_pScene->m_rigidStaticComponents.erase(m_handle);
	}

	bool Actor::hasRigidStatic() {
		return m_pScene->m_rigidStaticComponents.count(m_handle);
	}

	/* Light */

	void Actor::addLight(Light light) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_lightComponents.count(m_handle), "Actor can't have multiple lights");
		m_pScene->m_lightComponents[m_handle] = light;
		m_pScene->m_addLightEventHandler.pushEvent(AddLightEvent(m_pScene, *this, m_pScene->m_lightComponents.size()));
	}

	void Actor::addLight(glm::vec3 color, float strength, float radius) {
		addLight(Light{ color, strength, radius });
	}

	void Actor::destroyLight() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		m_pScene->m_lightComponents.erase(m_handle);
		m_pScene->getRemoveLightEventHandler()->pushEvent(RemoveLightEvent(m_pScene, *this, m_pScene->m_lightComponents.size()));
	}

	bool Actor::hasLight() {
		return m_pScene->m_lightComponents.count(m_handle);
	}

	void Actor::cmpLight_setColor(glm::vec3 color) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		cmp->color = color;
	}

	glm::vec3 Actor::cmpLight_getColor() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		return cmp->color;
	}

	void Actor::cmpLight_setStrength(float strength) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		cmp->strength = strength;
	}

	float Actor::cmpLight_getStrength() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		return cmp->strength;
	}

	void Actor::cmpLight_setRadius(float radius) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		cmp->radius = radius;
	}

	float Actor::cmpLight_getRadius() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Light* cmp = &m_pScene->m_lightComponents.at(m_handle);
		return cmp->radius;
	}

	/* Camera */

	void Actor::addCamera(Camera camera) {
		ZP_ASSERT(!m_pScene->m_cameraComponents.count(m_handle), "Actor can't have multiple cameras");
		m_pScene->m_cameraComponents[m_handle] = camera;
	}

	void Actor::addCamera(glm::mat4 offset) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_cameraComponents.count(m_handle), "Actor can't have multiple cameras");
		m_pScene->m_cameraComponents[m_handle] = Camera{ false, offset };
	}

	void Actor::destroyCamera() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		m_pScene->m_cameraComponents.erase(m_handle);
	}

	bool Actor::hasCamera() const {
		return m_pScene->m_cameraComponents.count(m_handle);
	}

	void Actor::cmpCamera_lookAtCenter() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		cmp->lookAtCenter = true;
	}

	void Actor::cmpCamera_lookAtFront() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		cmp->lookAtCenter = false;
	}

	void Actor::cmpCamera_setOffset(glm::mat4 offset) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		cmp->offset = offset;
	}

	glm::mat4 Actor::cmpCamera_getOffset() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		return cmp->offset;
	}

	glm::mat4 Actor::cmpCamera_getView() const {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Camera* cmp = &m_pScene->m_cameraComponents.at(m_handle);
		glm::mat4 transform = m_pScene->m_transformComponents.at(m_handle).transform;
		if (cmp->lookAtCenter) {
			return glm::lookAt(glm::vec3(transform[3]) + glm::vec3(cmp->offset[3]), glm::vec3(transform[3]), glm::vec3(cmp->offset[1]));
		}
		else {
			return glm::lookAt(glm::vec3(transform[3]) + glm::vec3(cmp->offset[3]), glm::vec3(transform[3]) + glm::vec3(cmp->offset[3]) + glm::vec3(transform[2]), glm::vec3(cmp->offset * transform[1]));
		}
	}

	glm::mat4 Actor::cmpCamera_getPerspective(float aspect) const {
		return glm::perspective<float>(glm::radians<float>(60), aspect, 0.01, 1000);
	}

	/* AudioListener */

	void Actor::addAudioListener() {
		m_pScene->m_audioListenerComponent.first = true;
		m_pScene->m_audioListenerComponent.second.first = m_handle;
	}

	void Actor::destroyAudioListener() {
		m_pScene->m_audioListenerComponent.first = false;
		m_pScene->m_audioListenerComponent.second.first = 0x0;
	}

	bool Actor::hasAudioListener() {
		return m_pScene->m_audioListenerComponent.first;
	}

	/* getters */

	Transform* Actor::getTransform() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return &m_pScene->m_transformComponents.at(m_handle);
	}

	Model* Actor::getModel() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return &m_pScene->m_modelComponents.at(m_handle);
	}

	RigidDynamic* Actor::getRigidDynamic() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return &m_pScene->m_rigidDynamicComponents.at(m_handle);
	}

	RigidStatic* Actor::getRigidStatic() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return &m_pScene->m_rigidStaticComponents.at(m_handle);
	}

	Light* Actor::getLight() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return &m_pScene->m_lightComponents.at(m_handle);
	}

	Camera* Actor::getCamera() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return &m_pScene->m_cameraComponents.at(m_handle);
	}
}