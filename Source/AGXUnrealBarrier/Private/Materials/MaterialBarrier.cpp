#include "Materials/MaterialBarrier.h"

#include "AGXRefs.h"

#include <Misc/AssertionMacros.h>

FMaterialBarrier::FMaterialBarrier()
	: NativeRef {new FMaterialRef}
{
}

FMaterialBarrier::~FMaterialBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::uniue_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FMaterialRef.
}

bool FMaterialBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FMaterialRef* FMaterialBarrier::GetNative()
{
	return NativeRef.get();
}

const FMaterialRef* FMaterialBarrier::GetNative() const
{
	return NativeRef.get();
}

void FMaterialBarrier::AllocateNative(const FString& Name)
{
	check(!HasNative());
	NativeRef->Native = new agx::Material(Convert(Name));
}

void FMaterialBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FMaterialBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->Native->setName(Convert(Name));
}

FString FMaterialBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

void FMaterialBarrier::SetDensity(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setDensity(Density);
}

double FMaterialBarrier::GetDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getDensity();
}

void FMaterialBarrier::SetYoungsModulus(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setYoungsModulus(YoungsModulus);
}

double FMaterialBarrier::GetYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getYoungsModulus();
}

void FMaterialBarrier::SetBulkViscosity(double Viscosity)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setViscosity(Viscosity);
}

double FMaterialBarrier::GetBulkViscosity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getViscosity();
}

void FMaterialBarrier::SetDamping(double Damping)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setDamping(Damping);
}

double FMaterialBarrier::GetDamping() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getDamping();
}

void FMaterialBarrier::SetMinMaxElasticRestLength(double MinElasticRestLength, double MaxElasticRestLength)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setMinMaxElasticRestLength(MinElasticRestLength, MaxElasticRestLength);
}

double FMaterialBarrier::GetMinElasticRestLength() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getMinElasticRestLength();
}

double FMaterialBarrier::GetMaxElasticRestLength() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getMaxElasticRestLength();
}

void FMaterialBarrier::SetFrictionEnabled(bool bEnabled)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setFrictionEnabled(bEnabled);
}

bool FMaterialBarrier::GetFrictionEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getFrictionEnabled();
}

void FMaterialBarrier::SetRoughness(double Roughness)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setRoughness(Roughness);
}

double FMaterialBarrier::GetRoughness() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getRoughness();
}

void FMaterialBarrier::SetSurfaceViscosity(double Viscosity)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setViscosity(Viscosity);
}

double FMaterialBarrier::GetSurfaceViscosity() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getViscosity();
}

void FMaterialBarrier::SetAdhesion(double AdhesiveForce, double AdhesiveOverlap)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setAdhesion(AdhesiveForce, AdhesiveOverlap);
}

double FMaterialBarrier::GetAdhesiveForce() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getAdhesion();
}

double FMaterialBarrier::GetAdhesiveOverlap() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getAdhesiveOverlap();
}
