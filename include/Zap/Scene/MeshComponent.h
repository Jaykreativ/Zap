#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
    class MeshComponent : public Component {
    public:
        MeshComponent(Actor* pActor, Mesh* pMesh);

        static ComponentType type() {
            return COMPONENT_TYPE_MESH;
        }

        void setMesh(Mesh* pMesh) {
            m_pMesh = pMesh;
        }

    private:
        static std::vector<MeshComponent> all;

        Mesh* m_pMesh;

        friend class Base;
        friend class Renderer;
        friend class PBRenderer;
    };

}