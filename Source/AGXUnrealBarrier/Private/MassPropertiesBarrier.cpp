#include "MassPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "TypeConversions.h"
#include "AGX_LogCategory.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Vec3.h>
#include "EndAGXIncludes.h"

FMassPropertiesBarrier::FMassPropertiesBarrier()
	: NativePtr {new FMassPropertiesPtr}
{
}

FMassPropertiesBarrier::FMassPropertiesBarrier(std::unique_ptr<FMassPropertiesPtr> Native)
	: NativePtr {std::move(Native)}
{
}

FMassPropertiesBarrier::FMassPropertiesBarrier(FMassPropertiesBarrier&& Other)
	: NativePtr {std::move(Other.NativePtr)}
{
	Other.NativePtr = std::make_unique<FMassPropertiesPtr>();
}

FMassPropertiesBarrier::~FMassPropertiesBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the std::unique_ptr
	// NativePtr's destructor must be able to see the definition, not just the declaration, of
	// FMassPropertiesPtr.
}

void FMassPropertiesBarrier::SetMass(float MassUnreal)
{
	check(HasNative());
	agx::Real MassAgx = Convert(MassUnreal);
	NativePtr->Native->setMass(MassAgx);
}

float FMassPropertiesBarrier::GetMass() const
{
	check(HasNative());
	agx::Real MassAgx = NativePtr->Native->getMass();
	float MassUnreal = Convert(MassAgx);
	return MassUnreal;
}

void FMassPropertiesBarrier::SetInertiaTensor(const FMatrix& InertiaTensor)
{
	check(HasNative());
	agx::SPDMatrix3x3 InertiaTensorAGX = ConvertSPDMatrix(InertiaTensor);
	NativePtr->Native->setInertiaTensor(InertiaTensorAGX);
}

FMatrix FMassPropertiesBarrier::GetInertiaTensor() const
{
	check(HasNative());
	agx::SPDMatrix3x3 InertiaTensorAGX = NativePtr->Native->getInertiaTensor();
	FMatrix InertiaTensor = ConvertSPDMatrix(InertiaTensorAGX);
	return InertiaTensor;
}

void FMassPropertiesBarrier::SetPrincipalInertiae(const FVector& InertiaUnreal)
{
	check(HasNative());
	agx::Vec3 InertiaAgx = Convert(InertiaUnreal);
	NativePtr->Native->setInertiaTensor(InertiaAgx);
}

FVector FMassPropertiesBarrier::GetPrincipalInertiae() const
{
	check(HasNative());
	agx::Vec3 InertiaAgx = NativePtr->Native->getPrincipalInertiae();
	FVector InertiaUnreal = Convert(InertiaAgx);
	return InertiaUnreal;
}

#if 1
namespace MassPropertiesBarrier_helpers
{
	void SetAutoGenerateFlag(agx::MassProperties& MassProperties, agx::Int32 Flag)
	{
		agx::UInt32 Mask = MassProperties.getAutoGenerateMask();
		Mask &= Flag;
		MassProperties.setAutoGenerateMask(Mask);
	}

	bool GetAutoGenerateFlag(const agx::MassProperties& MassProperties, agx::Int32 Flag)
	{
		const agx::UInt32 Mask = MassProperties.getAutoGenerateMask();
		return (Mask & Flag) != 0;
	}
}

void FMassPropertiesBarrier::SetAutoGenerateMass(bool bAuto)
{
	check(HasNative());
	MassPropertiesBarrier_helpers::SetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::MASS);
}

bool FMassPropertiesBarrier::GetAutoGenerateMass() const
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::GetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::MASS);
}

void FMassPropertiesBarrier::SetAutoGenerateCenterOfMassOffset(bool bAuto)
{
	check(HasNative());
	MassPropertiesBarrier_helpers::SetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::CM_OFFSET);
}

bool FMassPropertiesBarrier::GetAutoGenerateCenterOfMassOffset() const
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::GetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::CM_OFFSET);
}

void FMassPropertiesBarrier::SetAutoGenerateInertiaTensor(bool bAuto)
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::SetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::INERTIA);
}

bool FMassPropertiesBarrier::GetAutoGenerateInertiaTensor() const
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::GetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::INERTIA);
}
#else
namespace MassPropertiesBarrier_helpers
{
	constexpr agx::UInt32 EnableMask = agx::MassProperties::AutoGenerateFlags::AUTO_GENERATE_ALL;
	constexpr agx::UInt32 DisableMask = agx::MassProperties::AutoGenerateFlags::CM_OFFSET;
}

void FMassPropertiesBarrier::SetAutoGenerate(bool bAutoGenerate)
{
	using namespace MassPropertiesBarrier_helpers;
	check(HasNative());
	const agx::UInt32 Mask = bAutoGenerate ? EnableMask : DisableMask;
	NativePtr->Native->setAutoGenerateMask(Mask);
}

bool FMassPropertiesBarrier::GetAutoGenerate() const
{
	using namespace MassPropertiesBarrier_helpers;
	check(HasNative());
	const agx::UInt32 Mask = NativePtr->Native->getAutoGenerateMask();
	if (Mask == EnableMask)
	{
		return true;
	}
	else if (Mask == DisableMask)
	{
		return false;
	}
	else
	{
		UE_LOG(LogAGX, Warning, TEXT("Found MassProperty with inconclusive AutoGenerate mask."));
		/// @todo What is a sane return value here? Is there some other way to handle the whole
		/// thing?
		return true;
	}
}
#endif

bool FMassPropertiesBarrier::HasNative() const
{
	return NativePtr->Native != nullptr;
}

FMassPropertiesPtr* FMassPropertiesBarrier::GetNative()
{
	check(HasNative());
	return NativePtr.get();
}

const FMassPropertiesPtr* FMassPropertiesBarrier::GetNative() const
{
	return NativePtr.get();
}

void FMassPropertiesBarrier::BindTo(FRigidBodyRef& RigidBody)
{
	if (RigidBody.Native == nullptr)
	{
		NativePtr->Native = nullptr;
		return;
	}

	NativePtr->Native = RigidBody.Native->getMassProperties();
}
