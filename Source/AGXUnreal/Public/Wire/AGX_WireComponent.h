#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "AGX_WireRenderIterator.h"
#include "Wire/AGX_WireEnums.h"
#include "Wire/AGX_WireWinch.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireComponent.generated.h"

class UAGX_ShapeMaterialBase;
class UAGX_WireWinchComponent;

/**
 * Route nodes are used to specify the initial route of the wire. Each node has a location but
 * no orientation. Some members are only used for some node types, such as RigidBody which is only
 * used by Eye and BodyFixed nodes.
 */
USTRUCT(BlueprintType)
struct FWireRoutingNode
{
	GENERATED_BODY();

	/**
	 * The type of wire node, e.g., Free, Eye, BodyFixed, etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	EWireNodeType NodeType;

	/**
	 * The location of this node relative to the Wire Component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	FVector Location;

	/**
	 * The Rigid Body that an Eye or BodyFixed node should be attached to.
	 *
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
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_WireRouteNode_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static bool SetBody(UPARAM(ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body);
};

/**
 * A Wire is a lumped element structure with dynamic resolution, the wire will adapt the resolution
 * so that no unwanted vibrations will occur. The Wire is initialized from a set of routing nodes
 * that the user places but during runtime nodes will be created and removed as necessary so the
 * routing nodes cannot be used to inspect the current wire path. Instead use the render iterator
 * to iterate over the wire, which will give access to FAGX_WireNode instances, which wrap the
 * underlying AGX Dynamics wire nodes.
 */
UCLASS(ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WireComponent : public USceneComponent, public IAGX_NativeOwner
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
	 * The maximum number of mass nodes per cm during simulation.
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
	float LinearVelocityDamping = 0.0f;

	// Not sure what this is, or if we should expose it.
	///**
	// * Value that indicates how likely it is that mass nodes appears along the wire. Higher value
	// * means more likely.
	// */
	// UPROPERTY(
	//	EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
	//	Meta = (ClampMin = "0", UIMin = "0"))
	// float ScaleConstant = 0.35f;

	/**
	 * The physical material of the wire.
	 *
	 * This determines things such as the density of the wire and how it behaves when in contact
	 * with Shapes in the world.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Wire")
	UAGX_ShapeMaterialBase* PhysicalMaterial;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Begin Winch",
		Meta = (DisplayName = "Winch Type"))
	EWireWinchOwnerType BeginWinchType = EWireWinchOwnerType::None;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Begin Winch",
		Meta = (EditConditionHides, EditCondition = "BeginWinchType == EWireWinchOwnerType::Wire"))
	FAGX_WireWinch OwnedBeginWinch;

	/**
	 * This is the Wire Winch that will be used when Begin Winch Type is set to Other. The code or
	 * Visual Script that sets Begin Winch Type to Other is fully responsible for this pointer and
	 * the FAGX_WireWinch that it points to.
	 */
	FAGX_WireWinch* BorrowedBeginWinch;

	/// @todo The engine example, ULiveLinkComponentController, uses EditInstanceOnly here.
	/// Determine if that is a requirement or not. Possibly related to Blueprint Editor
	/// weirdness. FComponentReference is not supported by Blueprint, so we must provide some
	/// other way to set the target from a Blueprint Visual Script.
	UPROPERTY(
		EditAnywhere, Category = "AGX Wire Begin Winch",
		Meta =
			(UseComponentPicker, AllowedClasses = "AGX_WireWinchComponent", DisallowedClasses = "",
			 EditConditionHides,
			 EditCondition = "BeginWinchType == EWireWinchOwnerType::WireWinch"))
	FComponentReference BeginWinchComponent;

	UFUNCTION(BlueprintCallable)
	UAGX_WireWinchComponent* GetBeginWinchComponent();

	const UAGX_WireWinchComponent* GetBeginWinchComponent() const;

	bool HasBeginWinchComponentWinch() const;
	FAGX_WireWinch* GetBeginWinchComponentWinch();
	const FAGX_WireWinch* GetBeginWinchComponentWinch() const;

	/**
	 * Determine if this wire has any type of winch as the begin side. The winch can be eiter
	 * owned by the wire, i.e., OwnedBeginWinch, owned by a Winch Component referenced through
	 * BeginWinchComponent, or a borrowed winch pointed to by BorrowedBeginWinch.
	 *
	 * @return True if this wire has any type winch at the begin side.
	 */
	bool HasBeginWinch() const;

	/**
	 * Get the Wire Winch object that the begin side of this wire is attached to, if any.
	 * @return The attached Wire Winch, or nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireWinch& GetBeginWinch();

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire End Winch",
		Meta = (DisplayName = "Winch Type"))
	EWireWinchOwnerType EndWinchType = EWireWinchOwnerType::None;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire End Winch",
		Meta = (EditConditionHides, EditCondition = "EndWinchType == EWireWinchOwnerType::Wire"))
	FAGX_WireWinch OwnedEndWinch;

	UPROPERTY(
		EditAnywhere, Category = "AGX Wire End Winch",
		Meta =
			(UseComponentPicker, AllowedClasses = "AGX_WireWinchComponent", DisallowedClasses = "",
			 EditConditionHides, EditCondition = "EndWinchType == EWireWinchOwnerType::WireWinch"))
	FComponentReference EndWinchComponent;

	/**
	 * An array of nodes that are used to initialize the wire.
	 *
	 * At BeginPlay these nodes are used to create simulation nodes and after that the route nodes
	 * aren't used anymore. Use the render iterator to track the motion of the wire over time.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Route")
	TArray<FWireRoutingNode> RouteNodes;

	/** For demonstration/experimentation purposes. Will be replaced with Wire Material shortly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire|ToBeMovedToMaterial")
	float DampingBend = 0.075f;

	/** For demonstration/experimentation purposes. Will be replaced with Wire Material shortly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire|ToBeMovedToMaterial")
	float DampingStretch = 0.075f;

	/** For demonstration/experimentation purposes. Will be replaced with Wire Material shortly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire|ToBeMovedToMaterial")
	float YoungsModulusBend = 6e10f;

	/** For demonstration/experimentation purposes. Will be replaced with Wire Material shortly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire|ToBeMovedToMaterial")
	float YoungsModulusStretch = 6e10f;

	/**
	 * Add a new route node to the wire.
	 *
	 * This should be called before BeginPlay since route nodes are only used during initialization.
	 *
	 * @param InNode The node to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNode(const FWireRoutingNode& InNode);

	/**
	 * Add a default-constructed route node at the designated local location to the end of the node
	 * array.
	 *
	 * @param InLocation The location of the node, relative to the Wire Component.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtLocation(const FVector& InLocation);

	/**
	 * Add a default-constructed route node at the designated index in the route array, pushing all
	 * subsequent nodes one index.
	 *
	 * @param InNode The route node to add.
	 * @param InIndex The place in the route node array to add the node at.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex);

	/**
	 * Add a default-constructed route node, placed at the given local location, at the designated
	 * index in the route array, pushing all subsequent nodes one index..
	 *
	 * @param InLocation The location of the new node relative to the Wire Component.
	 * @param InIndex The place in the route node array to add the new node at.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddNodeAtLocationAtIndex(const FVector& InLocation, int32 InIndex);

	/**
	 * Remove the route node at the given index.
	 * @param InIndex The index of the node to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void RemoveNode(int32 InIndex);

	/**
	 * Set the locaion of the node at the given index. The location is relative to the wire
	 * component.
	 *
	 * @param InIndex The index of the node to remove.
	 * @param InLocation The new local location for the node.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetNodeLocation(int32 InIndex, const FVector& InLocation);

	/**
	 * A wire is initialized when the AGX Dynamics object has been created and added to the AGX
	 * Dynamics simulation, which happens in BeginPlay. At this point that routing nodes become
	 * inactive and the render iterator should be used to inspect the simulation nodes.
	 *
	 * The initialization may fail, which will produce a wire for which HasNative is true but
	 * IsInitialized is false.
	 *
	 * @return True if the wire has been initialized.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool IsInitialized() const;

	double GetRestLength() const;

	/// @return True if this wire has at least one renderable simulation node.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool HasRenderNodes() const;

	/// @return True if there are no renderable simulation nodes.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool GetRenderListEmpty() const;

	/// @return An iterator pointing to the first renderable FAGX_WireNode simulation node.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireRenderIterator GetRenderBeginIterator() const;

	/// @return An iterator pointing one-past-end of the renderable simulation nodes.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireRenderIterator GetRenderEndIterator() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	TArray<FVector> GetRenderNodeLocations() const;

	/// Copy configuration from the given Barrier.
	/// @note Not yet implemented.
	void CopyFrom(const FWireBarrier& Barrier);

	//~ Begin IAGX_NativeOwner interface.
	/**
	 * @return True if a native AGX Dynamics representation has been created for this Wire
	 * Component.
	 */
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void AssignNative(uint64 NativeAddress) override;
	//~ End IAGX_NativeOwner interface.

	/**
	 * Return the Barrier object for this Wire Component, creating it if necessary. Should only be
	 * called at or after BeginPlay has begun for the current level.
	 *
	 * @return The Barrier object for this Wire Component.
	 */
	FWireBarrier* GetOrCreateNative();

	/// @return The Barrier object for this Wire Component, or nullptr if there is none.
	FWireBarrier* GetNative();

	/// @return The Barrier object for this Wire Component, or nullptr if there is none.
	const FWireBarrier* GetNative() const;

	// ~Begin UObject interface.
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.

	//~ Begin ActorComponent interface.
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
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
