#pragma once

#include "Zap/UUID.h"
#include "Zap/Scene/Shape.h"
#include "Zap/Scene/Material.h"
#include "glm.hpp"

namespace Zap {
	class Scene;
	class Transform;
	class Model;
	class Material;
	class RigidDynamic;
	class RigidStatic;
	class Light;
	class Camera;

	class Actor
	{
	public:
		Actor();
		Actor(UUID uuid, Scene* pScene);
		~Actor();

		operator UUID() { return m_handle; }

		bool operator ==(const Actor& act) { return m_handle == act.m_handle; }

		void destroy();

		UUID getHandle();

		bool isValid();

		/* Transform */

		//internally used method
		Transform& getTransformCmp();

		void addTransform(Transform transform);

		void addTransform(glm::mat4 transform = glm::mat4(1));

		void destroyTransform();

		bool hasTransform() const;

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
		void cmpTransform_setScale(float s);

		void cmpTransform_setTransform(glm::mat4& transform);

		glm::vec3 cmpTransform_getPos() const;

		glm::mat4 cmpTransform_getTransform() const;

		/* Model */

		//internally used method
		Model& getModelCmp();

		bool addModel(Model meshes);

		void destroyModel();

		bool hasModel();

		void cmpModel_setMaterial(Material material);

		void cmpModel_setMaterial(uint32_t meshIndex, Material material);

		void cmpModel_addMesh(Mesh mesh, Material material = Material());

		void cmpModel_removeMesh(uint32_t meshIndex);

		Model cmpModel_getModel();

		/* RigidDynamic */

		//internally used method
		RigidDynamic& getRigidDynamicCmp();

		//internally used method
		void addRigidDynamic(const RigidDynamic& rigidDynamic);

		void addRigidDynamic();

		void addRigidDynamic(Shape shape);

		void addRigidDynamic(const std::vector<Shape>& shapes);

		void destroyRigidDynamic();

		bool hasRigidDynamic();

		void cmpRigidDynamic_addForce(const glm::vec3& force);

		void cmpRigidDynamic_clearForce();

		void cmpRigidDynamic_addTorque(const glm::vec3& torque);

		void cmpRigidDynamic_clearTorque();

		void cmpRigidDynamic_updatePose();

		void cmpRigidDynamic_wakeUp();

		void cmpRigidDynamic_attachShape(Shape shape);

		void cmpRigidDynamic_detachShape(Shape shape);

		void cmpRigidDynamic_detachAllShapes();

		std::vector<Shape> cmpRigidDynamic_getShapes();

		void cmpRigidDynamic_setFlag(physx::PxActorFlag::Enum flag, bool value);

		bool cmpRigidDynamic_getFlag(physx::PxActorFlag::Enum flag);

		/* RigidStatic */

		//internally used method
		RigidStatic& getRigidStaticCmp();

		//internally used method
		void addRigidStatic(const RigidStatic& rigidStatic);

		void addRigidStatic();

		void addRigidStatic(Shape shape);

		void addRigidStatic(const std::vector<Shape>& shapes);

		void destroyRigidStatic();

		bool hasRigidStatic();

		void cmpRigidStatic_attachShape(Shape shape);

		void cmpRigidStatic_detachShape(Shape shape);

		void cmpRigidDynamic_setShapes(std::vector<Shape> shapes);

		std::vector<Shape> cmpRigidStatic_getShapes();

		void cmpRigidStatic_setFlag(physx::PxActorFlag::Enum flag, bool value);

		bool cmpRigidStatic_getFlag(physx::PxActorFlag::Enum flag);

		/* Light */

		//internally used method
		Light& getLightCmp();

		void addLight(Light light);

		void addLight(glm::vec3 color = {1, 1, 1}, float strength = 1, float radius = 1);

		void destroyLight();

		bool hasLight();

		void cmpLight_setColor(glm::vec3 color);

		glm::vec3 cmpLight_getColor() const;

		void cmpLight_setStrength(float strength);

		float cmpLight_getStrength() const;

		void cmpLight_setRadius(float radius);

		float cmpLight_getRadius() const;

		/* Camera */

		//internally used method
		Camera& Actor::getCameraCmp();

		void addCamera(Camera camera);

		void addCamera(glm::mat4 offset = glm::mat4(1));

		void destroyCamera();

		bool hasCamera() const;

		void cmpCamera_lookAtCenter();

		void cmpCamera_lookAtFront();

		void cmpCamera_setOffset(glm::mat4 offset);

		glm::mat4 cmpCamera_getOffset() const;

		glm::mat4 cmpCamera_getView() const;

		glm::mat4 cmpCamera_getPerspective(float aspect) const;

	private:
		UUID m_handle = UUID();
		Scene* m_pScene = nullptr;

		friend class Scene;
		friend class PBRenderer;
		friend class Serializer;
	};
}

