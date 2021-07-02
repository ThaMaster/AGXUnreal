#pragma once

// AGX Dynamics for Unreal includes.
#include "NativeBarrier.h"
#include "Wire/WireRenderIteratorBarrier.h"

// System includes.
#include <memory>

struct FWireRef;
class FWireNodeBarrier;
class FWireWinchBarrier;

extern template class AGXUNREALBARRIER_API FNativeBarrier<FWireRef>;

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
	void AllocateNative(
		float Radius, float ResolutionPerUnitLength, float DampingBend, float DampingStretch,
		float YoungsModulusBend, float YoungsModulusStretch);

	void SetScaleConstant(double ScaleConstant);
	double GetScaleConstant() const;

	void SetLinearVelocityDamping(double Damping);
	double GetLinearVelocityDamping() const;

	bool GetRenderListEmpty() const;

	void AddRouteNode(FWireNodeBarrier& RoutingNode);
	void AddWinch(FWireWinchBarrier& Winch);
	bool IsInitialized() const;
	double GetRestLength() const;

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
