// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Frame.h"
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
	 * The location of the wire node.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "Wire", Meta = (SkipUCSModifiedProperties))
	FAGX_Frame Frame;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	// FVector Location;

	/**
	 * The Rigid Body that an Eye or BodyFixed node should be attached to.
	 * Ignored for other node types.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire")
	FAGX_RigidBodyReference RigidBody;

	FWireRoutingNode()
		: NodeType(EWireNodeType::Free)
	{
	}

	FWireRoutingNode(const FVector& InLocation)
		: NodeType(EWireNodeType::Free)
	{
		Frame.LocalLocation = InLocation;
	}

	void SetBody(UAGX_RigidBodyComponent* Body);

	bool Serialize(FArchive& Archive);

private:
	/**
	 * The location of this node relative to the Wire Component [cm].
	 */
	UPROPERTY(Meta = (DeprecatedProperty, DeprecationMessage = "Use Frame instead."))
	FVector Location_DEPRECATED {FVector::ZeroVector};
};

template <>
struct TStructOpsTypeTraits<FWireRoutingNode> : public TStructOpsTypeTraitsBase2<FWireRoutingNode>
{
	enum
	{
		WithSerializer = true
	};
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_WireRouteNode_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static UPARAM(Ref) FWireRoutingNode& SetBody(UPARAM(Ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body)
	{
		WireNode.SetBody(Body);
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static UPARAM(Ref) FWireRoutingNode& SetFrame(UPARAM(Ref) FWireRoutingNode& WireNode, UPARAM(Ref) FAGX_Frame& Frame)
	{
		WireNode.Frame = Frame;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node")
	static UPARAM(Ref) FAGX_Frame& GetFrame(UPARAM(ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node|Frame")
	static UPARAM(Ref) FWireRoutingNode& SetParentName(UPARAM(Ref) FWireRoutingNode& WireNode, FName ParentName)
	{
		WireNode.Frame.Parent.Name = ParentName;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node|Frame")
	static FName GetParentName(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame.Parent.Name;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node|Frame")
	static UPARAM(Ref) FWireRoutingNode& SetParentOwningActor(UPARAM(Ref) FWireRoutingNode& WireNode, AActor* OwningActor)
	{
		WireNode.Frame.Parent.OwningActor = OwningActor;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node|Frame")
	static AActor* GetParentOwningActor(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame.Parent.OwningActor;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node|Frame")
	static UPARAM(Ref) FWireRoutingNode& SetLocalLocation(UPARAM(Ref) FWireRoutingNode& WireNode, FVector LocalLocation)
	{
		WireNode.Frame.LocalLocation = LocalLocation;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node|Frame")
	static FVector GetLocalLocation(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame.LocalLocation;
	}
};
