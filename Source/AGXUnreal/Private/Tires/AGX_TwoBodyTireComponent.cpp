#include "Tires/AGX_TwoBodyTireComponent.h"

UAGX_RigidBodyComponent* UAGX_TwoBodyTireComponent::GetHubRigidBody() const
{
	return HubRigidBody.GetRigidBody();
}

UAGX_RigidBodyComponent* UAGX_TwoBodyTireComponent::GetTireRigidBody() const
{
	return TireRigidBody.GetRigidBody();
}
