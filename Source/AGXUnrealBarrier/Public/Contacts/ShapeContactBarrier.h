#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointBarrier.h"
#include "Contacts/AGX_ContactState.h"
#include "Materials/ContactMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/EmptyShapeBarrier.h"

struct FShapeContactEntity;

class AGXUNREALBARRIER_API FShapeContactBarrier
{
public:
	FShapeContactBarrier();
	FShapeContactBarrier(std::unique_ptr<FShapeContactEntity> InNativeEntity);
	FShapeContactBarrier(FShapeContactBarrier&& InOther);
	~FShapeContactBarrier();

	FShapeContactBarrier& operator=(const FShapeContactBarrier& InOther);

	bool IsEnabled() const;

	EAGX_ContactState GetContactState();

	FRigidBodyBarrier GetBody1() const;
	FRigidBodyBarrier GetBody2() const;

	FEmptyShapeBarrier GetShape1() const;
	FEmptyShapeBarrier GetShape2() const;

	FVector CalculateRelativeVelocity(int32 PointIndex) const;

	FContactMaterialBarrier GetContactMaterial() const;

	int32 GetNumContactPoints() const;
	TArray<FContactPointBarrier> GetContactPoints() const;

	FContactPointBarrier GetContactPoint(int32 Index) const;

	bool HasNative() const;
	FShapeContactEntity* GetNative();
	const FShapeContactEntity* GetNative() const;

private:
	FShapeContactBarrier(const FShapeContactBarrier&) = delete;

private:
	std::unique_ptr<FShapeContactEntity> NativeEntity;
};
