#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireRenderIteratorBarrier.h"

// System includes.
#include <memory>

struct FWireRef;
class FWireNodeBarrier;
class FWireWinchBarrier;

class AGXUNREALBARRIER_API FWireBarrier
{
public:
	FWireBarrier();
	FWireBarrier(FWireBarrier&& Other);
	FWireBarrier(std::unique_ptr<FWireRef>&& Native);
	~FWireBarrier();

	bool HasNative() const;

	/** Damping and Young's modulus for demonstration/experimentation purposes. Will be replaced
	 * with Wire Material shortly. */
	void AllocateNative(
		float Radius, float ResolutionPerUnitLength, float DampingBend, float DampingStretch,
		float YoungsModulusBend, float YoungsModulusStretch);

	FWireRef* GetNative();
	const FWireRef* GetNative() const;
	void ReleaseNative();

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

private:
	FWireBarrier(const FWireBarrier&) = delete;
	void operator=(const FWireBarrier&) = delete;

private:
	std::unique_ptr<FWireRef> NativeRef;
};
