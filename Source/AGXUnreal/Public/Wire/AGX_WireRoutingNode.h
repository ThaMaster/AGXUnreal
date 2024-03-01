// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireRoutingNode.generated.h"

/**
* Route nodes are used to specify the initial route of the wire. Each node has a location but
 * no orientation. Some members are only used for some node types, such as RigidBody which is only
 * used by Eye and BodyFixed nodes.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FWireRoutingNode
{
	GENERATED_BODY();

	/**
	 * The type of wire node, e.g., Free, Eye, BodyFixed, etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	EWireNodeType NodeType;

	/**
	 * The location of this node relative to the Wire Component [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	FVector Location;

	/**
	 * The Rigid Body that an Eye or BodyFixed node should be attached to.
	 * Ignored for other node types.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire")
	FAGX_RigidBodyReference RigidBody;

	FWireRoutingNode()
		: NodeType(EWireNodeType::Free)
		, Location(FVector::ZeroVector)
	{
	}

	FWireRoutingNode(const FVector& InLocation)
		: NodeType(EWireNodeType::Free)
		, Location(InLocation)
	{
	}

	void SetBody(UAGX_RigidBodyComponent* Body);
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_WireRouteNode_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static void SetBody(UPARAM(ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body);
};
