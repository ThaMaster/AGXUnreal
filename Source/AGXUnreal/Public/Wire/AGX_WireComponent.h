#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "AGX_WireRenderIterator.h"
#include "Wire/AGX_WireEnums.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_WireComponent.generated.h"

class UAGX_ShapeMaterialBase;


USTRUCT(BlueprintType)
struct FWireRoutingNode
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	EWireNodeType NodeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	FVector Location;

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

	/**
	 * The radius of the wire, in cm.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float Radius = 1.5f;

	/**
	 * The maximum number of mass nodes per cm.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float ResolutionPerUnitLength = 0.02;

	/**
	 * Velocity damping value of the wire.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float LinearVelocityDamping;

	/**
	 * Value that indicates how likely it is that mass nodes appears along the wire. Higher value
	 * means more likely.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float ScaleConstant;

	/**
	 * The physical material of the wire.
	 *
	 * This determines things such as the density of the wire and how it behaves when in contact
	 * with Shapes in the world.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	UAGX_ShapeMaterialBase* PhysicalMaterial;

	/**
	 * A list of nodes that are used to initialize the wire.
	 *
	 * At BeginPlay these nodes are used to create simulation nodes so the route nodes cannot be
	 * used to track the motion of the wire.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire")
	TArray<FWireRoutingNode> RouteNodes;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNode(const FWireRoutingNode& InNode);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtLocation(const FVector& InLocation);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtLocationAtIndex(const FVector& InLocation, int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void RemoveNode(int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetNodeLocation(int32 InIndex, const FVector& InLocation);

	/**
	 * A wire is initialized when the AGX Dynamics object has been created and added to the AGX
	 * Dynamics simulation. At this point that routing nodes become inactive and the node iterator
	 * should be used to inspect the nodes.
	 * @return
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool IsInitialized() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireRenderIterator GetRenderBeginIterator() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireRenderIterator GetRenderEndIterator() const;

	bool HasNative() const;
	FWireBarrier* GetOrCreateNative();
	FWireBarrier* GetNative();
	const FWireBarrier* GetNative() const;
	void CopyFrom(const FWireBarrier& Barrier);


	//~ Begin ActorComponent interface.
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End ActorComponent interface.

protected:
	// ~Begin UActorComponent interface.
	virtual void OnRegister() override;
	// ~End UActorComponent interface.

private:
	void CreateNative();

private:
	FWireBarrier NativeBarrier;
};
