#pragma once

// AGXUnreal includes.
#include "Tires/TireBarrier.h"

class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FTwoBodyTireBarrier : public FTireBarrier
{
public:
	FTwoBodyTireBarrier();
	FTwoBodyTireBarrier(FTwoBodyTireBarrier&& Other) = default;
	FTwoBodyTireBarrier(std::unique_ptr<FTireRef> Native);
	virtual ~FTwoBodyTireBarrier();

	void AllocateNative(
		const FRigidBodyBarrier* TireRigidBody, float OuterRadius, const FRigidBodyBarrier* HubRigidBody, float InnerRadius);

private:
	FTwoBodyTireBarrier(const FTwoBodyTireBarrier&) = delete;
	void operator=(const FTwoBodyTireBarrier&) = delete;
};
