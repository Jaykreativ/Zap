#include "Zap/Zap.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"
#include "Zap/Scene/Model.h"
#include "Zap/Physics/PhysicsComponent.h"
#include "Zap/Scene/Light.h"
#include "Zap/Scene/Camera.h"
#include "Zap/Scene/Transform.h"
#include "Zap/Scene/Material.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

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

	UUID Actor::getHandle() {
		return m_handle;
	}

	Scene* Actor::getScene() {
		return m_pScene;
	}

	bool Actor::isValid() {
		if (!m_pScene) return false;
		return true;
	}

	/* Transform */

	Transform& Actor::getTransformCmp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return m_pScene->m_transformComponents.at(m_handle);
	}

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

	Model& Actor::getModelCmp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return m_pScene->m_modelComponents.at(m_handle);
	}

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

	Model Actor::cmpModel_getModel() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		Model* cmp = &m_pScene->m_modelComponents.at(m_handle);
		return *cmp;
	}

	/* RigidDynamic */

	RigidDynamic& Actor::getRigidDynamicCmp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return m_pScene->m_rigidDynamicComponents.at(m_handle);
	}

	void Actor::addRigidDynamic(const RigidDynamic& rigidDynamic) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_rigidDynamicComponents.count(m_handle), "Actor can't have multiple RigidDynamics");
		RigidDynamic* cmp = &(m_pScene->m_rigidDynamicComponents[m_handle] = rigidDynamic);
	}

	void Actor::addRigidDynamic() {
		addRigidDynamic(RigidDynamic());
		auto& cmp = getRigidDynamicCmp();
		auto base = Base::getBase();
		cmp.pxActor = base->m_pxPhysics->createRigidDynamic(PxUtils::glmMat4ToTransform(m_pScene->m_transformComponents.at(m_handle).transform));
		cmp.pxActor->userData = (void*)(uint64_t)m_handle;
		m_pScene->m_pxScene->addActor(*cmp.pxActor);
	}

	void Actor::addRigidDynamic(Shape shape) {
		addRigidDynamic();
		cmpRigidDynamic_attachShape(shape);
	}

	void Actor::addRigidDynamic(const std::vector<Shape>& shapes) {
		addRigidDynamic();
		for (auto& shape : shapes) {
			cmpRigidDynamic_attachShape(shape);
		}
	}

	void Actor::destroyRigidDynamic() {
		auto& cmp = getRigidDynamicCmp();
		m_pScene->m_pxScene->removeActor(*cmp.pxActor);
		m_pScene->m_rigidDynamicComponents.erase(m_handle);
	}

	bool Actor::hasRigidDynamic() {
		return m_pScene->m_rigidDynamicComponents.count(m_handle);
	}

	void Actor::cmpRigidDynamic_addForce(const glm::vec3& force) {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->addForce(*((physx::PxVec3*)&force), physx::PxForceMode::eIMPULSE, true);
	}

	void Actor::cmpRigidDynamic_clearForce() {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->clearForce();
	}

	void Actor::cmpRigidDynamic_addTorque(const glm::vec3& torque) {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->addTorque(*((physx::PxVec3*)&torque), physx::PxForceMode::eIMPULSE, true);
	}

	void Actor::cmpRigidDynamic_clearTorque() {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->clearTorque();
	}

	void Actor::cmpRigidDynamic_updatePose() {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->setGlobalPose(PxUtils::glmMat4ToTransform(m_pScene->m_transformComponents.at(m_handle).transform));
	}

	void Actor::cmpRigidDynamic_wakeUp() {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->wakeUp();
	}

	void Actor::cmpRigidDynamic_attachShape(Shape shape) {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->attachShape(shape);
	}

	void Actor::cmpRigidDynamic_detachShape(Shape shape) {
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->detachShape(shape);
	}

	void Actor::cmpRigidDynamic_detachAllShapes() {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		auto shapes = cmpRigidDynamic_getShapes();
		for (auto& shape : shapes)
			cmpRigidDynamic_detachShape(shape);
	}

	void Actor::cmpRigidDynamic_setAngularDamping(float damping) {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		pxActor->setAngularDamping(damping);
	}

	void Actor::cmpRigidDynamic_setLinearDamping(float damping) {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		pxActor->setLinearDamping(damping);
	}

	void Actor::cmpRigidDynamic_setAngularVelocity(glm::vec3 velocity) {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		pxActor->setAngularVelocity(PxUtils::glmVec3toVec3(velocity));
	}

	void Actor::cmpRigidDynamic_setLinearVelocity(glm::vec3 velocity) {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		pxActor->setLinearVelocity(PxUtils::glmVec3toVec3(velocity));
	}

	glm::vec3 Actor::cmpRigidDynamic_getAngularVelocity() {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		return PxUtils::vec3ToGlmVec3(pxActor->getAngularVelocity());
	}

	glm::vec3 Actor::cmpRigidDynamic_getLinearVelocity() {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		return PxUtils::vec3ToGlmVec3(pxActor->getLinearVelocity());
	}

	void Actor::cmpRigidDynamic_setShapes(std::vector<Shape> shapes) {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		cmpRigidDynamic_detachAllShapes();
		for (auto& shape : shapes)
			cmpRigidDynamic_attachShape(shape);
	}

	std::vector<Shape> Actor::cmpRigidDynamic_getShapes() {
		auto& cmp = getRigidDynamicCmp();
		auto* pxActor = cmp.pxActor;
		uint32_t nbShapes = pxActor->getNbShapes();
		physx::PxShape** shapeBuffer = new physx::PxShape * [nbShapes];
		pxActor->getShapes(shapeBuffer, nbShapes, 0);
		std::vector<Shape> shapeVector = {};
		shapeVector.resize(nbShapes);
		for (uint32_t i = 0; i < nbShapes; i++) {
			auto* pxShape = shapeBuffer[i];
			Shape shape = Shape(pxShape);
			shapeVector[i] = shape;
		}
		return shapeVector;
	}

	void Actor::cmpRigidDynamic_setFlag(physx::PxActorFlag::Enum flag, bool value) { // TODO Make own enum
		auto& cmp = getRigidDynamicCmp();
		cmp.pxActor->setActorFlag(flag, value);
	}

	bool Actor::cmpRigidDynamic_getFlag(physx::PxActorFlag::Enum flag) {
		auto& cmp = getRigidDynamicCmp();
		return (flag & (uint8_t)cmp.pxActor->getActorFlags()) == flag;
	}
	
	/* RigidStatic */

	RigidStatic& Actor::getRigidStaticCmp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return m_pScene->m_rigidStaticComponents.at(m_handle);
	}

	void Actor::addRigidStatic(const RigidStatic& rigidStatic) {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		ZP_ASSERT(!m_pScene->m_rigidStaticComponents.count(m_handle), "Actor can't have multiple RigidStatics");
		ZP_ASSERT(m_pScene->m_transformComponents.count(m_handle), "Actor must have a transform for rigid bodies");
		RigidStatic* cmp = &(m_pScene->m_rigidStaticComponents[m_handle] = rigidStatic);
	}

	void Actor::addRigidStatic() {
		addRigidStatic(RigidStatic());
		auto* base = Base::getBase();
		auto& cmp = getRigidStaticCmp();
		cmp.pxActor = base->m_pxPhysics->createRigidStatic(PxUtils::glmMat4ToTransform(m_pScene->m_transformComponents.at(m_handle).transform));
		cmp.pxActor->userData = (void*)(uint64_t)m_handle;
		m_pScene->m_pxScene->addActor(*cmp.pxActor);
	}

	void Actor::addRigidStatic(Shape shape) {
		addRigidStatic();
		cmpRigidStatic_attachShape(shape);
	}
	
	void Actor::addRigidStatic(const std::vector<Shape>& shapes) {
		addRigidStatic();
		for (auto& shape : shapes) {
			cmpRigidStatic_attachShape(shape);
		}
	}

	void Actor::destroyRigidStatic() {
		auto& cmp = getRigidStaticCmp();
		m_pScene->m_pxScene->removeActor(*cmp.pxActor);
		m_pScene->m_rigidStaticComponents.erase(m_handle);
	}

	bool Actor::hasRigidStatic() {
		return m_pScene->m_rigidStaticComponents.count(m_handle);
	}

	void Actor::cmpRigidStatic_attachShape(Shape shape) {
		auto& cmp = getRigidStaticCmp();
		cmp.pxActor->attachShape(shape);
	}

	void Actor::cmpRigidStatic_detachShape(Shape shape) {
		auto& cmp = getRigidStaticCmp();
		cmp.pxActor->detachShape(shape);
	}

	void Actor::cmpRigidStatic_updatePose() {
		auto& cmp = getRigidStaticCmp();
		cmp.pxActor->setGlobalPose(PxUtils::glmMat4ToTransform(m_pScene->m_transformComponents.at(m_handle).transform));
	}

	std::vector<Shape> Actor::cmpRigidStatic_getShapes() {
		auto& cmp = getRigidStaticCmp();
		auto* pxActor = cmp.pxActor;
		uint32_t nbShapes = pxActor->getNbShapes();
		physx::PxShape** shapeBuffer = new physx::PxShape*[nbShapes];
		pxActor->getShapes(shapeBuffer, nbShapes, 0);
		std::vector<Shape> shapeVector = {};
		shapeVector.resize(nbShapes);
		for (uint32_t i = 0; i < nbShapes; i++) {
			auto* pxShape = shapeBuffer[i];
			Shape shape = Shape(pxShape);
			shapeVector[i] = shape;
		}
		return shapeVector;
	}

	void Actor::cmpRigidStatic_setFlag(physx::PxActorFlag::Enum flag, bool value) {
		auto& cmp = getRigidStaticCmp();
		cmp.pxActor->setActorFlag(flag, value);
	}

	bool Actor::cmpRigidStatic_getFlag(physx::PxActorFlag::Enum flag) {
		auto& cmp = getRigidStaticCmp();
		return (flag & (uint8_t)cmp.pxActor->getActorFlags()) == flag;
	}

	/* Light */

	Light& Actor::getLightCmp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return m_pScene->m_lightComponents.at(m_handle);
	}

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

	Camera& Actor::getCameraCmp() {
		ZP_ASSERT(m_pScene, "Actor is not part of scene");
		return m_pScene->m_cameraComponents.at(m_handle);

	}

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
}