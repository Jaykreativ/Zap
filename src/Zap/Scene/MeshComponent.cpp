#include "Zap/Scene/MeshComponent.h"

namespace Zap {
    std::vector<MeshComponent> MeshComponent::all;

    MeshComponent::MeshComponent(Actor* pActor, Mesh* pMesh) : Component(pActor), m_pMesh(pMesh)
    {
        m_id = all.size();
        all.push_back(*this);
    };
}