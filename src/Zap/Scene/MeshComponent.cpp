#include "Zap/Scene/MeshComponent.h"

namespace Zap {
    std::vector<MeshComponent> MeshComponent::all;

    MeshComponent::MeshComponent(Actor* pActor, uint32_t mesh) : Component(pActor), m_mesh(mesh)
    {
        m_id = all.size();
        all.push_back(*this);
    };
}