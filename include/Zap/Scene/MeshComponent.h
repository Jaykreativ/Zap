#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
    class MeshComponent : public Component {
    public:
        MeshComponent(Actor* pActor, uint32_t mesh);

        static ComponentType type() {
            return COMPONENT_TYPE_MESH;
        }

        void setMesh(uint32_t mesh) {
            m_mesh = mesh;
        }

        Material m_material = Material();

    private:
        uint32_t m_mesh;

        static std::vector<MeshComponent> all;

        friend class Base;
        friend class Actor;
        friend class Renderer;
        friend class PBRenderer;
    };

}