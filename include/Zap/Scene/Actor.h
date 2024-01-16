#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Shape.h"
#include "glm.hpp"

namespace Zap {
	class Scene;
	class Transform;
	class Model;
	class PhysicsComponent;
	class Light;
	class Camera;

	class Actor
	{
	public:
		Actor();
		~Actor(); 

		/* Transform */
		void addTransform(glm::mat4 transform); //TODO reduce components to be only databuckets using structs

		bool hasTransform();

		void cmpTransform_translate(glm::vec3 pos);
		void cmpTransform_translate(float x, float y, float z);//TODO add faster functions with pointer to component as argument

		void cmpTransform_setPos(glm::vec3 pos);
		void cmpTransform_setPos(float x, float y, float z);

		void cmpTransform_rotateX(float angle);
		void cmpTransform_rotateY(float angle);
		void cmpTransform_rotateZ(float angle);
		void cmpTransform_rotate(float angle, glm::vec3 axis);

		void cmpTransform_setScale(glm::vec3 scale);
		void cmpTransform_setScale(float x, float y, float z);

		void cmpTransform_setTransform(glm::mat4& transform);

		glm::vec3 cmpTransform_getPos();

		glm::mat4 cmpTransform_getTransform();

		/* Model */

		bool addModel(std::vector<uint32_t> meshes);

		bool hasModel();

		/* RigidDynamic */

		void addRigidDynamic(Shape shape);

		bool hasRigidDynamic();

		void cmpRigidDynamic_addForce(const glm::vec3& force);

		void cmpRigidDynamic_clearForce();

		void cmpRigidDynamic_addTorque(const glm::vec3& torque);

		void cmpRigidDynamic_clearTorque();

		void cmpRigidDynamic_updatePose();

		void cmpRigidDynamic_wakeUp();

		void cmpRigidDynamic_setFlag(physx::PxActorFlag::Enum flag, bool value);

		bool cmpRigidDynamic_getFlag(physx::PxActorFlag::Enum flag);

		/* RigidStatic */
		void addRigidStatic(Shape shape);

		bool hasRigidStatic();

		/* Light */
		void addLight(glm::vec3 color);

		bool hasLight();

		void cmpLight_setColor(glm::vec3 color);

		glm::vec3 cmpLight_getColor();

		/* Camera */
		void addCamera(glm::mat4 offset = glm::mat4(1));

		bool hasCamera();

		void cmpCamera_lookAtCenter();

		void cmpCamera_lookAtFront();

		void cmpCamera_setOffset(glm::mat4 offset);

		glm::mat4 cmpCamera_getOffset();

		glm::mat4 cmpCamera_getView();

		glm::mat4 cmpCamera_getPerspective(float aspect);

	private:

#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
	public:
#endif
		Actor(UUID uuid, Scene* pScene);

		Transform* getTransform();
		Model* getModel();// TODO rework mesh system with models
		RigidDynamicComponent* getRigidDynamic();
		RigidStaticComponent* getRigidStatic();
		Light* getLight();
		Camera* getCamera();

		UUID m_handle = UUID();
		Scene* m_pScene = nullptr;
#ifdef ZP_ENTITY_COMPONENT_SYSTEM_ACCESS
	private:
#endif

		friend class Scene;
		friend class PBRenderer;
	};
}

