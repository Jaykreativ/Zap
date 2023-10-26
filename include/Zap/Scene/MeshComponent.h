#pragma once

#include "Zap/Zap.h"
#include "Zap/Scene/Component.h"
#include "Zap/Scene/Mesh.h"

namespace Zap {
    class MeshComponent : public Component {
    public:
        static std::vector<MeshComponent> all;

        MeshComponent(Actor* pActor, Mesh* pMesh);

        static ComponentType type() {
            return COMPONENT_TYPE_MESH;
        }

        void setMesh(Mesh* pMesh) {
            m_pMesh = pMesh;
        }

    private:
        Mesh* m_pMesh;

        friend class Renderer;
        friend class PBRenderer;
    };

}