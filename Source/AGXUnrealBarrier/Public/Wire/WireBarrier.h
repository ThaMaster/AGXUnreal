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
 * We need to separate the Linux and Windows and Linux declarations because Linux must have the
 * visibility decorator here for the inherited member functions to be visible to users of
 * FWireBarrier, while Visual Studio explicitly forbids it and instead require that the visibility
 * decorator is on the actual instantiation in the .cpp file instead.
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

	void SetRadius(float Radius);
	float GetRadius() const;

	void SetResolutionPerUnitLength(float InResolution);
	float GetResolutionPerUnitLength() const;

	void SetScaleConstant(double ScaleConstant);
	double GetScaleConstant() const;

	void SetLinearVelocityDamping(double Damping);
	double GetLinearVelocityDamping() const;

	void SetMaterial(const FShapeMaterialBarrier& Material);

	bool GetRenderListEmpty() const;

	void AddRouteNode(FWireNodeBarrier& RoutingNode);
	void AddWinch(FWireWinchBarrier& Winch);
	bool IsInitialized() const;

	/**
	 * @return The length of the wire not including wire inside any winches.
	 */
	double GetRestLength() const;

	double GetMass() const;

	double GetTension() const;

	/**
	 * Attach a winch to a free end of this wire.
	 *
	 * If an object is attached to begin, it will be detached, and this winch controller will be attached at this position instead.
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

	FWireRenderIteratorBarrier GetRenderBeginIterator() const;
	FWireRenderIteratorBarrier GetRenderEndIterator() const;

protected:
	//~ Begin FNativeBarrier interface.
/// @todo Determine if we need to override these
#if 0
	virtual void PreNativeChanged() override;
	virtual void PostNativeChanged() override;
#endif
	//~ End FNativeBarrier interface.
};
