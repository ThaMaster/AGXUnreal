#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// System includes.
#include <memory>

struct FMassPropertiesPtr;
struct FRigidBodyRef;

class AGXUNREALBARRIER_API FMassPropertiesBarrier
{
public:
	FMassPropertiesBarrier();
	FMassPropertiesBarrier(std::unique_ptr<FMassPropertiesPtr> Native);
	FMassPropertiesBarrier(FMassPropertiesBarrier&& Other);
	~FMassPropertiesBarrier();

	void SetMass(float NewMass);
	float GetMass() const;

	void SetPrincipalInertiae(const FVector& NewPrincipalInertiae);
	FVector GetPrincipalInertiae() const;

	void SetAutoGenerate(bool bAutoGenerate);
	bool GetAutoGenerate() const;

	bool HasNative() const;
	FMassPropertiesPtr* GetNative();
	const FMassPropertiesPtr* GetNative() const;
	// No AllocateNative or ReleaseNative because MassProperties are not stand-alone objects in AGX
	// Dynamics, the are always owned by a RigidBody;

	void BindTo(FRigidBodyRef& RigidBody);

private:
	std::unique_ptr<FMassPropertiesPtr> NativePtr;
};
