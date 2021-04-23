#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_WireComponent.generated.h"

/** Types of wire nodes we have */
UENUM(BlueprintType)
enum class EWireNodeType : uint8
{
	FreeNode,
	EyeNode,
	BodyFixedNode
};

USTRUCT(BlueprintType)
struct FWireNode
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	EWireNodeType NodeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	FVector Location;

	FWireNode()
		: NodeType(EWireNodeType::FreeNode)
		, Location(FVector::ZeroVector)
	{
	}

	FWireNode(const FVector& InLocation)
		: NodeType(EWireNodeType::FreeNode)
		, Location(InLocation)
	{
	}
};

/**
 * A Wire is a lumped element structure with dynamic resolution, the wire will adapt the resolution
 * so that no unwanted vibrations will occur. The Wire is initialized from a set of routing nodes
 * that the user places but during runtime nodes may be created and removed as necessary. There are
 * different types of nodes and some nodes are persistent.
 */
UCLASS(ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WireComponent : public USceneComponent
{
public:
	GENERATED_BODY()

public:
	UAGX_WireComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire")
	TArray<FWireNode> RouteNodes;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNode(const FWireNode& InNode);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtLocation(const FVector& InLocation);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtIndex(const FWireNode& InNode, int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtLocationAtIndex(const FVector& InLocation, int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void RemoveNode(int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetNodeLocation(int32 InIndex, const FVector& InLocation);

	//~ Begin ActorComponent Interface.

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnRegister() override;

	//~ End ActorComponent Interface.
};
