// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireRoutingNode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_RigidBodyComponent.h"

void FWireRoutingNode::SetBody(UAGX_RigidBodyComponent* Body)
{
	if (Body == nullptr)
	{
		RigidBody.OwningActor = nullptr;
		RigidBody.Name = NAME_None;
		return;
	}

	RigidBody.OwningActor = Body->GetOwner();
	RigidBody.Name = Body->GetFName();
}

bool FWireRoutingNode::Serialize(FArchive& Archive)
{
	// Serialize the normal UPROPERTY data.
	//
	// In what cases is this needed and when is it not? If I don't have this here then the Nodes
	// array become empty after load and Unreal Engine prints an error about too few bytes being
	// read. So it makes sense that it is needed. But FAGX_TerrainCompactionProperties::Serialize
	// doesn't call SerializeTaggedProperties. How are this and that struct different? Is it
	// because this Serialize is called because there is a TStructOpsTypeTraits for
	// FWireRoutingNode, causing the default serialization to not happen at all, as opposed to
	// FAGX_TerrainCompactionProperties which have the Serialize function called manually by
	// UAGX_TerrainMaterial after the default serialization has completed?
	if (Archive.IsLoading() || Archive.IsSaving() || Archive.IsModifyingWeakAndStrongReferences())
	{
		UScriptStruct* Struct = FWireRoutingNode::StaticStruct();
		Struct->SerializeTaggedProperties(Archive, reinterpret_cast<uint8*>(this), Struct, nullptr);
	}

	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::WireRouteNodeFrame))
	{
		Frame.LocalLocation = Location_DEPRECATED;
	}

	return true;
}

void UAGX_WireRouteNode_FL::SetBody(
	UPARAM(ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body)
{
	WireNode.SetBody(Body);
}
