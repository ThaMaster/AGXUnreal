// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "NativeBarrier.h"
#include "Wire/WireRenderIteratorBarrier.h"

// System includes.
#include <memory>

class FShapeMaterialBarrier;
struct FWireRef;
class FWireNodeBarrier;
class FWireWinchBarrier;

/*
 * The goal here is to tell the compiler that there is a template instantiation of
 * FNativeBarrier<FWireRef> somewhere, without actually triggering an instantiation here and now.
 * The instantiation is instead done explicitly in the .cpp file. We do this because FWireRef is
 * only declared, not defined, here in the header file since the type contains AGX Dynamics types
 * that can't be named outside of the AGXUnrealBarrier module, and this header file is included in
 * other modules.
 *
 * We need to separate the Linux and Windows declarations because Linux must have the visibility
 * decorator here for the inherited member functions to be visible to users of FWireBarrier, while
 * Visual Studio explicitly forbids it and instead require that the visibility decorator is on the
 * instantiation in the .cpp file instead.
 */
#if PLATFORM_LINUX
extern template class AGXUNREALBARRIER_API FNativeBarrier<FWireRef>;
#elif PLATFORM_WINDOWS
extern template class FNativeBarrier<FWireRef>;
#else
#pragma error("This platform is currently not supported.");
#endif

class AGXUNREALBARRIER_API FWireBarrier : public FNativeBarrier<FWireRef>
{
public:
	using Super = FNativeBarrier<FWireRef>;

	FWireBarrier();
	FWireBarrier(std::unique_ptr<FWireRef>&& Native);
	FWireBarrier(FWireBarrier&& Other);
	virtual ~FWireBarrier();

	/** Damping and Young's modulus for demonstration/experimentation purposes. Will be replaced
	 * with Wire Material shortly. */
	void AllocateNative(float Radius, float ResolutionPerUnitLength);

	/** Set the radius of the wire [cm]. */
	void SetRadius(float Radius);

	/** Get the radius of the wire [cm]. */
	float GetRadius() const;

	/** Get the maximum resolution of the wire [nodes/cm]. */
	void SetResolutionPerUnitLength(float InResolution);

	/** Set the maximum resolution of the wire [nodes/cm]. */
	float GetResolutionPerUnitLength() const;

	void SetScaleConstant(double ScaleConstant);
	double GetScaleConstant() const;

	void SetLinearVelocityDamping(double Damping);
	double GetLinearVelocityDamping() const;

	void SetMaterial(const FShapeMaterialBarrier& Material);

	FShapeMaterialBarrier GetMaterial() const;

	bool GetRenderListEmpty() const;

	void AddRouteNode(FWireNodeBarrier& RoutingNode);
	void AddWinch(FWireWinchBarrier& Winch);

	/**
	 * Get the very first node in the wire. This may be before the node pointed to by Get Render
	 * Begin Iterator, for example when the begin side of the wire is attached to a winch.
	 */
	FWireNodeBarrier GetFirstNode() const;

	/**
	 * Get the very last node in the wire. This may be after last node reachable from the Get Render
	 * Begin Iterator, for example when the end side of the wire is attached to a winch.
	 */
	FWireNodeBarrier GetLastNode() const;

	FWireWinchBarrier GetBeginWinch() const;
	FWireWinchBarrier GetEndWinch() const;
	FWireWinchBarrier GetWinch(EWireSide Side) const;

	bool IsInitialized() const;

	/**
	 * @return The length of the wire not including wire inside any winches [cm].
	 */
	double GetRestLength() const;

	/** @return The mass of the wire [kg]. */
	double GetMass() const;

	/** Get the tension at the begin side of the wire [N] */
	double GetTension() const;

	/**
	 * Attach a winch to a free end of this wire.
	 *
	 * If an object is attached to begin, it will be detached, and this winch controller will be
	 * attached at this position instead.
	 *
	 * Parameters
	 * @param Winch	Winch to attach.
	 * @param bBegin True if the winch should be attached at begin, false at end.
	 * @return True if the Winch was attached, false otherwise.
	 */
	bool Attach(FWireWinchBarrier& Winch, bool bBegin);

	/**
	 * Detach begin or end of this wire (if attached to something).
	 *
	 * @param bBegin If true begin of this wire will be detached, otherwise end.
	 * @return True if a detach was performed, false otherwise.
	 */
	bool Detach(bool bBegin);

	bool Detach(FWireWinchBarrier& Winch);

	FWireRenderIteratorBarrier GetRenderBeginIterator() const;
	FWireRenderIteratorBarrier GetRenderEndIterator() const;

	bool IsLumpedNode(const FWireNodeBarrier& Node) const;
	bool IsLumpedNode(const FWireRenderIteratorBarrier& Node) const;

	FString GetName() const;

	FGuid GetGuid() const;

protected:
	//~ Begin FNativeBarrier interface.
/// @todo Determine if we need to override these
#if 0
	virtual void PreNativeChanged() override;
	virtual void PostNativeChanged() override;
#endif
	//~ End FNativeBarrier interface.
};
