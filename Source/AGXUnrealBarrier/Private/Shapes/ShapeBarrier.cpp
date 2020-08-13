#include "Shapes/ShapeBarrier.h"

// AGXUnreal includes.
#include "AGXBarrierFactories.h"
#include "AGXRefs.h"
#include "TypeConversions.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "AGX_LogCategory.h"

// AGX Dynamics includes.
#include "agxCollide/CollisionGroupManager.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

FShapeBarrier::FShapeBarrier()
	: NativeRef {new FGeometryAndShapeRef}
{
}

FShapeBarrier::FShapeBarrier(FShapeBarrier&& Other) noexcept
	: NativeRef {std::move(Other.NativeRef)}
{
}

FShapeBarrier::FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: NativeRef {std::move(Native)}
{
}

FShapeBarrier::~FShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FGeometryAndShapeRef.
}

FShapeBarrier& FShapeBarrier::operator=(FShapeBarrier&& Other) noexcept
{
	NativeRef = std::move(Other.NativeRef);
	return *this;
}

bool FShapeBarrier::HasNative() const
{
	return NativeRef->NativeGeometry != nullptr && NativeRef->NativeShape != nullptr;
}

void FShapeBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->NativeGeometry = new agxCollide::Geometry();
	AllocateNativeShape();
	NativeRef->NativeGeometry->add(NativeRef->NativeShape);
}

void FShapeBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->NativeGeometry->remove(NativeRef->NativeShape);
	ReleaseNativeShape();
	NativeRef->NativeGeometry = nullptr;
}

FGeometryAndShapeRef* FShapeBarrier::GetNative()
{
	return NativeRef.get();
}

const FGeometryAndShapeRef* FShapeBarrier::GetNative() const
{
	return NativeRef.get();
}

void FShapeBarrier::SetLocalPosition(const FVector& Position)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalPosition(ConvertVector(Position));
}

void FShapeBarrier::SetLocalRotation(const FQuat& Rotation)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalRotation(Convert(Rotation));
}

FVector FShapeBarrier::GetLocalPosition() const
{
	return std::get<0>(GetLocalPositionAndRotation());
}

FQuat FShapeBarrier::GetLocalRotation() const
{
	return std::get<1>(GetLocalPositionAndRotation());
}

void FShapeBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->NativeGeometry->setName(Convert(Name));
}

FString FShapeBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->NativeGeometry->getName());
}

void FShapeBarrier::SetMaterial(const FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	NativeRef->NativeGeometry->setMaterial(Material.GetNative()->Native);
}

FShapeMaterialBarrier FShapeBarrier::GetMaterial() const
{
	check(HasNative());
	agx::Material* Material = NativeRef->NativeGeometry->getMaterial();
	return AGXBarrierFactories::CreateShapeMaterialBarrier(Material);
}

void FShapeBarrier::SetEnableCollisions(bool CanCollide)
{
	check(HasNative());
	NativeRef->NativeGeometry->setEnableCollisions(CanCollide);
}

bool FShapeBarrier::GetEnableCollisions() const
{
	check(HasNative());
	return NativeRef->NativeGeometry->getEnableCollisions();
}

void FShapeBarrier::AddCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Add collision group as (hashed) unsigned int.
	NativeRef->NativeGeometry->addGroup(StringTo32BitFnvHash(GroupName.ToString()));
}

TArray<FName> FShapeBarrier::GetCollisionGroups() const
{
	check(HasNative());
	TArray<FName> Result;
	agxCollide::GroupIdCollection Groups = NativeRef->NativeGeometry->findGroupIdCollection();
	for (const agx::Name& Name : Groups.getNames())
	{
		Result.Add(FName(*Convert(Name)));
	}
	for (const agx::UInt32 Id : Groups.getIds())
	{
		Result.Add(FName(*FString::FromInt(Id)));
	}
	return Result;
}

FGuid FShapeBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->NativeShape->getUuid());
	return Guid;
}

bool FShapeBarrier::HasRenderData() const
{
	check(HasNative());
	return NativeRef->NativeShape->getRenderData() != nullptr;
}

FAGX_RenderData FShapeBarrier::GetRenderData() const
{
	check(HasNative());
	const agxCollide::RenderData* RenderDataAGX = NativeRef->NativeShape->getRenderData();
	const agxCollide::RenderMaterial* RenderMaterial = RenderDataAGX->getRenderMaterial();
	/// @todo Use the RenderMaterial::Has.+ functions here. FAGX_RenderData must know which
	/// properties has been set.
	agx::Vec3 DiffuseColorAgx(RenderMaterial->getDiffuseColor().asVec3());
	FAGX_RenderData RenderDataUnreal;
	RenderDataUnreal.DiffuseColor = Convert(DiffuseColorAgx);
	return RenderDataUnreal;
}

namespace
{
	agxCollide::ShapeIterator FindShape(agxCollide::Geometry* Geometry, agxCollide::Shape* Shape)
	{
		agxCollide::ShapeIterator Iterator(Geometry);
		while (Iterator.isValid() && Iterator.getShape() != Shape)
		{
			Iterator.next();
		}
		if (!Iterator.isValid())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Found invalid FShapeBarrier. The native Geometry does not contain the native "
					 "Shape."));
		}
		return Iterator;
	}
}

std::tuple<FVector, FQuat> FShapeBarrier::GetLocalPositionAndRotation() const
{
	check(HasNative());
	agxCollide::ShapeIterator Iterator =
		FindShape(NativeRef->NativeGeometry, NativeRef->NativeShape);
	if (!Iterator.isValid())
	{
		return {FVector::ZeroVector, FQuat::Identity};
	}

	// The ShapeTransform is always Identity when the Native objects have been
	// created from AGXUnreal objects. However, it can be a non-Identity transform
	// during import from a .agx archive. Split this implementation if this step
	// is shown to be a performance problem.
	const agx::AffineMatrix4x4& GeometryTransform = NativeRef->NativeGeometry->getLocalTransform();
	const agx::AffineMatrix4x4& ShapeTransform = Iterator.getLocalTransform();
	const agx::AffineMatrix4x4 ShapeRelativeBody = ShapeTransform * GeometryTransform;
	return {ConvertVector(ShapeRelativeBody.getTranslate()),
			Convert(ShapeRelativeBody.getRotate())};
}
